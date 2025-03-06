#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*unsolved*/
// int board[9][9] = {{1, 2, 3, 4, 5, 6, 7, 8, 9},
//                    {1, 1, 1, 1, 1, 1, 1, 1, 1},
//                    {9, 8, 7, 6, 5, 4, 3, 2, 1}, 
//                    {2, 1, 3, 4, 6, 5, 7, 9, 8},
//                    {3, 1, 3, 4, 5, 2, 2, 2, 2},
//                    {3, 3, 3, 3, 3, 3, 3, 3, 3},
//                    {7, 7, 7, 7, 7, 7, 7, 6, 5},
//                    {8, 8, 8, 8, 8, 8, 8, 4, 3},
//                    {9, 9, 9, 9, 9, 9, 9, 1, 2}};

/*solved*/
int board[9][9] = {{4, 3, 5, 2, 6, 9, 7, 8, 1},
                   {6, 8, 2, 5, 7, 1, 4, 9, 3},
                   {1, 9, 7, 8, 3, 4, 5, 6, 2}, 
                   {8, 2, 6, 1, 9, 5, 3, 4, 7},
                   {3, 7, 4, 6, 8, 2, 9, 1, 5},
                   {9, 5, 1, 7, 4, 3, 6, 2, 8},
                   {5, 1, 9, 3, 2, 6, 8, 7, 4},
                   {2, 4, 8, 9, 5, 7, 1, 3, 6},
                   {7, 6, 3, 4, 1, 8, 2, 5, 9}};
int option = 1;
/*Results for multithreaded functions are recorded in these global arrays*/
int results_rows[9];
int results_cols[9];
int results_blocks[9];
/*Rows is index 0, columns is index 1, blocks is index 2*/
int results_final[3];

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
    results_rows[y] = check_valid(arr);
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
    results_cols[x] = check_valid(arr);
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
    results_blocks[x + 3 * y] = check_valid(arr);
    pthread_exit(0);
}

/*Checks all rows in one thread*/
void *check_rows(){
    for (int i = 0; i < 9; ++i){
        if (check_row(i) == 0){
            results_final[0] = 0;
            pthread_exit(0);
        }
    }
    results_final[0] = 1;
    pthread_exit(0);
}

/*Checks all columns in one thread*/
void *check_cols(){
    for (int i = 0; i < 9; ++i){
        if (check_col(i) == 0){
            results_final[1] = 0;
            pthread_exit(0);
        }
    }
    results_final[1] = 1;
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
            results_final[0] = 0;
            pthread_exit(0);
        }
    }
    /*All results were valid!*/
    results_final[0] = 1;
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
        if (results_rows[i] == 0){
            /*At least one result was invalid*/
            results_final[1] = 0;
            pthread_exit(0);
        }
    }
    /*All results were valid!*/
    results_final[1] = 1;
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
            results_final[2] = 0;
            pthread_exit(0);
        }
    }
    /*All results were valid!*/
    results_final[2] = 1;
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

int main(){
    printf("%d\n", option_1());
    return 0;
}