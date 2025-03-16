#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/stat.h>


/* Global variables for board and line in board */
char line[80];  // Buffer to hold each line
int board[9][9]; // Empty sudoku board


/*Results for multithreaded functions are recorded in these global arrays*/
int results_rows[9];
int results_cols[9];
int results_blocks[9];

/*Rows is index 0, columns is index 1, blocks is index 2*/
int results_final[3];

/*Locks for the result arrays*/
pthread_mutex_t rows_lock;  
pthread_mutex_t cols_lock;
pthread_mutex_t blocks_lock;
pthread_mutex_t results_lock;

/*Structure to send values to check_block()*/
typedef struct {
    int x;
    int y;
} block_offset;

/*test. dont include me in final lol*/
void print_arr(int *arr) {
    printf("[");
    for(int i = 0; i < 9; ++i) {
        printf("%d, ", arr[i]);
    }
    printf("]\n");
    return;
}

/*test. dont include me in final lol*/
void print_arr3(int *arr) {
    printf("[");
    for(int i = 0; i < 3; ++i) {
        printf("%d, ", arr[i]);
    }
    printf("]\n");
    return;
}

/* Locks given location until result is given */
void set_results(pthread_mutex_t lock, int *result_location, int result) {
    pthread_mutex_lock(&lock);
    *result_location = result;
    pthread_mutex_unlock(&lock);
}

/*Sorts the given integer array, assumes size of 9.*/
void sort_array(int *arr) {
    /*Bubble sort is chosen, n is confirmed to be pretty small so it doesn't really matter*/
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8 - i; ++j) {
            if (arr[j] > arr[j + 1]) {
                /*swap*/
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/*Given an array of numbers, checks if exactly [1, 2, 3, 4, 5, 6, 7, 8, 9] after sorted*/
int check_valid(int *arr){
    /*Sort so numbers are in order*/
    sort_array(arr);
    for (int i = 0; i < 9; ++i){
        if (arr[i] != i + 1){
            /*Number is not expected, return false*/
            return 0;
        }
    }
    /*Array is correct, line has 1 of each digit*/
    return 1;
}

/*Checks if a row on the sudoku board is valid*/
int check_row(int y){
    if (y >= 9) {
        /*Invalid offset*/
        return -1;
    }
    int arr[9];
    for (int i = 0; i < 9; ++i){
        arr[i] = (board[y][i]);
    }
    return check_valid(arr);
}

/*Checks if a column on the sudoku board is valid*/
int check_col(int x){
    if (x >= 9) {
        /*Invalid offset*/
        return -1;
    }
    int arr[9];
    for (int i = 0; i < 9; ++i){
        arr[i] = (board[i][x]);
    }
    return check_valid(arr);
}

/*Checks if a row on the sudoku board is valid, multithreaded*/
void *check_row_mt(void *param){
    /*Obtain parameter*/
    int y = *((int *)param);
    free(param);
    /*Check row*/
    int arr[9];
    for (int i = 0; i < 9; ++i){
        arr[i] = (board[y][i]);
    }
    set_results(rows_lock, &results_rows[y], check_valid(arr));
    pthread_exit(0);
}

/*Checks if a column on the sudoku board is valid, multithreaded*/
void *check_col_mt(void *param){
    /*Obtain parameter*/
    int x = *((int *)param);
    free(param);
    /*Check column*/
    int arr[9];
    for (int i = 0; i < 9; ++i){
        arr[i] = (board[i][x]);
    }
    set_results(cols_lock, &results_cols[x], check_valid(arr));
    pthread_exit(0);
}

/*Checks if a 3x3 square is valid. This guy is always multithreaded based on assignment specs.*/
void *check_block(void *param){
    int arr[9];
    /*Input is a structure containing x and y offset*/
    block_offset *offset = (block_offset *)param;
    int x = offset->x;
    int y = offset->y;
    free(offset);
    /*Check in a square*/
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j){
            arr[i * 3 + j] = board[i + (y * 3)][j + (x * 3)];
        }
    }
    set_results(blocks_lock, &results_blocks[x + 3 * y], check_valid(arr));
    pthread_exit(0);
}

