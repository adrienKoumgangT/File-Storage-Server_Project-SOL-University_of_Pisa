/*
* MIT License
*
* Copyright (c) 2021 Adrien Koumgang Tegantchouang
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

/**
* @file client.c
*
* definition of utility functions for client
*
* @author adrien koumgang tegantchouang
* @version 1.0
* @date 00/05/2021
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>

#include <assert.h>

#include "interface.h"
#include "utils.h"



/************** values to be used throughout the program **************/

// possible number of command flags that the client could use
#define N_FLAGS 13
#define LEN_BUF_ARGS 27

// definition of the possible dimensions for the strings
#define STR_LEN 80
#define STR_SIZE (STR_LEN * sizeof(char))

// define for connection to server
#define time_to_retry (5 * 1000)
#define time_to_connect_sec 5
#define time_to_connect_nsec (time_to_connect_sec * 1000000)

//char *socket_name;
//int fd_skt;
//struct sockaddr_un serv_addr;

int isConnect = 0;
int print_operation = 0;
long timeToPause = 0;


int flag_f = 0;

int flag_r = 0;
int flag_R = 0;
int flag_d = 0;
int flags_r_d = 0;
int flags_R_d = 0;

struct timespec time_dodo;

/**
* prints a detailed message related to each server command
*/
void print_help(){
    fprintf(stdout, "User manual to communicate with the server\n");
    fprintf(stdout, "%s\n\n", "-h -f filename -w dirname[,n=0] -W file1[,file2] -D dirname -r file1[,file2] -R [n=0] -d dirname -t time -l file1[file2] -u file1[,file2] -c file1[,file2] -p");
    fprintf(stdout, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
                    "-h : print the list of all accepted options",
                    "-f filename : specifies the name of the AF_UNIX socket to connect to",
                    "-w dirname[,n=0] : send the files in the 'dirname' folder to the server",
                    "-W file1[,file2] : list of file names to be written to the server",
                    "-D dirname : folder in secondary memory where the files that the server removes following a capacity miss are written",
                    "-r file1[,file2] : list of file names to be read by the server",
                    "-R [n=0] : allows you to read 'n' files currently stored on the server",
                    "-d dirname : folder in secondary memory where to write the files read by the server with the '-r' or '-R' option",
                    "-t time : time in milliseconds between sending two successive requests to the server",
                    "-l file1[,file2] : list of file names on which to acquire the mutual exclusion",
                    "-u file1[,file2] : list of file names on which to release the mutual exclusion",
                    "-c file1[,file2] : list of files to be removed from the server if any",
                    "-p : enable standard output printouts"
            );
    fflush(stdout);
}


int connect_to_server( char* sockname ){
    if(!sockname){
        fprintf(stdout, "Error: wrong address! try again...\n");
        return -1;
    }

    int len_sock = strlen(sockname);
    if(len_sock >= PATH_MAX){
        fprintf(stdout,
            "Error: wrong adress! the given address is too long\n try again...");
        return -1;
    }

    char *socket_name = (char *) malloc((len_sock + 1) * sizeof(char));
    memset(socket_name, '\0', len_sock+1);
    strncpy(socket_name, sockname, len_sock+1);
    struct timespec abstime = {0, 0};
    if(clock_gettime(CLOCK_REALTIME, &abstime) == -1){

    }
    abstime.tv_sec += time_to_connect_sec;
    abstime.tv_nsec += time_to_connect_nsec;

    if( openConnection(socket_name, time_to_retry, abstime) == -1){
        if(socket_name) free(socket_name);
        if(errno == EINVAL)
            isConnect = 0;
        return -1;
    }

    return 0;
}


/****************************** main client **********************/

int main(int argc, char** argv){

    if(argc < 2){
        fprintf(stderr, "FATAL ERROR: this program is used with at least one argument.\nTry '%s -h' for a list and details\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char sockname[STR_LEN];
    memset(sockname, '\0', STR_LEN);

    int i;
    for(i=0; i<argc; i++){
        if(strncmp(argv[i], "-h", 2) == 0){
            print_help();
            exit(EXIT_SUCCESS);
        }else if(strncmp(argv[i], "-f", 2) == 0){
            if(!isConnect){ // if the client has not yet been connected to a server
                isConnect = 1;
                if(argv[i][2] != '\0'){ // if the server address is attached to the command '-f'
                    strncpy(sockname, (argv[i] + 2), STR_LEN);
                }else{
                    if((i+1 < argc) && strncmp(argv[i+1], "-", 1) != 0){ // if the server address is contained in the following argument, the command '-f'
                        strncpy(sockname, argv[i+1], STR_LEN);
                    }else{
                        fprintf(stderr, "FATAL ERROR: problem with the server address (socket name) to connect to. try more again\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }else{
                fprintf(stderr, "the client is already connected to a server with address: %s\n", sockname);
            }
        }else if(strncmp(argv[i], "-p", 2) == 0){
            print_operation = 1;
        } else if(strncmp(argv[i], "-t", 2) == 0){
            if(!timeToPause){
                char *s = (char *) malloc(STR_LEN * sizeof(char));
                memset(s, '\0', STR_LEN);
                if(argv[i][2] != '\0'){
                    strncpy(s, (argv[i] + 2), STR_LEN);
                    timeToPause = getNumber(s, 10);
                }else{
                    if((i+1 < argc) && (strncmp(argv[i+1], "-", 1) != 0)){
                        strncpy(s, argv[i+1], STR_LEN);
                        timeToPause = getNumber(s, 10);
                    }
                }
                if(s) free(s);
            }
        }

        //if(flag_f && print_operation) break;
    }

    if(!isConnect){
        fprintf(stderr, "FATAL ERROR: no server connection command was given.\nTry '%s -h' for a list and details\n", argv[0]);
        exit(EXIT_FAILURE);
    }else{
        if(connect_to_server(sockname) < 0){
            perror("connect_to_server");
            exit(EXIT_FAILURE);
        }
    }
    extern char* optarg;
    int opt;
    optind = 0;
    opterr = 0;

    while((opt = getopt(argc, argv, ":hf:w:W:D:r:R:d:t:l:u:c:p")) != -1){
        switch(opt){
            case 'f':
            case 'h':
            case 'p':
            case 't':
                break;
            case 'w':
                printf("w\n");
                //cmd_w(optarg);
                break;
            case 'W':
                printf("W\n");
                //cmd_W(optarg);
                break;
            case 'D':
                printf("D\n");
                //cmd_D(optarg);
                break;
            case 'r':
                printf("r\n");
                //cmd_r(optarg);
                break;
            case 'R':
                printf("R\n");
                //cmd_R(optarg);
                break;
            case 'd':
                printf("d\n");
                //cmd_d(optarg);
                break;
            case 'l':
                printf("l\n");
                //cmd_l(optarg);
                break;
            case 'u':
                printf("u\n");
                //cmd_u(optarg);
                break;
            case 'c':
                printf("c\n");
                //cmd_c(optarg);
                break;
            case ':':
                switch(optopt){
                    case 'R':
                        //cmd_R( NULL );
                        break;
                    case 't':
                        //cmd_tt( NULL );
                        break;
                    default:
                        fprintf(stderr,
                            "l'opzione '-%c' richiede un argomento\n",
                                        optopt);
                }
                break;
            default: /* case '?': */
                fprintf(stderr, "Error: command not recognized\n");
        }
    }



    return EXIT_SUCCESS;
}
