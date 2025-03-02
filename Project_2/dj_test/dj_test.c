#include <stdio.h>


/*test*/
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

int main(){
    int board[9][9] = {{1, 2, 3, 4, 5, 6, 7, 8, 9},
                       {1, 1, 1, 1, 1, 1, 1, 1, 1},
                       {9, 8, 7, 6, 5, 4, 3, 2, 1}, 
                       {2, 1, 3, 4, 6, 5, 7, 9, 8},
                       {3, 1, 3, 4, 5, 2, 2, 2, 2},
                       {3, 3, 3, 3, 3, 3, 3, 3, 3},
                       {7, 7, 7, 7, 7, 7, 7, 6, 5},
                       {8, 8, 8, 8, 8, 8, 8, 4, 3},
                       {9, 9, 9, 9, 9, 9, 9, 1, 2}};

    printf("%d\n", check_row(board, 0));
    printf("%d\n", check_row(board, 1));
    printf("%d\n", check_col(board, 0));
    printf("%d\n", check_col(board, 5));
    printf("%d\n", check_block(board, 0, 0));
    printf("%d\n", check_block(board, 2, 2));
    return 0;
}