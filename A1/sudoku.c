#include <stdio.h>

int check_regular_sudoku(int **puzzle);

int main() {
    // You could change these values to hand-test your function. 
    // This example is invalid. Notice that the very middle inner 3x3
    // box has a pair of 3's and a pair of 4's, no 6 and no 7.
    int row1[9] = {5, 3, 9, 2, 1, 6, 8, 7 ,4};
    int row2[9] = {7, 6, 4, 5, 8 ,3, 2, 9, 1};
    int row3[9] = {1, 8 ,2, 9, 7, 4, 5, 6, 3};
    int row4[9] = {6, 9, 3, 7, 2, 8, 4, 1, 5};
    int row5[9] = {4, 5, 7, 3, 9, 1, 6, 2, 8};
    int row6[9] = {2 ,1, 8, 4, 6, 5, 7, 3, 9};
    int row7[9] = {8, 7, 5, 6, 3, 9 ,1, 4, 2};
    int row8[9] = {3, 2, 1 ,8, 4, 7, 9, 5, 6};
    int row9[9] = {9, 4, 6, 1, 5, 2, 3, 8, 7};

    int *puzzle[9] = {row1, row2, row3, row4, row5, row6, row7, row8, row9};
    if (check_regular_sudoku(puzzle)) {
        printf("Invalid Sudoku\n");
    } else {
        printf("Valid Sudoku\n");
    }

    return 0;
}
