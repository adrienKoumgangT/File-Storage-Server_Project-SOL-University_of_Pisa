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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>

#include <assert.h>

#include "client.h"
#include "interface.h"
#include "utils.h"

// define for all programs
#define STR_LEN 80
#define STR_SIZE (STR_LEN * sizeof(char))
#define N_FLAGS 13
#define LEN_BUF_ARGS 27

// define for connection to server
#define time_to_retry (5 * 1000)
#define time_to_connect_sec 5
#define time_to_connect_nsec (time_to_connect_sec * 1_000_000)

//char *socket_name;
//int fd_skt;
//struct sockaddr_un serv_addr;

int flag_h = 0;
int flag_f = 0;
int flag_p = 0;
int flag_tt = 0;

struct timespec time_dodo;
time_dodo.tv_sec = 0;
time_dodo.tv_nsec = 0;

// help messages related to each terminal line command
char help[]   = "-h -f filename -w dirname[,n=0] -W file1[,file2] -D dirname -r file1[,file2] -R [n=0] -d dirname -t time -l file1[file2] -u file1[,file2] -c file1[,file2] -p";
char help_h[] = "-h : print the list of all accepted options";
char help_f[] = "-f filename : specifies the name of the AF_UNIX socket to connect to";
char help_w[] = "-w dirname[,n=0] : send the files in the 'dirname' folder to the server";
char help_W[] = "-W file1[,file2] : list of file names to be written to the server";
char help_D[] = "-D dirname : folder in secondary memory where the files that the server removes following a capacity miss are written";
char help_r[] = "-r file1[,file2] : list of file names to be read by the server";
char help_R[] = "-R [n=0] : allows you to read 'n' files currently stored on the server";
char help_d[] = "-d dirname : folder in secondary memory where to write the files read by the server with the '-r' or '-R' option";
char help_tt[] = "-t time : time in milliseconds between sending two successive requests to the server";
char help_l[] = "-l file1[,file2] : list of file names on which to acquire the mutual exclusion";
char help_u[] = "-u file1[,file2] : list of file names on which to release the mutual exclusion";
char help_c[] = "-c file1[,file2] : list of files to be removed from the server if any";
char help_p[] = "-p : enable standard output printouts";

/**
* prints a detailed message related to each server command
*/
void print_help(){
    fprintf(stdout, "User manual to communicate with the server\n");
    fprintf(stdout, "%s\n\n", help);
    fprintf(stdout, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
                    help_h,
                    help_f,
                    help_w,
                    help_W,
                    help_D,
                    help_r,
                    help_R,
                    help_d,
                    help_tt,
                    help_l,
                    help_u,
                    help_c,
                    help_p
            );
}

/**
* check if the client is connected to a server or not
*
* @returns : -1 if the client is not connected to a server
*             0 otherwise
*/
int control_configuration_connection(){
    if(!flag_f){
        fprintf(stdout, "Error : connection to socket not yet configured\n");
        fprintf(stdout, "          Write '-h' for more details\n");
        fflush(stdout);
        return -1;
    }

    return 0;
}

/**
* connects the client to a server
*
* @params args : string containing the name of the server socket to connect to
*
* @returns : -1 if there has been an error while processing the command
*             0 if successful
*/
int cmd_f( char *args ){
    int len_sock = strlen(args);
    socket_name = (char *) malloc((len_sock + 1) * sizeof(char));
    strncpy(socket_name, optarg, len_sock+1);
    struct timespec abstime = {0, 0};
    if(clock_gettime(CLOCK_REALTIME, &abstime) == -1){

    }
    abstime.tv_sec += time_to_connect_sec;
    abstime.tv_nsec += time_to_connect_nsec;

    if( openConnection(socket_name, time_to_retry, abstime) == -1){

    }

}

/**
* function that allows to handle the command '-w'
*
* @params args : arguments to the '-w' command
*
* @returns : -2 if the client is not yet connected to a server
*            -1 if there has been an error while processing the command
*             0 if successful
*/
int cmd_w( char *args ){
    if(control_configuration_connection() != 0)
        return -2;

}

/**
* function that allows to handle the command '-W'
*
* @params args : arguments to the '-W' command
*
* @returns : -2 if the client is not yet connected to a server
*            -1 if there has been an error while processing the command
*             0 if successful
*/
int cmd_W( char *args ){
    if(control_configuration_connection() != 0)
        return -2;
}

/**
* function that allows to handle the command '-D'
*
* @params args : arguments to the '-D' command
*
* @returns : -2 if the client is not yet connected to a server
*            -1 if there has been an error while processing the command
*             0 if successful
*/
int cmd_D( char *args ){
    if(control_configuration_connection() != 0)
        return -2;
}

/**
* function that allows to handle the command '-r'
*
* @params args : arguments to the '-r' command
*
* @returns : -2 if the client is not yet connected to a server
*            -1 if there has been an error while processing the command
*             0 if successful
*/
int cmd_r( char *args ){
    if(control_configuration_connection() != 0)
        return -2;
}

/**
* function that allows to handle the command '-R'
*
* @params args : arguments to the '-R' command
*
* @returns : -2 if the client is not yet connected to a server
*            -1 if there has been an error while processing the command
*             0 if successful
*/
int cmd_R( char *args ){
    if(control_configuration_connection() != 0)
        return -2;
}

