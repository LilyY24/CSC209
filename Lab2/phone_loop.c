#include <stdio.h>
#include <stdlib.h>

int main(){
    char phone[11];
    int num;
    scanf("%s", phone);
    while(scanf("%d", &num) != EOF){
        if (num < -1 || num > 9){
            printf("ERROR\n");
        } else{
            if (num == -1){
                printf("%s\n", phone);
            } else{
                printf("%c\n", phone[num]);
            }
        }
    }
    return 0;
}