/*Checks all rows in one thread*/
void *check_rows(){
    for (int i = 0; i < 9; ++i){
        if (check_row(i) == 0){
            set_results(results_lock, &results_final[0], 0);
            pthread_exit(0);
        }
        //printf("Row %d is valid\n", i); //DELETE ME-- JUST FOR CHECKS
    }
    set_results(results_lock, &results_final[0], 1);
    
    pthread_exit(0);
}

/*Checks all columns in one thread*/
void *check_cols(){
    for (int i = 0; i < 9; ++i){
        if (check_col(i) == 0){
            set_results(results_lock, &results_final[1], 0);
            pthread_exit(0);
        }
    }
    set_results(results_lock, &results_final[1], 1);
    pthread_exit(0);
}

/*Checks all rows in nine threads*/
void *check_rows_mt(){
    pthread_t tid[9];
    /*Dispatch threads*/
    for (int i = 0; i < 9; ++i){
        int *input = malloc(sizeof(*input));
        *input = i;
        pthread_create(&tid[i], NULL, check_row_mt, input);
    }
    /*Wait for threads to conclude*/
    for (int i = 0; i < 9; ++i){
        pthread_join(tid[i], NULL);
    }
    for (int i = 0; i < 9; ++i){
        if (results_rows[i] == 0){
            /*At least one result was invalid*/
            set_results(results_lock, &results_final[0], 0);
            pthread_exit(0);
        }
    }
    /*All results were valid!*/
    set_results(results_lock, &results_final[0], 1);
    pthread_exit(0);
}

/*Checks all rows in nine threads*/
void *check_cols_mt(){
    pthread_t tid[9];
    /*Dispatch threads*/
    for (int i = 0; i < 9; ++i){
        int *input = malloc(sizeof(*input));
        *input = i;
        pthread_create(&tid[i], NULL, check_col_mt, input);
    }
    /*Wait for threads to conclude*/
    for (int i = 0; i < 9; ++i){
        pthread_join(tid[i], NULL);
    }
    for (int i = 0; i < 9; ++i){
        if (results_cols[i] == 0){
            /*At least one result was invalid*/
            //printf("lol: %d\n", i);
            set_results(results_lock, &results_final[1], 0);
            pthread_exit(0);
        }
    }
    /*All results were valid!*/
    set_results(results_lock, &results_final[1], 1);
    pthread_exit(0);
}

/*Checks all blocks, multithreaded*/
void *check_blocks_mt(){
    pthread_t tid[9];
    /*Dispatch threads*/
    for (int i = 0; i < 3; ++i){
        for (int j = 0; j < 3; ++j){
            /*i*3+j returns a nice little index number, same as 2d array to pointer offset*/
            block_offset *input = malloc(sizeof(block_offset));
            input->x = i;
            input->y = j;
            pthread_create(&tid[i * 3 + j], NULL, check_block, input);
        }
    }
    /*Wait for threads to conclude*/
    for (int i = 0; i < 9; ++i){
        pthread_join(tid[i], NULL);
    }
    /*Check result*/
    for (int i = 0; i < 9; ++i){
        if (results_blocks[i] == 0){
            /*At least one result was invalid*/
            set_results(results_lock, &results_final[2], 0);
            pthread_exit(0);
        }
    }
    /*All results were valid!*/
    set_results(results_lock, &results_final[2], 1);
    pthread_exit(0);
}


/* Check all rows in one thread for child process */
void *check_rows_child(void* mem){
    int* results = (int*)mem;
    /* assume row is valid */
    int valid = 1;
    for (int i = 0; i < 9; ++i){
        if (check_row(i) == 0){
            valid = 0;
            break;
        }
    }
    *results =  valid;
    //printf("Rows: %d", valid);
    
    pthread_exit(0);
}

