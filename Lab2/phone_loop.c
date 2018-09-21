#include <stdio.h>
#include <stdlib.h>

int main(){
    char phone[11];
    int num;
    int has_error = 0;
    scanf("%s", phone);
    while(scanf("%d", &num) != EOF){
        if (num < -1 || num > 9){
            printf("ERROR\n");
            has_error = 1;
        } else{
            if (num == -1){
                printf("%s\n", phone);
            } else{
                printf("%c\n", phone[num]);
            }
        }
    }
    if (has_error){
        return 1;
    }
    return 0;
}