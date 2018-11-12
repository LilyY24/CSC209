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

/* 
 * Read a word from stdin. For each entry in the directory, eliminate
 *. and .., and check to make sure that the entry is a directory, then 
 * call run_worker to process the index file contained in the directory. 
 * The process repeat until stdin is closed.   
 */
int main(int argc, char **argv) {
    char ch;
    char path[PATHLENGTH];
    char *startdir = ".";

    int fdw[MAXWORKERS][2];
    int fdr[MAXWORKERS][2];
    /* this models using getopt to process command-line flags and arguments */
    while ((ch = getopt(argc, argv, "d:")) != -1) {
        switch (ch) {
        case 'd':
            startdir = optarg;
            break;
        default:
            fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME]\n");
            exit(1);
        }
    }

    // Open the directory provided by the user (or current working directory)
    DIR *dirp;
    if ((dirp = opendir(startdir)) == NULL) {
        perror("opendir");
        exit(1);
    }

    /* For each entry in the directory, eliminate . and .., and check
     * to make sure that the entry is a directory, then call run_worker
     * to process the index file contained in the directory.
     * Note that this implementation of the query engine iterates
     * sequentially through the directories, and will expect to read
     * a word from standard input for each index it checks.
     */
    struct dirent *dp;
    int i = 0;
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
            if (pipe(fdr[i]) == -1 || pipe(fdw[i]) == -1) {
                perror("pipe for fdr");
                exit(1);
            }
            int r = fork();
            if (r == 0) {
                //close all fd of open for master process and un-used fd for 
                // this i
                for (int k = 0; k <= i; k++) {
                    if (close(fdw[k][1]) == -1) {
                        perror("close for child");
                        exit(1);
                    }
                    if (close(fdr[k][0]) == -1) {
                        perror("close for child");
                        exit(1);
                    }
                }
                run_worker(path, fdw[i][0], fdr[i][1]);
                exit(0);
            } else if (r > 0) {
                if (close(fdw[i][0]) == -1 || close(fdr[i][1]) == -1) {
                    perror("close");
                    exit(1);
                }
                i++;
                if (i > MAXWORKERS) {
                    fprintf(stderr, "Maximum number of workers exceed!\n");
                    exit(1);
                }
            } else {
                perror("fork");
                exit(1);
            }
        }
    }
    char buffer[MAXWORD];
    while (fgets(buffer, MAXWORD, stdin) != NULL) {
        FreqRecord result[MAXRECORDS + 1];
        int result_num = 0;
        for (int k = 0; k < i; k++) {
            if (write(fdw[k][1], buffer, sizeof(char) * MAXWORD) == -1) {
                perror("write to child");
                exit(1);
            }
        }
        for (int k = 0; k < i; k++) {
            FreqRecord temp;
            if (read(fdr[k][0], &temp, sizeof(FreqRecord)) == -1) {
                perror("read from child");
                exit(1);
            }
            while (temp.freq != 0) {
                if (result_num < MAXRECORDS) {
                    result[result_num] = temp;
                    result_num++;
                    qsort(result, result_num, sizeof(FreqRecord), compare);
                } else {
                    if (result[MAXRECORDS - 1].freq < temp.freq) {
                        result[MAXRECORDS - 1] = temp;
                        qsort(result, MAXRECORDS, sizeof(FreqRecord), compare);
                    }
                }
                if (read(fdr[k][0], &temp, sizeof(FreqRecord)) == -1) {
                    perror("read from child");
                    exit(1);
                }
            }
        }
        // Add a sentinel at the end of last valid record. 
        FreqRecord temp;
        temp.freq = 0;
        result[result_num] = temp;
        print_freq_records(result);
    }
    // close all file descriptor and wait for all child process to exit
    for (int k = 0; k < i; k++) {
        if (close(fdw[k][1]) == -1) {
            perror("close");
            exit(1);
        }
        if (close(fdr[k][0]) == -1) {
            perror("close");
            exit(1);
        }
    }
    for (int k = 0; k < i; k++) {
        int status;
        wait(&status);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Child process exit abnormally\n");
            exit(1);
        }
    }
    if (closedir(dirp) < 0)
        perror("closedir");

    return 0;
}