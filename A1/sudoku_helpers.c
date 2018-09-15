#include <stdio.h>
#include <stdlib.h>

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

    // TODO: replace this return statement with a real function body
    return 0;
}

int main(int argc, char const *argv[])
{
    int array1[] = {1, 2, 3};
    int array2[] = {4, 5, 6};
    int array3[] = {7, 8, 9};
    int* element[] = {array1, array2, array3};
    printf("%d", check_group(element, 3));
    return 0;
}

