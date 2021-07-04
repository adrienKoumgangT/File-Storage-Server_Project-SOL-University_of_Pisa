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

// #define _POSIX_C_SOURCE 200112L

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
#include "read_write_file.h"
#include "utils.h"

extern char *realpath(const char *, char *);
extern int usleep(useconds_t);

/************** values to be used throughout the program **************/

// possible number of command flags that the client could use
#define N_FLAGS 13
#define LEN_BUF_ARGS 27

// define for connection to server
#define time_to_retry (5 * 1000)
#define time_to_connect_sec 5
#define time_to_connect_nsec (time_to_connect_sec * 1000000)

char PATHNAME[2048];
int LEN_PATHNAME = 0;

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

char dirname_D[STR_LEN];
char dirname_d[STR_LEN];

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
        if(errno == EINVAL) fprintf(stderr, "ERROR: Failed to connect. The connection address given is incorrect (%s).\n", socket_name);
        if(errno == ETIME) fprintf(stderr, "ERROR: Failed to connect. the server took too long to answer the connection request.\n");
        if(socket_name) free(socket_name);
        return -1;
    }

    if(socket_name) free(socket_name);

    return 0;
}

/****************************** utility functions *************************/

/*
char* getAbsolutePath( char* path ){
    if(!path) return NULL;
    size_t len = strlen(path);
    if(len <= 0) return NULL;

    // if the path is already absolute, I return it
    if(path[0] == '/') return path;

    if(len + LEN_PATHNAME + 1 > MAX_FILE_NAME) return NULL;
    char* new_path = (char *) malloc((len + LEN_PATHNAME + 1) * sizeof(char));
    memset(new_path, '\0', len+LEN_PATHNAME+1);
    strncpy(new_path, PATHNAME, LEN_PATHNAME+1);
    strncat(new_path, path, len+1);
    return new_path;
}
*/

int isdot(const char dir[]) {
  int l = strlen(dir);

  if ( (l>0 && dir[l-1] == '.') ) return 1;
  return 0;
}

