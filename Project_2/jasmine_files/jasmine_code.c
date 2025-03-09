#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <wait.h>

/* DJ CODE HERE ==============================*/

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
int check_row(int board[9][9], int y){
    if (y >= 9) {
        /*Invalid offset*/
        return -1;
    }
    int arr[9];
    for (int i = 0; i < 9; ++i){
        //arr[i] = **(board + i + (y * 9));
        arr[i] = (board[y][i]);
    }
    return check_valid(arr);
}

/*Checks if a column on the sudoku board is valid*/
int check_col(int board[9][9], int x){
    if (x >= 9) {
        /*Invalid offset*/
        return -1;
    }
    int arr[9];
    for (int i = 0; i < 9; ++i){
        //arr[i] = **(board + x + (i * 9));
        arr[i] = (board[i][x]);
    }
    return check_valid(arr);
}

/*Checks if a 3x3 square is valid*/
int check_block(int board[9][9], int x, int y){
    if ((x >= 3) | (y >= 3)) {
        /*Invalid offset*/
        return -1;
    }
    x *= 3;
    y *= 3;
    int arr[9];
    int index = 0;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j){
            arr[index] = board[i + y][j + x];
            ++index;
        }
    }
    return check_valid(arr);
}

/* DJ CODE END HERE =======================================*/

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
    return 0;
    
}


/* Global variables for board and line in board */
    char line[80];  // Buffer to hold each line
    int board[9][9]; // Empty sudoku board

/* Main function */
int main(int argc, char** argv) {
    FILE* inputFile = fopen("input.txt", "r");



    make_board(inputFile, board, line);
    
    /* print board to verify */
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