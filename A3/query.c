#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>

#include "freq_list.h"
#include "worker.h"
// TODO: FIX DOCUMENTATION
//TODO: In case index file missing, should it fail compeletely?

/* A program to model calling run_worker and to test it. Notice that run_worker
 * produces binary output, so the output from this program to STDOUT will 
 * not be human readable.  You will need to work out how to save it and view 
 * it (or process it) so that you can confirm that your run_worker 
 * is working properly.
 */
int main(int argc, char **argv) {
    char ch;
    char path[PATHLENGTH];
    char *startdir = ".";

    /* this models using getopt to process command-line flags and arguments */
    while ((ch = getopt(argc, argv, "d:")) != -1) {
        switch (ch) {
        case 'd':
            startdir = optarg;
            break;
        default:
            break;
        }
    }

    /* For each entry in the directory, eliminate . and .., and check
     * to make sure that the entry is a directory, then call run_worker
     * to process the index file contained in the directory.
     * Note that this implementation of the query engine iterates
     * sequentially through the directories, and will expect to read
     * a word from standard input for each index it checks.
     */
    //TODO: need to handle the exception where worker is more than 10;
    
    char buffer[MAXWORD];
    FreqRecord result[MAXRECORDS];
    while (fgets(buffer, MAXWORD * sizeof(char), stdin) != NULL){
        int i = 0;
        // Use as read from child to master
        int fdr[MAXWORKERS][2];
        // Use as write from master to child
        int fdw[MAXWORKERS][2];
        // Open the directory provided by the user (or current working directory)
        DIR *dirp;
        if ((dirp = opendir(startdir)) == NULL) {
            perror("opendir");
            exit(1);
        }
        struct dirent *dp;
        while ((dp = readdir(dirp)) != NULL) {
            if (strcmp(dp->d_name, ".") == 0 ||
                strcmp(dp->d_name, "..") == 0 ||
                strcmp(dp->d_name, ".svn") == 0 ||
                strcmp(dp->d_name, ".git") == 0) {
                    continue;
            }

            strncpy(path, startdir, PATHLENGTH);
            strncat(path, "/", PATHLENGTH - strlen(path));
            strncat(path, dp->d_name, PATHLENGTH - strlen(path));
            path[PATHLENGTH - 1] = '\0';

            struct stat sbuf;
            if (stat(path, &sbuf) == -1) {
                // This should only fail if we got the path wrong
                // or we don't have permissions on this entry.
                perror("stat");
                exit(1);
            }

            // Only call run_worker if it is a directory
            // Otherwise ignore it.
            if (S_ISDIR(sbuf.st_mode)) {
                if (pipe(fdr[i]) == -1) {
                    perror("pipe");
                    exit(1);
                }
                if (pipe(fdw[i]) == -1) {
                    perror("pipe");
                    exit(1);
                }
                int r = fork();
                if (r < 0) {
                    perror("fork");
                    exit(1);
                } else if (r == 0) {
                    // Close all the readend and writeend of its parent has 
                    // opened so far.
                    for (int j = 0; j < i; j++) {
                        if (close(fdr[j][0]) == -1) {
                            perror("close");
                            exit(1);
                        }
                        if (close(fdw[j][1]) == -1) {
                            perror("close");
                            exit(1);
                        }
                    }
                    if (close(fdw[i][1]) == -1) {
                        perror("close");
                        exit(1);
                    }
                    if (close(fdr[i][0] == -1)) {
                        perror("close");
                        exit(1);
                    }
                    run_worker(path, fdw[i][0], fdr[i][1]);
                    // if run_worker returns, that means fd[i][0] is closed
                    // TODO: is the above right?
                    if (close(fdr[i][1]) == -1) {
                        perror("close");
                        exit(1);
                    }
                    exit(0);
                } else {
                    if (close(fdw[i][0]) == -1) {
                        perror("close");
                        exit(1);
                    } 
                    if (close(fdr[i][1]) == -1) {
                        perror("close");
                        exit(1);
                    }
                    i++;
                    // Reach Maximum number of worker, close file descriptors
                    // and quit after all child process quit
                    if (i == MAXWORKERS) {
                        fprintf(stderr, "Maximum number of workers exceed!\n");
                        for (int k = 0; k < i; k++) {
                            if (close(fdw[k][1]) == -1) {
                                perror("close");
                                exit(1);
                            }  
                        }
                        for (int k = 0; k < i; k++) {
                            wait(NULL);
                        }
                        exit(1);
                    }
                } 
            }
        }
        int cur_num = 0; // the current number of element in result array
        for (int k = 0; k < i; k++) {
            if (write(fdw[k][1], buffer, MAXWORD * sizeof(char)) == -1) {
                perror("write to pipe");
                exit(1);
            }
            if (close(fdw[k][1]) == -1) {
                perror("close");
                exit(1);
            }
        }
        for (int k = 0; k < i; k++) {
            int status;
            FreqRecord temp;
            wait(&status);
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                fprintf(stderr, "An Error occured!\n");
                exit(1);
            }
            while(read(fdr[k][0], &temp, sizeof(FreqRecord)) > 0) {
                if (cur_num < MAXRECORDS) {
                    result[cur_num] = temp;
                    qsort(result, cur_num, sizeof(FreqRecord), compare);
                    cur_num++;
                } else {
                    result[MAXRECORDS-1] = temp;
                    qsort(result, cur_num, sizeof(FreqRecord), compare);
                }
            }
        }
        print_freq_records(result);
        if (closedir(dirp) < 0) {
            perror("closedir");
        }
    }
    
    return 0;
}
