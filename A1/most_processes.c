#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int handle_without_argument(char** content, int length);

int handle_with_argument(char** content, int pid, int length);

char* get_name(char* line);

int get_pid(char* line);

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

    //Handle the input accordingly
    if (arg == 1){
        handle_without_argument(content, used);
    } else{
        handle_with_argument(content, (int)argv[1], used);
    }
    return 0;
}

int handle_without_argument(char** content, int length){
    char** names = (char**)malloc(sizeof(char*) * length);
    int numbers[length];
    int cursor = 0;
    int num_people = 0;
    // init numbers to 0;
    for (int i = 0; i < length; i++){
        numbers[i] = 0;
    }
    for (int i = 0; i < length; i++){
        if (i == 0){
            names[0] = get_name(content[0]);
            numbers[0]++;
            num_people++;
        } else if (strcmp(get_name(content[i-1]), get_name(content[i]))){
            numbers[cursor]++;
        } else{
            cursor++;
            num_people++;
            names[cursor] = get_name(content[i]);
            numbers[cursor]++;
        }
    }
    int winner_cursor = 0;
    for (int i =0; i < num_people; i++){
        if (numbers[i] > numbers[winner_cursor]){
            winner_cursor = i;
        }
    }
    printf("%s %d", *names[winner_cursor], numbers[winner_cursor]);
    return 0;
}

int handle_with_argument(char** content, int pid, int length){
    pid++;
    return "a";
}

