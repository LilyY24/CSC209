#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "freq_list.h"
#include "worker.h"

int main(int argc, char **argv) {
    Node *head = NULL;
    char **filenames = init_filenames();
    char arg;
    char *listfile = "index";
    char *namefile = "filenames";

    /* an example of using getop to process command-line flags and arguments */
    while ((arg = getopt(argc,argv,"i:n:")) > 0) {
        switch(arg) {
        case 'i':
            listfile = optarg;
            break;
        case 'n':
            namefile = optarg;
            break;
        default:
            fprintf(stderr, "Usage: printindex [-i FILE] [-n FILE]\n");
            exit(1);
        }
    }

    read_list(listfile, namefile, &head, filenames);
    char temp[MAXWORD];
    while (fgets(temp, MAXWORD, stdin) != NULL) {
        FreqRecord *result = get_word(trim(temp), head, filenames);
        print_freq_records(result);
        free(result);
    }
    free_filenames(filenames);
    free_node(head);
    return 0;
}