/*Checks all columns in one thread for child process*/
void *check_cols_child(void* mem){
    int* results = (int*)mem;
    /* assume col is valid */
    int valid = 1;
    for (int i = 0; i < 9; ++i){
        if (check_col(i) == 0){
            valid = 0;
            break;
        }
    }
    *results = valid;
    //printf("Cols: %d", valid);
    pthread_exit(0);
}


/*Checks all blocks, multithreaded for child process */
void *check_blocks_child(void* mem){
    int* results = (int*)mem;
    /* assume col is valid */
    int valid = 1;
    pthread_t tid[9];
    /*Dispatch threads*/
    for (int i = 0; i < 3; ++i){
        for (int j = 0; j < 3; ++j){
            /*i*3+j returns a nice little index number, same as 2d array to pointer offset*/
            block_offset *input = malloc(sizeof(block_offset));
            input->x = i;
            input->y = j;
            pthread_create(&tid[i * 3 + j], NULL, check_block, input);
        }
    }
    /*Wait for threads to conclude*/
    for (int i = 0; i < 9; ++i){
        pthread_join(tid[i], NULL);
    }
    /*Check result*/
    for (int i = 0; i < 9; ++i){
        if (results_blocks[i] == 0){
            /*At least one result was invalid*/
            valid = 0;
            break;
        }
    }
    *results = valid;
    //printf("Blocks: %d", valid);
    pthread_exit(0);
}

/*1 thread for rows, 1 thread for cols, 9 threads for blocks. 11 worker threads, 1 dispatch thread*/
int option_1(){
    pthread_t tid_row, tid_col, tid_block;
    pthread_create(&tid_row, NULL, check_rows, NULL);
    pthread_create(&tid_col, NULL, check_cols, NULL);
    pthread_create(&tid_block, NULL, check_blocks_mt, NULL);
    /*Bring the boys back home*/
    pthread_join(tid_row, NULL);
    pthread_join(tid_col, NULL);
    pthread_join(tid_block, NULL);
    /*Check results*/
    for (int i = 0; i < 3; ++i){
        if (results_final[i] == 0){
            /*At least one result was invalid*/
            return 0;
        }
    }
    /*All were valid!*/
    return 1;
}

/*9 threads for rows, 9 threads for cols, 9 threads for blocks. 27 worker threads, 3 dispatch thread*/
int option_2(){
    pthread_t tid_row, tid_col, tid_block;
    pthread_create(&tid_row, NULL, check_rows_mt, NULL);
    pthread_create(&tid_col, NULL, check_cols_mt, NULL);
    pthread_create(&tid_block, NULL, check_blocks_mt, NULL);
    /*Bring the boys back home*/
    pthread_join(tid_row, NULL);
    pthread_join(tid_col, NULL);
    pthread_join(tid_block, NULL);
    /*Check results*/
    for (int i = 0; i < 3; ++i){
        if (results_final[i] == 0){
            /*At least one result was invalid*/
            return 0;
        }
    }
    /*All were valid!*/
    return 1;
}

