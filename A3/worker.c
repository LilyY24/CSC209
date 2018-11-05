#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

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
 * Return an array of FreqRecoed elements for the given word. End of the valid
 * record is indicated by a FreqRecord with freq 0 and empty filename.
 */
FreqRecord *get_word(char *word, Node *head, char **file_names) {
    Node *target = find_node(word, head);
    int file_num = find_file_num(target);
    FreqRecord *result = (FreqRecord*)malloc(sizeof(FreqRecord) * (file_num + 1));
    int k = 0;
    for (int i = 0; i < MAXFILES; i++) {
        if (target->freq[i] != 0) {
            result[k].freq = target->freq[i];
            strncpy(result[k].filename, file_names[i], PATHLENGTH);
            result[k].filename[PATHLENGTH-1] = '\0';
            k++;
        }
    }
    result[file_num].freq = 0;
    result[file_num].filename[0] = '\0';
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
    return;
}

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
    int result = 0;
    for (int i = 0; i < MAXFILES; i++) {
        if (target->freq[i] != 0) {
            result++;
        }
    }
    return result;
}