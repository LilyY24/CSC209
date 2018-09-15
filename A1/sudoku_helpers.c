#include <stdio.h>
#include <stdlib.h>

/*
 * Function that compare the value of two integer.
 */
int cmp(const void* num1, const void* num2){
    return *(int*)num1 - *(int*)num2;
}

/* Each of the n elements of array elements, is the address of an
 * array of n integers.
 * Return 0 if every integer is between 1 and n^2 and all
 * n^2 integers are unique, otherwise return 1.
 */
int check_group(int **elements, int n) {
    int all_element[n*n];
    // the position of the int that should be put in the all_element array
    int cur_pos = 0; 
    for (int i = 0; i < n; i++){
        int* this_line;
        this_line = elements[i];
        for (int j = 0; j < n; j++){
            all_element[cur_pos] = this_line[j];
            cur_pos++; 
        }
    }
    // Sort the array with all the element to determine whether there are duplicates
    qsort(&all_element, n*n, sizeof(int), cmp);
    for (int i = 0; i < n*n - 1; i++){
        if (all_element[i] == all_element[i+1]){
            return 1;
        }
    }
    return 0;
}

/* puzzle is a 9x9 sudoku, represented as a 1D array of 9 pointers
 * each of which points to a 1D array of 9 integers.
 * Return 0 if puzzle is a valid sudoku and 1 otherwise. You must
 * only use check_group to determine this by calling it on
 * each row, each column and each of the 9 inner 3x3 squares
 */
int check_regular_sudoku(int **puzzle) {
    // check each row
    for (int i = 0; i < 9; i++){
        int* group[] = {&((puzzle[i])[0]), &((puzzle[i])[3]),&((puzzle[i])[6])};
        if (check_group(group, 3)){
            return 1;
        }
    }
    // check each column
    for (int i = 0; i < 9; i++){
        int* group[3];
        int temp[9];
        for (int j =0; j < 9; j++){
            temp[j] = puzzle[j][i];
        }
        group[0] = &temp[0];
        group[1] = &temp[3];
        group[2] = &temp[6];
        if (check_group(group, 3)){
            return 2;
        }
    }
    // check each 3x3 squares
    for (int i = 0; i < 9; i++){
        int* group[3];
        int block_row = i / 3;
        int block_column = i - (i / 3) * block_row;
        int temp[9];
        int cursor = 0;
        for (int j = 0; j < 3; j++){
            for (int k = 0; k < 3; k++){
                temp[cursor] = puzzle[block_row * 3 + j][block_column * 3 + k];
                cursor++;
            }
        }
        group[0] = &temp[0];
        group[1] = &temp[3];
        group[2] = &temp[6];
        if (check_group(group, 3)){
            for (int i = 0; i<9; i++){
                printf("%d", temp[i]);
            }
            printf("block_row %d\n", block_row);
            return 3;
        }
    }
    
    return 0;
}
