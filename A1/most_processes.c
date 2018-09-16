#include <stdio.h>
#include <stdlib.h>

char* handle_without_argument(char** content);

char* handle_with_argument(char** content, int pid);

int main(int arg, char **argv){
    if (arg >= 3){
        printf("USAGE: most_processes [ppid]\n");
        return 1;
    }
    // Read from stdin
    char **content = (char**)malloc(sizeof(char*)*2);
    int i = 1;
    int used = 0;
    char* temp = (char*)malloc(sizeof(char)*1024);
    while ((fgets(temp, 1024, stdin) != NULL)){
        content[used] = temp;
        used++;
        free(temp);
        temp = (char*)malloc(sizeof(char)*1024);
        if (i == used){
            i = i * 2;
            content = (char**)realloc(content, sizeof(char*) * i );
        }
    }

    // Handele the input accordingly
    // if (arg == 1){
    //     printf("%s", handle_without_argument(content));
    // } else{
    //     printf("%s", handle_with_argument(content, (int)argv[1]));
    // }
    return 0;
}

char* handle_without_argument(char** content){
    return "a";
}

char* handle_with_argument(char** content, int pid){
    pid++;
    return "a";
}