/**
* function that allows to handle the command '-d'
*
* @params args : arguments to the '-d' command
*
* @returns : -2 if the client is not yet connected to a server
*            -1 if there has been an error while processing the command
*             0 if successful
*/
int cmd_d( char *args ){
    if(control_configuration_connection() != 0)
        return -2;
}

/**
* function that sets the time (in milliseconds) that elapses between
* the sending of two successive requests to the server
*
* @param args : string containing the time
*/
void cmd_tt( char *args ){
    if(control_configuration_connection() != 0)
        return;

    if(args == NULL){
        time_dodo.tv_nsec = 0;
        return;
    }

    long new_time = getNumber(args);
    if(new_time < 0){
        fprintf(stderr, "Error : invalid argument: default value = 0\n");
        return;
    }

    time_dodo.tv_nsec = new_time * 1_000_000;
}

/**
* function that allows to handle the command '-l'
*
* @params args : arguments to the '-l' command
*
* @returns : -2 if the client is not yet connected to a server
*            -1 if there has been an error while processing the command
*             0 if successful
*/
int cmd_l( char *args ){
    if(control_configuration_connection() != 0)
        return -2;
}

/**
* function that allows to handle the command '-u'
*
* @params args : arguments to the '-u' command
*
* @returns : -2 if the client is not yet connected to a server
*            -1 if there has been an error while processing the command
*             0 if successful
*/
int cmd_u( char *args ){
    if(control_configuration_connection() != 0)
        return -2;
}

/**
* function that allows to handle the command '-c'
*
* @params args : arguments to the '-c' command
*
* @returns : -2 if the client is not yet connected to a server
*            -1 if there has been an error while processing the command
*             0 if successful
*/
int cmd_c( char *args ){
    if(control_configuration_connection() != 0)
        return -2;
}

/**
* function that allows me to extract client commands from a string
*
* @param buffer_input : string containing all client commands
* @param buffer_output : array of strings containing commands
*                            and their arguments
*
* @returns : the number of strings written to the output buffer on success
*           -1 to error
*/
int parse_arguments( char* buffer_input, char** buffer_output ){

    int i = 1;
    char *tmp;
    char *token = strtok_r(buffer_input, " ", &tmp);

    while(token){
        int len = strlen(token);
        buffer_output[i] = (char *) malloc((len + 1) * sizeof(char) );
        memset(buffer_output[i], '\0', sizeof(char));

        strncpy(buffer_output[i], token, len+1);

        token = strtok_r(NULL, " ", &tmp),
        i++;
    }

    int num_str = i;

    buffer_output[i-1][strcspn(buffer_output[i-1], "\n")] = '\0';

//    while(i<LEN_BUF_ARGS-1){
//        NULL;
//        i++;

    buffer_output[LEN_BUF_ARGS-1] = NULL;

    return num_str;
}

/**
* client startup function
*/
void run_client(){

    int opt;
    int n;
    char buffer_input[STR_LEN];
    char *cmd_args[LEN_BUF_ARGS];

    cmd_args[0] = "client";

    while(1){
        memset(buffer_input, '\0', sizeof(char));

        if(fgets(buffer_input, STR_LEN, stdin) == NULL)
            continue;

        n = parse_arguments(buffer_input, cmd_args);
        if(n == 1)
            continue;

        optind = 1;
        opterr = 0;

        while((opt = getopt(n, cmd_args, ":hf:w:W:D:r:R:d:t:l:u:c:p")) != -1){
            switch(opt){
                case 'h':
                    flag_h = 1;
                    print_help();
                    break;
                case 'p':
                    if(!flag_p){
                        flag_p = 1;
                    }else{
                        fprintf(stdout, "Attention : prints on the output for each operation already enabled\n");
                    }
                    break;
                case 'f':
                    if(!flag_f){
                        flag_f = 1;
                        cmd_f(optarg);
                    }else{
                        fprintf(stderr, "Error : the socket name has already been set (%s)\n", socket_name);
                    }
                    break;
                case 't':
                    cmd_tt(optarg);
                    break;
                case 'w':
                    cmd_w(optarg);
                    break;
                case 'W':
                    cmd_W(optarg);
                    break;
                case 'D':
                    cmd_D(optarg);
                    break;
                case 'r':
                    cmd_r(optarg);
                    break;
                case 'R':
                    cmd_R(optarg);
                    break;
                case 'd':
                    cmd_d(optarg);
                    break;
                case 'l':
                    cmd_l(optarg);
                    break;
                case 'u':
                    cmd_u(optarg);
                    break;
                case 'c':
                    cmd_c(optarg);
                    break;
                case ':':
                    switch(optopt){
                        case 'R':
                            cmd_R( NULL );
                            break;
                        case 't':
                            cmd_tt( NULL );
                            break;
                        default:
                            fprintf(stderr, "l'opzione '-%c' richiede un argomento\n",
                                            optopt);
                    }
                    break;
                default: /* case '?': */
                    fprintf(stderr, "Error: command not recognized\n");
            }
        }

        for(int i=1; i<n; i++){
            if(cmd_args[i]){
                free(cmd_args[i]);
                cmd_args[i] = NULL;
            }
        }

    }

}