int* shared_mem(){
	
	// Memory map the pointer to shared memory object
    int* mem = mmap(NULL, sizeof(int) * 3, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    /* initialize to all zeros */
    for(int i=0; i<3; i++){
        mem[i]=0;
    }
    return mem;
}

int option_3(){

    pid_t pid;
    pthread_t tid_row, tid_col, tid_block;
        
    /* pass this as argument for check- functions */
    int* memory = shared_mem();
    
    for(int i = 0; i<3; i++){
        pid = fork();

        if(pid == 0){ /* Child */
            // check rows
            if(i==1){
                //printf("checking rows\n");
                pthread_create(&tid_row, NULL, check_rows_child, &memory[0]);
                pthread_join(tid_row, NULL);
                exit(0);
            }
            else if(i==2){
                //printf("checking cols\n");
                pthread_create(&tid_col, NULL, check_cols_child, &memory[1]);
                pthread_join(tid_col, NULL);
                exit(0);
            }
            else{
                //printf("checkings blocks\n");
                pthread_create(&tid_block, NULL, check_blocks_child, &memory[2]);
                pthread_join(tid_block, NULL);
                exit(0);
            }
            
        }
        else if(pid<0){
            if(i==1){
                perror("Error in checking rows\n");
            }
            else if(i==2){
                perror("Error in checking columns\n");
            }
            else{
                perror("Error in checking blocks\n");
            }
            exit(1);
        }
    }
    for(int i = 0 ; i<3 ; i++){ /* Parent process*/
        wait(NULL);
    }
    for(int i = 0; i < 3; ++i){
        if (memory[i] == 0){
            /*At least one result was invalid*/
            munmap(memory, sizeof(int)*3); /* clean up shared mem*/
            return 0;
        }
    }

    /*All were valid!*/
    munmap(memory, sizeof(int)*3); /* clean up shared mem*/
    return 1;
}


/* Open input file and format into 2d array board */
int make_board(FILE* inputFile, int board[9][9], char line[80] ){
    if(inputFile == NULL) {
    perror("Error opening file");
    return 1;
    }

    for (int i = 0; i < 9; i++) {
        if (fgets(line, 80, inputFile) == NULL) {
            perror("Error getting line");
            fclose(inputFile);
            return 1;
        }

        // Tokenize after fgets successfully reads a line
        char* num = strtok(line, " \n");
        for (int j = 0; j < 9 && num != NULL; j++) {
            board[i][j] = atoi(num);
            num = strtok(NULL, " \n");  // Get the next number
        }
    }

    fclose(inputFile);

    /* print board to verify */
    printf("BOARD STATE IN input.txt: \n");
    for(int i =0; i<9; i++){
        for(int j=0; j<9; j++){
            printf("%d ",board[i][j]);
            if(j%3 ==2){
            printf(" ");
        }
        }
        printf("\n");
        if(i%3 ==2){
            printf("\n");
        }
    }
    return 0;
    
}
/** Thought process for t test:
 * make 3 methods to run the method 50 times and take the average of all the times
 * return the average (double)
 * -- for graphs: print the data from each run into a csv file
 * 
 * in Excel:
 * run the t tests using the given data in the CSV
 * 
 * null hyp: mean_option1 == mean option2 == mean option3 (there is no statistical difference)
 * alt hyp: there is a difference 
 * 
 * graph the means of each run on separate lines (so three lines)-- peak may or may not be similar
 * 
 */

// int run_stats(){

//     FILE* fp = fopen("timing_results.csv", "w"); /* making 3 data collection files-- seeing if running each option first has any effect*/
//     if (!fp) {
//         perror("Failed to open file");
//         return 1;
//     }

//     fprintf(fp, "Option,Run,Time(seconds),Mean Time(seconds)\n");
//     clock_t start,finish;
    
//     for (int op = 1; op <= 3; op++) {
//         double full_time = 0; // Total time for all runs in one option
//         double timings[50]; // Array to store each run's timing

//         for (int run = 0; run < 50; run++) {
            
//             start = clock();
//             /* Choose option to run based on outter for loop */
//             if (op == 1) {
//                 option_1(); 
//             } 
//             else if (op == 2) {
//                 option_2();
//             } 
//             else if (op == 3) {
//                 option_3();
//             }
//             else{
//                 perror("There was an error collecting data.\n");
//             }

//             finish = clock();
//             double run_time = ((double)(finish - start)) / CLOCKS_PER_SEC;
//             full_time += run_time;
//             timings[run] = run_time;
//         }
        

//         double mean = full_time / 50;

//         /* Write each run and the mean time only on the last run of the set */
//         for (int i = 0; i < 50; i++) {
//             /* Only print mean on the last line for each option */
//             if (i == 49) { 
//                 //printf("%d,%d,%f,%f\n", op, i + 1, timings[i], mean);
//                 fprintf(fp, "%d,%d,%f,%f\n", op, i + 1, timings[i], mean);
//             } else {
//                 fprintf(fp, "%d,%d,%f\n", op, i + 1, timings[i]);
//             }
//         }
        
        
//     }

//     fclose(fp);
//     printf("Data collected. Please check timing_results.csv for results.\n");
//     return 0;
// }

int run_stats(){
    FILE* fp = fopen("timing_results.csv", "w"); /* making 3 data collection files-- seeing if running each option first has any effect*/
    if (!fp) {
        perror("Failed to open file");
        return 1;
    }

    fprintf(fp, "Option,Run,Time(seconds),Mean Time(seconds), %d\n", getpid());
    clock_t begin,end;
    double full_time = 0;
    double timings[50];

    for(int run =0; run<50; run++){
        begin = clock();
        option_1();
        end = clock();

        double run_time = ((double)(end - begin)) / CLOCKS_PER_SEC;
        
        timings[run] = run_time;
        fprintf(fp, "%d,%d,%f\n", 1, run + 1, timings[run]);
    }
    for(int run =0; run<50; run++){
        begin = clock();
        option_2();
        end = clock();
        

        double run_time = ((double)(end - begin)) / CLOCKS_PER_SEC;
        
        timings[run] = run_time;
        fprintf(fp, "%d,%d,%f\n", 2, run + 1, timings[run]);
    }
    fclose(fp);
    // if you comment this for loop out, it prints option 1 and 2 to the csv perfectly.
    for(int run =0; run<50; run++){
        begin = clock();
        option_3();
        end = clock();
        
        double run_time = ((double)(end - begin)) / CLOCKS_PER_SEC;
        
        timings[run] = run_time;
        fp = fopen("timing_results.csv", "a");
        fprintf(fp, "%d,%d,%f\n", 3, run + 1, timings[run]);
        fclose(fp);
    }
    return 0;
}



int main(int argc, char** argv){
    /* Get option choice from user */
    int option = atoi(argv[1]);
    //int option = 1;
    FILE* inputFile = fopen("input.txt", "r");

    make_board(inputFile, board, line);

    /* Execute chosen option and get timing */
    int verdict = 2;
    clock_t begin,end;
    begin = clock();
    if(option == 1){
        verdict = option_1();
    }
    else if(option == 2){
        verdict = option_2();
    }
    else if(option == 3){
        verdict = option_3();
    }
    else{
        printf("Invalid option selected. Please select 1,2 or 3.\n");
    }
    end = clock();

    /* Printing verdict with timing*/
    double total_time = ((double) (end - begin)) / CLOCKS_PER_SEC;
    if(verdict == 1){
        printf("SOLUTION: YES(%lf seconds)\n", total_time);
    }
    else if(verdict == 0){
        printf("SOLUTION: NO(%lf seconds)\n", total_time);
    }
    else{
        printf("There was an error in determining the solution. \n");
    }

    run_stats();

    return 0;
}

/* BAD BOARD
1 2 3 4 5 6 8 9 7
1 2 3 4 5 6 8 9 7
1 2 3 4 5 6 8 9 5
1 2 3 4 5 6 8 9 5
1 2 3 4 5 6 8 9 5
1 2 3 4 5 6 8 9 5
1 2 3 4 5 6 8 9 5
1 2 3 4 5 6 8 9 5
1 2 3 4 5 6 8 9 5
*/

/* SOLUTION BOARD
4 3 5 2 6 9 7 8 1
6 8 2 5 7 1 4 9 3
1 9 7 8 3 4 5 6 2
8 2 6 1 9 5 3 4 7
3 7 4 6 8 2 9 1 5
9 5 1 7 4 3 6 2 8
5 1 9 3 2 6 8 7 4
2 4 8 9 5 7 1 3 6
7 6 3 4 1 8 2 5 9
*/
