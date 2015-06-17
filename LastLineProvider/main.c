/*
 * Copyright (C) 2014, Komputerowiec, <komputerowiec.net@gmail.com>,
 *
 * https://github.com/komputerowiec/
 *
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <string.h>
#include <regex.h>
#include "configuration.h"

/***********************************************************
 * print_usage()
 */
void print_usage()
{
    printf("lastlineprovider version %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
    printf("Description:\n");
    printf("\n");
    printf("Read text from standard input, line by line, and provides the most recent one through file specified with -o option. Additionlaly, by default, each read line is being rewrited to the standard output.\n");
    printf("\n");
    printf("Usage:\n");
    printf("\n");
    printf("lastlineprovider -h\n");
    printf("lastlineprovider -o <output-file-path> [-f <regex-filter>] [-l] [-n] [-m] [-s]\n");
    printf("\n");
    printf("Options:\n");
    printf("\n");
    printf("-o <output-file-path>, specifies the path to the output file which provides the content of the last line read from standard input.\n");
    printf("\n");
    printf("-f <regex-filter>, if set, only lines which match the <regex-filter> regular expression will be provided through the output file specified with -o option.\n");
    printf("\n");
    printf("-l, if set, before any new line is written to the <output-file-path> file, the file is beaing locked with the LOCK_EX operation of the flock(2) system call, the lock is being released after the whole line is written to the file\n");
    printf("\n");
    printf("-n, no standard output, i.e. the lines read from standard input are not copied to the standard output, so only <output-file-path> file is being updated\n");
    printf("\n");
    printf("-s, sychronize each write to the <output-file-path>, i.e. call fsync() after each line is written\n");
    printf("\n");
    printf("-m, matching only, i.e. to the standard output copy only lines matching the <regex-filter> filter (or all lines if the filter is not defined)\n");
    printf("\n");

} // print_usage()




/*************************************************************
 * main()
 */
int main(int argc, char *argv[])
{
    /*
     * options to be set from CLI
     */
    char *opt_output_file = NULL;
    char *opt_filter_regex_str = NULL;
    int opt_lock_output_file = 0;
    int opt_no_stdout = 0;
    int opt_sync = 0;
    int opt_matching_stdout = 0;

    /*
     * function scope, auxiliary variables
     */
    int err = 0;
    int match_result = 0;
    regex_t filter_regex;

    /*
     * analyze and set CLI options
     */
    if(argc == 1) {
        print_usage();
        exit(1);
    }

    int i = 0;
    for(i=0;i<argc;i++) {


        if(strncmp(argv[i], "-h", 2) == 0) {

            print_usage();
            exit(0);

        } // if(strncmp)


        if(strncmp(argv[i], "-o", 2) == 0) {

            if(i + 1 < argc) {
                opt_output_file = argv[i+1];
            }

        } // if(strncmp)

        if(strncmp(argv[i], "-f", 2) == 0) {

            if(i + 1 < argc) {
                opt_filter_regex_str = argv[i+1];
            }

        } // fi(strncmp)


        if(strncmp(argv[i], "-l", 2) == 0) {

            opt_lock_output_file = 1;

        } // if(strncmp)


        if(strncmp(argv[i], "-n", 2) == 0) {

            opt_no_stdout = 1;

        } // if(strncmp)


        if(strncmp(argv[i], "-m", 2) == 0) {

            opt_matching_stdout = 1;

        } // if(strncmp)


        if(strncmp(argv[i], "-s", 2) == 0) {

            opt_sync = 1;

        } // if(strncmp)


    } // for()


    /*
     * check if mandatory options has been set
     */
    if(opt_output_file == NULL) {

        fprintf(stderr, "ERROR: Output file must be defined. Use -h and see -o option.\n");
        exit(1);

    } // if()


    /*
     * compile the filter regular expression
     */
     if (opt_filter_regex_str != NULL) {

        err = regcomp(&filter_regex, opt_filter_regex_str, REG_EXTENDED);
        if(0 != err) {
            perror("Failed to use the filter regular expression. Probably the format of the expression is wrong.");
            exit(1);
        }

     } // if()


    /*
     * open output file
     */
    int fd = open(opt_output_file, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);


    /*
     * main loop, read standard input line by line and put it to output file
     * if filter matches the read line
     */
    char *lineptr = NULL;
    ssize_t read_char_nbr = 0;
    size_t  line_buf_size = 0;


    while((read_char_nbr = getline(&lineptr, &line_buf_size, stdin))) {


        if(!opt_no_stdout && !opt_matching_stdout) {
            printf("%s", lineptr);
        }
        //printf("Read line: %s\n", lineptr);

        /*
         * check if read line matches the user defined filter
         */
        if(opt_filter_regex_str != NULL) {

            // apply the filter

            match_result = regexec(&filter_regex, lineptr, 0, NULL, 0);
            if(match_result != 0) {
                continue;
            }

        } // if()

        // lock the output file for writing
        if(opt_lock_output_file) {
            err = flock(fd, LOCK_EX);
            if(0 != err) {
                perror("Failed to set exlusive lock on the output file.");
                exit(1);
            }

        } // if(opt_lock_output_file)

        // shrink the output file
        err = ftruncate(fd, 0);
        if(0 != err) {
            perror("Failed to shrink the output file.");
            exit(1);
        }


        /*
         * write date to the output file
         */
        ssize_t wb = 0;

        do {

            wb += pwrite(fd, lineptr + wb, read_char_nbr - wb, wb);
            if(-1 == 0) {
                perror("Failed to write data to the output file.");
                exit(1);
            }

        } while (wb < read_char_nbr);

        // sync the output file (if required)
        if(opt_sync) {
            err = fsync(fd);
            if(0 != err) {
                perror("Failed to fsync the output file.");
                exit(1);
            }
        } // if(opt_sync)

        // unlock the output file
        if(opt_lock_output_file) {

            err = flock(fd, LOCK_UN);
            if(0 != err) {
                perror("Failed to unlock the output file.");
                exit(1);
            }

        } // if(opt_lock_output_file)

        // print the output also to the stdout (if appropriate CLI option is set)
        if(opt_matching_stdout) {
            printf("%s", lineptr);
        }

    } // while()

    /*
     * close output file
     */
    err = close(fd);
    if(0 != err) {
        perror("Failed to close output file.");
    }

    /*
     * release the allocated memory
     */
    if(lineptr)  {
        free(lineptr);
        lineptr = NULL;
    }

    return 0;


} // main()
