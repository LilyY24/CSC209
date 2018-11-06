#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>

#include "freq_list.h"
#include "worker.h"

/*
 * Find the Node in head that is for the provided word. Return NULL if there
 * if no such Node.
 */ 
Node *find_node(char *word, Node *head);

/*
 * Return the number of file that has non-zero word frenquency.
 */ 
int find_file_num(Node *target);

/*
 * Return a pointer to the string where all trailing whitespaces
 * of given word are removed.
 */
char* trim(char *word);

/*
 * Free the Node linked list at head.
 */
void free_node(Node *head);

/*
 * Return an array of FreqRecoed elements for the given word. End of the valid
 * record is indicated by a FreqRecord with freq 0 and empty filename.
 */
FreqRecord *get_word(char *word, Node *head, char **file_names) {
    Node *target = find_node(word, head);
    int file_num = find_file_num(target);
    FreqRecord *result = (FreqRecord*)malloc(sizeof(FreqRecord) * (file_num + 1));
    if (result == NULL) {
        perror("malloc for get_word");
        exit(1);
    }
    if (target == NULL) {
        result[0].freq = 0;
        result[0].filename[0] = '\0';
        return result;
    }
    int k = 0;
    for (int i = 0; i < MAXFILES; i++) {
        if (target->freq[i] != 0) {
            result[k].freq = target->freq[i];
            strncpy(result[k].filename, file_names[i], PATHLENGTH);
            result[k].filename[PATHLENGTH-1] = '\0';
            k++;
        }
    }
    result[k].freq = 0;
    result[k].filename[0] = '\0';
    return result;   
}

/* Print to standard output the frequency records for a word.
* Use this for your own testing and also for query.c
*/
void print_freq_records(FreqRecord *frp) {
    int i = 0;

    while (frp != NULL && frp[i].freq != 0) {
        printf("%d    %s\n", frp[i].freq, frp[i].filename);
        i++;
    }
}

/* Complete this function for Task 2 including writing a better comment.
*/
void run_worker(char *dirname, int in, int out) {
    Node *head = NULL;
    char **filenames = init_filenames();
    char *listfile = "/index";
    char *namefile = "/filenames";
    char listpath[PATHLENGTH];
    char namepath[PATHLENGTH];
    strncpy(listpath, dirname, PATHLENGTH);
    strncat(listpath, listfile, PATHLENGTH - strlen(listpath));
    listpath[PATHLENGTH - 1] = '\0';
    strncpy(namepath, dirname, PATHLENGTH);
    strncat(namepath, namefile, PATHLENGTH - strlen(namepath));
    namepath[PATHLENGTH - 1] = '\0';
    
    read_list(listpath, namepath, &head, filenames);
    char word[MAXWORD];
    /*TODO: Is it correct to use sizeof()? Is the read correct? */
    while (read(in, word, MAXWORD * sizeof(char)) > 0) {
        FreqRecord *result = get_word(trim(word), head, filenames);
        
        int i = 0;
        while (result != NULL) {
            if (write(out, result + i, sizeof(FreqRecord)) == -1) {
                perror("Write to out");
                exit(1);
            }
            if (result[i].freq == 0) {
                break;
            }
            i++;   
        }
        // TODO: is this right?
        free(result);
    }
    free_node(head);
    free(filenames);
}

//Following is the implementation of helper function
Node *find_node(char *word, Node *head) {
    Node *cur = head;
    while (cur != NULL) {
        if (strcmp(cur->word, word) == 0){
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

int find_file_num(Node *target) {
    if (target == NULL){
        return 0;
    }
    int result = 0;
    for (int i = 0; i < MAXFILES; i++) {
        if (target->freq[i] != 0) {
            result++;
        }
    }
    return result;
}

char* trim(char *word) {
    while (isspace(*word)){
        word++;
    }
    int len = strlen(word);
    while (len > 0 && isspace(word[len-1])) {
        word[len-1] = '\0';
        len--;
    }
    return word;
}

/*
 * Compare function to used to sort FreqRecord in descending order
 */
int compare(const void *s1, const void *s2) {
    FreqRecord *f1 = (FreqRecord*)s1;
    FreqRecord *f2 = (FreqRecord*)s2;
    return -(f1->freq - f2->freq);
}

void free_node(Node *head) {
    Node *cur = head;
    while (cur->next != NULL) {
        Node *temp = cur;
        cur = cur->next;
        free(temp);
    }
    free(cur);
}

void free_filenames(char **filenames) {
    int i = 0;
    while (filenames[i] != NULL) {
        free(filenames[i]);
        i++;
    }
    free(filenames);
}