arg_list* listOfFile( const char* nomedir, arg_list** list, long* n ){
    if(!n || *n == -1) return NULL;

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

        while(*n != 0 && (errno = 0, file = readdir(dir)) != NULL){
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
                *n = *n -1;
                arg_list* new_file = (arg_list *) malloc(sizeof(arg_list));
                new_file->arg = (char *) malloc(MAX_FILE_NAME * sizeof(char));
                memset(new_file->arg, '\0', MAX_FILE_NAME);
                strncpy(new_file->arg, nomedir, MAX_FILE_NAME-1);
                strncat(new_file->arg, "/", MAX_FILE_NAME-1);
                strncat(new_file->arg, file->d_name, MAX_FILE_NAME-1);
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

/******** management functions of each command passed by the client *********/


/**
* "-w" command management function
* takes care of sending a certain number of files ("n" if n> 0 or all)
*   contained in the "arg" folder
*
* @params arg : argument passed to the command
* @params n : second argument passed to the command
*
* @returns : 0 if successful
*           -1 in case of failure
*/
int do_cmd_w( char* arg, long n ){
    if(!arg) return -1;

    char mdir[MAX_FILE_NAME];
    if(realpath(arg, mdir) == NULL){
        fprintf(stderr, "ERROR: invalid path name for folder %s\n", arg);
        return -1;
    }

    arg_list* flist = NULL;
    if(n == 0) n = -2;
    if(listOfFile(mdir, &flist, &n) == NULL) return -1;
    arg_list *corr = flist;
    arg_list *prev = NULL;
    while(corr != NULL){
        if(openFile(corr->arg, O_CREATE_LOCK) == -1){

        }else if(writeFile(corr->arg, dirname_D) == -1){

        }
        prev = corr;
        corr = corr->next;
        if(prev->arg) free(prev->arg);
        free(prev);
    }

    return 0;
}

/**
* "-W" command management function
* writes "n" files (args) to the server
*
* @params args : argument passed to the command
* @params n : second argument passed to the command
*
* @returns : 0 if successful
*           -1 in case of failure
*/
int do_cmd_W( char** args, long n ){
    if(!args || n<=0) return -1;

    if(print_operation) fprintf(stdout, "[%ld] writing '%ld' files to the server\n", timeToPrint++, n);
    for(int i=0; i<n; i++){
        char path[MAX_FILE_NAME];
        if(realpath(args[i], path) == NULL){
            fprintf(stderr, "ERROR: invalid pathname '%s'\n", args[i]);
            continue;
        }else{
            if(print_operation) fprintf(stdout, "[%ld] written of file '%s' to the server\n", timeToPrint++, path);
        }
        if(openFile(path, O_CREATE_LOCK) == -1){
            switch(errno){
                case EINVAL:{
                    break;
                }
                case EOPNOTSUPP:{
                    break;
                }
            }
        }else if(writeFile(path, dirname_D) == -1){

        }
    }

    return 0;
}

/**
* "-D" command management function
* change the folder where the files ejected from the server are stored
*
* @params arg : argument passed to the command
*
* @returns : 0 if successful
*           -1 in case of failure
*/
int do_cmd_D( char* arg ){
    if(print_operation) fprintf(stdout, "[%ld] change the save folder for files ejected from the server for the '-w' and '-W' commands : %s\n", timeToPrint++ ,arg);
    memset(dirname_D, '\0', STR_LEN);
    if(realpath(arg, dirname_D) == NULL){
        fprintf(stderr, "ERROR: invalid path name for folder '%s'\n", arg);
        return -1;
    }
    if(print_operation) fprintf(stdout, "[%ld] the save folder for files ejected from the server for the '-w' and '-W' commands now is : %s\n", timeToPrint++, dirname_D);
    return 0;
}

/**
* "-r" command management function
* it reads "n" files (args) present on the server
*
* @params args : argument passed to the command
* @params n : second argument passed to the command
*
* @returns : 0 if successful
*           -1 in case of failure
*/
int do_cmd_r( char** args, long n ){
    if(!args || n<=0) return -1;

    if(print_operation) fprintf(stdout, "[%ld] reading '%ld' files to the server\n", timeToPrint++, n);
    char* buf_read = NULL;

    for(int i=0; i<n; i++){
        char path[MAX_FILE_NAME];
        if(realpath(args[i], path) == NULL){
            fprintf(stderr, "ERROR: invalid pathname '%s'\n", args[i]);
            continue;
        }

        size_t sz;
        if(openFile(path, O_CREATE_LOCK) == -1){

        }else if(readFile(path, (void **) &buf_read, &sz) == -1){

        }else{
            char* p = NULL;
            if(write_file(p, buf_read, sz) == -1){

            }
        }
        fprintf(stdout, "contents of file '%s' read on server:%s\n", path, buf_read);
        if(buf_read) free(buf_read);
    }

    return 0;
}

/**
* "-R" command management function
* it reads any "arg" files present on the server
*
* @params arg : argument passed to the command
*
* @returns : 0 if successful
*           -1 in case of failure
*/
int do_cmd_R( long arg ){
    if(print_operation){
        if(arg<=0) fprintf(stdout, "[%ld] reading any n files from the server\n", timeToPrint++);
        else fprintf(stdout, "[%ld] reading any '%ld' files from the server\n", timeToPrint++, arg);
    }
    if(readNFile((int) arg, dirname_d) == -1){
        if(arg<=0) fprintf(stdout, "[%ld] failed to read any n files from the server\n", timeToPrint++);
        else fprintf(stdout, "[%ld] failed to read any '%ld' files from the server\n", timeToPrint++, arg);
        return -1;
    }
    if(print_operation){
        if(arg<=0) fprintf(stdout, "[%ld] successful reading any n files from the server\n", timeToPrint++);
        else fprintf(stdout, "[%ld] successful reading any '%ld' files from the server\n", timeToPrint++, arg);
    }
    return 0;
}

/**
* "-d" command management function
* change the folder where the files ejected from the server are stored
*
* @params arg : argument passed to the command
*
* @returns : 0 if successful
*           -1 in case of failure
*/
int do_cmd_d( char* arg ){
    if(print_operation) fprintf(stdout, "[%ld] change the save folder for files ejected from the server for the '-r' and '-R' commands.\n", timeToPrint++);
    memset(dirname_d, '\0', STR_LEN);
    if(realpath(arg, dirname_d) == NULL){
        fprintf(stderr, "ERROR: invalid path name for folder '%s'\n", arg);
        return -1;
    }
    if(print_operation) fprintf(stdout, "[%ld] the save folder for files ejected from the server for the '-r' and '-R' commands now is : %s\n", timeToPrint++, dirname_d);
    return 0;
}

/**
* "-t" command management function
* change the waiting time between 2 requests made to the server
*
* @params arg : argument passed to the command
*/
void do_cmd_tt( long arg ){
    if(print_operation) fprintf(stdout, "[%ld] change of waiting time between 2 requests : %ld milliseconds\n", timeToPrint++, arg);
    timeToPause = arg;
}

/**
* "-l" command management function
* requests the server to acquire logs on "n" files (args)
*
* @params arg : argument passed to the command
* @params n : second argument passed to the command
*
* @returns : 0 if successful
*           -1 in case of failure
*/
int do_cmd_l( char** args, long n ){
    for(int i=0; i<n; i++){
        char path[MAX_FILE_NAME];
        if(realpath(args[i], path) == NULL){
            fprintf(stderr, "ERROR: invalid pathname '%s'\n", args[i]);
            continue;
        }
        if(print_operation) fprintf(stdout, "[%ld] acquisition of the mutual exclusion on the file '%s'\n", timeToPrint++, path);
        if(lockFile(path) == -1){
            if(print_operation) fprintf(stdout, "[%ld] acquisition of mutual exclusion on file '%s' failed\n", timeToPrint++, path);
        }else{
            if(print_operation) fprintf(stdout, "[%ld] successful acquisition of mutual exclusion on file '%s'\n", timeToPrint++, path);
        }
    }
    return 0;
}

/**
* "-u" command management function
* prompts the server to release log capture on "n" files (args)
*
* @params arg : argument passed to the command
* @params n : second argument passed to the command
*
* @returns : 0 if successful
*           -1 in case of failure
*/
int do_cmd_u( char** args, long n ){
    for(int i=0; i<n; i++){
        char path[MAX_FILE_NAME];
        if(realpath(args[i], path) == NULL){
            fprintf(stderr, "ERROR: invalid pathname '%s'\n", args[i]);
            continue;
        }
        if(print_operation) fprintf(stdout, "[%ld] releasing of the mutual exclusion on the file '%s'\n", timeToPrint++, path);
        if(unlockFile(args[i]) == -1){
            if(print_operation) fprintf(stdout, "[%ld] releasing of mutual exclusion on file '%s' failed\n", timeToPrint++, path);
        }else{
            if(print_operation) fprintf(stdout, "[%ld] successful releasing of mutual exclusion on file '%s'\n", timeToPrint++, path);
        }
    }
    return 0;
}

/**
* "-c" command management function
* requests the server to close "n" files (args) opened by the client
*
* @params arg : argument passed to the command
* @params n : second argument passed to the command
*
* @returns : 0 if successful
*           -1 in case of failure
*/
int do_cmd_c( char** args, long n ){
    for(int i=0; i<n; i++){
        char path[MAX_FILE_NAME];
        if(realpath(args[i], path) == NULL){
            fprintf(stderr, "ERROR: invalid pathname '%s'\n", args[i]);
            continue;
        }
        if(print_operation) fprintf(stdout, "[%ld] removing the '%s' file on the server\n", timeToPrint++, path);
        if(removeFile(args[i]) == -1){
            if(print_operation) fprintf(stdout, "[%ld] removing the '%s' file on the server failed\n", timeToPrint++, path);
        }else{
            if(print_operation) fprintf(stdout, "[%ld] successful removing the '%s' file on the server\n", timeToPrint++, path);
        }
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

    //int err;
    char* p_err = NULL;

    memset(PATHNAME, '\0', 2048);
    SYSCALL_EXIT_EQ("realpath", p_err, realpath(".", PATHNAME), NULL, "");
    LEN_PATHNAME = strlen(PATHNAME);

    int i=0;

    // gives the "command handler" arguments passed on the command line
    // it checks the correctness of the commands
    if(initCmds(argc, argv) == EXIT_FAILURE){
        char** s = getErrors();
        for(i=0; i<number_of_errors; i++){
            if(s[i]) fprintf(stderr, "%s\n", s[i]);
        }
        finish();
        return EXIT_FAILURE;
    }

    // verify that the user passed the "-h" command or not
    if(hasHCmd()){
        print_help();
        goto endClient;
    }

    // verify that the user passed the "-p" command or not
    if(hasPCmd()){
        print_operation = 1;
        fprintf(stdout, "INFO: print operations enable\n\n");
    }

    // configuration and connection to the server at the given address
    char* sockname = getF();
    if(print_operation) fprintf(stdout, "[%ld] Attempt to connect to the server at: '%s'.\n", timeToPrint++, sockname);
    if(connect_to_server(sockname) == -1){
        goodEnd = 0;
        goto endClient;
    }
    if(print_operation) fprintf(stdout, "[%ld] Connection to server successful.\n", timeToPrint);

    char* mdir = getFirstDdir();
    if(mdir){
        if((LEN_PATHNAME + strlen(mdir) +1) > 2048){
            fprintf(stderr, "invalid path name for folder '%s': too long\n", mdir);
            goto endClient;
        }
        memset(dirname_D, '\0', STR_LEN);
        strncat(dirname_D, mdir, STR_LEN);
    }
    mdir = getFirstddir();
    if(mdir){
        if((LEN_PATHNAME + strlen(mdir) +1) > 2048){
            fprintf(stderr, "invalid path name for folder '%s': too long\n", mdir);
            goto endClient;
        }
        memset(dirname_d, '\0', STR_LEN);
        strncpy(dirname_d, mdir, STR_LEN);
    }
    mdir = NULL;

    cmd* mycmd = NULL;

    // sequential processing of each command
    while((mycmd = nextCmd()) != NULL){
        switch(mycmd->command){
            case cmd_w:{
                if(do_cmd_w(mycmd->list_of_arguments[0], mycmd->intArg) == -1){

                }
                break;
            }
            case cmd_W:{
                if(do_cmd_W(mycmd->list_of_arguments, mycmd->countArgs) == -1){

                }
                break;
            }
            case cmd_D:{
                if(do_cmd_D(mycmd->list_of_arguments[0]) == -1){
                    goto endClient;
                }
                break;
            }
            case cmd_r:{
                if(do_cmd_r(mycmd->list_of_arguments, mycmd->countArgs) == -1){

                }
                break;
            }
            case cmd_R:{
                if(do_cmd_R(mycmd->intArg) == -1){

                }
                break;
            }
            case cmd_d:{
                if(do_cmd_d(mycmd->list_of_arguments[0]) == -1){
                    goto endClient;
                }
                break;
            }
            case cmd_tt:{
                do_cmd_tt(mycmd->intArg);
                break;
            }
            case cmd_l:{
                if(do_cmd_l(mycmd->list_of_arguments, mycmd->countArgs) == -1){

                }
                break;
            }
            case cmd_u:{
                if(do_cmd_u(mycmd->list_of_arguments, mycmd->countArgs) == -1){

                }
                break;
            }
            case cmd_c:{
                if(do_cmd_c(mycmd->list_of_arguments, mycmd->countArgs) == -1){

                }
                break;
            }
            default:{
                if(print_operation) fprintf(stderr, "[%ld] FATAL ERROR: error while running the client.\n", timeToPrint++);
                goto endClient;
            }
        }

        usleep(timeToPause * 1000);
    }

    endClient:
        if(print_operation) fprintf(stdout, "[%ld] close the connection with server : '%s'\n", timeToPrint++, sockname);
        if(isConnect) closeConnection(sockname);
        finish();

    if(!goodEnd) return EXIT_FAILURE;
    else return EXIT_SUCCESS;
}
