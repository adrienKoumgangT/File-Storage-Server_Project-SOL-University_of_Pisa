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
#include <dirent.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>

#include <assert.h>

#include "interface.h"
#include "command_handler.h"
#include "utils.h"



/************** values to be used throughout the program **************/

// possible number of command flags that the client could use
#define N_FLAGS 13
#define LEN_BUF_ARGS 27

// definition of the possible dimensions for the strings
#define STR_LEN 1024
#define STR_SIZE (STR_LEN * sizeof(char))
#define MAX_FILE_NAME 2048

// define for connection to server
#define time_to_retry (5 * 1000)
#define time_to_connect_sec 5
#define time_to_connect_nsec (time_to_connect_sec * 1000000)


typedef struct _arg_list{
    char* arg;
    struct _arg_list* next;
} arg_list;


long time_arg = 1;
int goodEnd = 1;

int hasCmdF = 0;
cmd* cmdF = NULL;
int isConnect = 0;
int print_operation = 0;
long timeToPrint = 1;
long timeToPause = 0;


int flag_f = 0;

int flag_D = 0;
int flag_w_W = 0;

int flag_r = 0;
int flag_R = 0;

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


/************* functions related to commands passed by the client ************/

/**
* function that requires the client to connect to a server
*
* @params sockname : address of the server to connect to
*
* @returns :    -1
*               0
*/
int connect_to_server( char* sockname ){
    if(!sockname){
        fprintf(stdout, "Error: wrong address! try again...\n");
        return -1;
    }

    int len_sock = strlen(sockname);
    if(len_sock >= PATH_MAX){
        fprintf(stdout,
            "Error: wrong adress! the given address is too long\n try again...\n");
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

/****************************** utility functions *************************/



/******** management functions of each command passed by the client *********/

int isdot(const char dir[]) {
  int l = strlen(dir);

  if ( (l>0 && dir[l-1] == '.') ) return 1;
  return 0;
}

arg_list* listOfFile( const char* nomedir, arg_list** list, int* n ){
    if(*n == 0) return NULL;

    // controllo se il parametro sia una directory
    struct stat statbuf;
    int err;
    SYSCALL_RETURN_VAL_EQ("stat", err, stat(nomedir, &statbuf), -1, NULL, "");

    DIR *dir;
    if((dir = opendir(nomedir)) == NULL){
        perror("opendir");
        fprintf(stderr, "ERROR: error opening directory '%s'\n", nomedir);
        return NULL;
    }else{
        struct dirent* file;
        arg_list* last = *list;

        while(*n-- != 0 && (errno = 0, file = readdir(dir)) != NULL){
            struct stat statbuf;
            char filename[MAX_FILE_NAME];
            int len1 = strlen(nomedir);
            int len2 = strlen(file->d_name);
            if((len1 + len2 + 2) > MAX_FILE_NAME){
                fprintf(stderr, "ERROR: MAX_FILE_NAME too small : %d\n", MAX_FILE_NAME);
                return NULL;
            }
            strncpy(filename, nomedir, MAX_FILE_NAME-1);
            strncat(filename, "/", MAX_FILE_NAME-1);
            strncat(filename, file->d_name, MAX_FILE_NAME-1);

            if(stat(filename, &statbuf)==-1) {
		      perror("eseguendo la stat");
		      fprintf(stderr, "ERROR: Error in file %s\n", filename);
		      return NULL;
	        }

	        if(S_ISDIR(statbuf.st_mode)){
		              if ( !isdot(filename) ){
                          last = listOfFile(filename, &last, n);
                          if(last == NULL) return NULL;
                      }
	        }else{
                arg_list* new_file = (arg_list *) malloc(sizeof(arg_list));
                int len = strlen(file->d_name);
                new_file->arg = (char *) malloc((len+1) * sizeof(char));
                memset(new_file->arg, '\0', len+1);
                strncpy(new_file->arg, file->d_name, len+1);
                new_file->next = NULL;
                if(last == NULL) *list = new_file;
                else last->next = new_file;
                last = new_file;
	       }
        }
        if (errno != 0) perror("readdir");
	    closedir(dir);
        return last;
    }
}

// -w dirname[,n=0] : invia al server i file nella cartella ‘dirname’,
// ovvero effettua una richiesta di scrittura al server per i file
int do_cmd_w( char* args, char* dirname ){
    if(!args) return -1;

    arg_list* list_files = NULL;
    int hasComma = 0;
    int i=0;
    //int count=-1;
    while((args[i] != '\0') && (args[i] != ',')) i++;
    if(args[i] == ',') hasComma = 1;

    if(!hasComma){
        int n = -1;
        if(listOfFile(dirname, &list_files, &n) != NULL){
            arg_list* elem = list_files;
            arg_list* prev = NULL;
            while(elem != NULL){
                if(writeFile(elem->arg, dirname) == -1){
                    fprintf(stderr, "ERROR: Failure to write the file '%s' to the server\n", args);
                    perror("writeFile");
                }
                prev = elem;
                elem = elem->next;
                free(prev->arg);
                free(prev);
            }
        }
    }else{/*
        arg_list* list_args = NULL;
        int n = 0;
        if((n = parse_arguments(args, &list_args)) == -1){
            fprintf(stderr, "ERROR: Bad parameter value for parsing\n");
            return -1;
        }

        if(n != 2){
            fprintf(stderr, "ERROR: Bad argument for comand '-w'\n");
            return -1;
        }*/

    }

    return 0;
}

// -W file1[,file2] lista dei file da scrivere sul server
int do_cmd_W( char* args, char* dirname ){
    if(!args) return -1;

    // arg_list* list_args = NULL;
    int hasComma = 0;
    int i=0;
    while((args[i] != '\0') && (args[i] != ',')) i++;
    if(args[i] == ',') hasComma = 1;

    if(!hasComma){
        if(writeFile(args, dirname) == -1){
            fprintf(stderr, "ERROR: Failure to write the file '%s' to the server\n", args);
            perror("writeFile");
            return -1;
        }
    }else{/*
        int n = 0;
        if((n = parse_arguments(args, &list_args)) != 0){
            while(list_args){
                arg_list* corr = list_args;
                list_args = list_args->next;
                if(writeFile(corr->arg, dirname) == -1){
                    fprintf(stderr, "ERROR: Failure to write the file '%s' to the server\n", corr->arg);
                    perror("writeFile");
                    free(corr->arg);
                    free(corr);
                    while(list_args){
                        corr = list_args;
                        list_args = list_args->next;
                        free(corr->arg);
                        free(corr);
                    }
                    return -1;
                }
                free(corr->arg);
                free(corr);
            }
        }*/
    }

    return 0;
}

/********************************* main client ******************************/

/*
#define cmd_w (0)
#define cmd_W (1)
#define cmd_D (3)
#define cmd_r (4)
#define cmd_R (5)
#define cmd_d (6)
#define cmd_tt (7)
#define cmd_l (8)
#define cmd_u (9)
#define cmd_c (10)

#define cmd_p (11)
#define cmd_f (12)
#define cmd_h (13)
*/

int main(int argc, char** argv){

    if(argc < 2){
        fprintf(stderr, "FATAL ERROR: this program is used with at least one argument.\nTry '%s -h' for a list and details\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int i=0;

    if(initCmds(argc, argv) == EXIT_FAILURE){
        char** s = getErrors();
        for(i=0; i<number_of_errors; i++){
            if(s[i]) fprintf(stderr, "%s\n", s[i]);
        }
        finish();
        return EXIT_FAILURE;
    }

    if(hasHCmd()){
        print_help();
        goto endClient;
    }

    if(hasPCmd()){
        print_operation = 1;
        fprintf(stdout, "INFO: print operations enable\n\n");
    }

    char* sockname = getF();
    if(print_operation) fprintf(stdout, "[%ld] Attempt to connect to the server at: '%s'.\n", timeToPrint++, sockname);
    if(connect_to_server(sockname) == -1){
        goodEnd = 0;
        goto endClient;
    }
    if(print_operation) fprintf(stdout, "[%ld] Connection to server successful.\n", timeToPrint)


    cmd* mycmd = NULL;

    while((mycmd = nextCmd()) != NULL){
        switch(mycmd->command){
            case cmd_w:{
                break;
            }
            case cmd_W:{
                break;
            }
            case cmd_D:{
                break;
            }
            case cmd_r:{
                break;
            }
            case cmd_R:{
                break;
            }
            case cmd_d:{
                break;
            }
            case cmd_tt:{

            }
            case cmd_l:{
                break;
            }
            case cmd_u:{
                break;
            }
            case cmd_c:{
                break;
            }
        }
    }

    endClient:
        finish();

    if(!goodEnd) return EXIT_FAILURE;
    else return EXIT_SUCCESS;
}
