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
* @file interface.c
*
* definition of utility functions for client
*
* @author adrien koumgang tegantchouang
* @version 1.0
* @date 00/05/2021
*/


// TODO: riguardare tutte le funzioni : la parte di ritorno di ogni funzione

#define _POSIX_C_SOURCE  200112L  // needed for S_ISSOCK

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>
#include <dirent.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#include <pwd.h>
#include <grp.h>
#include <stddef.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <assert.h>

//#include "utils.h"
#include "interface.h"
#include "communication.h"
#include "read_write_file.h"
#include "utils.h"

extern char* rindex(const char*, int);

#define PRINT_OPERATION
#define PRINT_INFORMATION
#define PRINT_REASON

static int fd_sock;
static struct sockaddr_un serv_addr;
static int operation = -1;
static int result = -1;
long bytes_read;
long bytes_write;



/**
* An AF_UNIX connection is opened to the socket file sockname
*
* @param sockname : name of the socket to connect to
* @param msec : time to repeat to retry the connection with
*                       the server if it fails
* @param abstime : time until you try to connect to the server
*
* @returns : 0 if successful
*            -1 if the request fails and errno is set
*
* errno :
*   EINVAL => in case of invalid parameter
*   ETIME => in case of timer expired
*/
int openConnection( const char* sockname, int msec,
                                        const struct timespec abstime ){
    if(!sockname || (msec < 0)){
        errno = EINVAL;
        return -1;
    }

    if((fd_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
        return -1;

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, sockname, strlen(sockname)+1);

    struct timespec time_sleep;
    time_sleep.tv_sec = 0;
    time_sleep.tv_nsec = msec * 1000000;
    struct timespec time_request;
    time_request.tv_sec = 0;
    time_request.tv_nsec = msec * 1000000;

    struct timespec current_time;
    memset(&current_time, '0', sizeof(current_time));

    do{
        if(connect(fd_sock, (struct sockaddr*) &serv_addr,
                                        sizeof(serv_addr)) != -1)
            return 0;

        nanosleep(&time_sleep, &time_request);

        if(clock_gettime(CLOCK_REALTIME, &current_time) == -1){
            #ifdef PRINT_INFORMATION
                fprintf(stderr, "Information: open connection operation failed, reason: problem in calculating the current time (clock_gettime)\n");
            #endif
            return -1;
        }

    }while(current_time.tv_sec < abstime.tv_sec ||
        current_time.tv_nsec < abstime.tv_nsec);

    #ifdef PRINT_INFORMATION
        fprintf(stderr, "Information: open connection operation failed, reason: connection test time passed\n");
    #endif
    errno =  ETIME;
    return -1;
}

/**
* closes the established connection between the client and the server
*
* @params sockname : the name of the socket on which the client was connected
*
* @returns : 0 if successful
*            -1 if the request fails and errno is set
*
* errno :
*   EINVAL => in case of invalid parameter
*/
int closeConnection( const char* sockname ){
    if(!sockname){
        #ifdef PRINT_INFORMATION
            fprintf(stderr, "Information: close connection operation failed, reason: wrong socket name\n");
        #endif
        errno = EINVAL;
        return -1;
    }

    if(strncmp(serv_addr.sun_path, sockname, strlen(sockname)+1) ==  0){
        operation = _CC_O;
        // I write the operation to do
        if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
            return -1;
        }

        /* receiving the response to the 'closeConnection' request to the server */
        result = -1;

        if((readn(fd_sock, (void *) &result, sizeof(int))) == -1){
            return -1;
        }

        if( result == SUCCESS_O){
            #ifdef PRINT_INFORMATION
                fprintf(stderr, "Information:the connection was successfully closed!\n");
            #endif
            errno = EFAULT;
            return -1;
        }else{
            #ifdef PRINT_INFORMATION
                fprintf(stderr, "Information: close connection operation failed!\n");
            #endif
            errno = EFAULT;
            return -1;
        }

        return close(fd_sock);
    }else{
        #ifdef PRINT_INFORMATION
            fprintf(stderr, "Information: close connection operation failed, reason: wrong socket name\n");
        #endif
        errno = EFAULT;
        return -1;
    }
}

/**
* API function that allows the client to ask the server
*       to create or open a file
*
*
*
* @returns : 0 if successful
*            -1 if the request fails and errno is set
*
* errno :
*   EINVAL => in case of invalid parameter
*   EACCES => in case of an error response from the server
*/
int openFile( const char* pathname, int flags ){
    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    switch( flags ){
        case O_CREATE:{
            #ifdef PRINT_INFORMATION
                //fprintf(stdout, "Information: open file operation with flag 'O_CREATE'\n");
            #endif
            break;
        }
        case O_LOCK:{
            #ifdef PRINT_INFORMATION
                //fprintf(stdout, "Information: open file operation with flag 'O_LOCK'\n");
            #endif
            break;
        }
        case O_CREATE_LOCK:{
            #ifdef PRINT_INFORMATION
                //fprintf(stdout, "Information: open file operation with flag 'O_CREATE_LOCK'\n");
            #endif
            break;
        }
        default:{
            #ifdef PRINT_INFORMATION
                //fprintf(stderr, "Information: open file operation failed, reason: invalid opening flag (%d)\n", flags);
            #endif
            errno = EOPNOTSUPP;
            return -1;
        }
    }

    size_t sz_p = strlen(pathname)+1;
    if(sz_p <= 1){
        errno = EINVAL;
        return -1;
    }

    /******* sending the 'openFile' request to the server ******/

    operation = _OF_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    // I specify the type of operation to be done
    if((writen(fd_sock, (void *) &flags, sizeof(int))) == -1){
        return -1;
    }

    if(sz_p <= 0){
        errno = EFAULT;
        return -1;
    }

    if((writen(fd_sock, (void *) &sz_p, sizeof(size_t))) == -1){
        return -1;
    }

    if((writen(fd_sock, (void *) pathname, sz_p)) == -1){
        return -1;
    }

    /*** receiving the response to the 'open File' request to the server ***/

    result = -1;
    if((readn(fd_sock, (void *) &result, sizeof(int))) == -1){
        return -1;
    }

    if(result == FAILED_O){
        char* reason = NULL;
        size_t sz_r = 0;
        if((readn(fd_sock, (void *) &sz_r, sizeof(size_t))) == -1){
            return -1;
        }

        reason = (char *) malloc(sz_r);
        memset(reason, '\0', sz_r);
        if((readn(fd_sock, (void *) reason, sz_r)) == -1){
            return -1;
        }

        #ifdef PRINT_REASON
            fprintf(stderr, "Information: open file operation failed, reason: %s\n", reason);
        #endif

        free(reason);
        errno = EACCES;
        return -1;
    }

    return 0;

}

int readFile( const char* pathname, void** buf, size_t* size ){
    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    operation = _RF_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    /************* sending the 'readFile' request to the server ************/
    size_t sz_p = strlen(pathname)+1;
    if(sz_p <= 1){
        buf = NULL;
        *size = 0;
        errno = EFAULT;
        return -1;
    }

    if((writen(fd_sock, (void *) &sz_p, sizeof(size_t))) == -1){
        *buf = NULL;
        *size = 0;
        return -1;
    }

    if((writen(fd_sock, (void *) pathname, sz_p)) == -1){
        *buf = NULL;
        *size = 0;
        return -1;
    }

    /***** receiving the response to the 'readFile' request to the server *****/
    result = -1;

    if((readn(fd_sock, &result, sizeof(int))) == -1){
        *buf = NULL;
        *size = 0;
        return -1;
    }
    if(result == SUCCESS_O){
        if((readn(fd_sock, (void *) size, sizeof(size_t))) == -1){
            *buf = NULL;
            *size = 0;
            return -1;
        }

        *buf = malloc(*size);
        memset(*buf, '\0', *size);
        if((readn(fd_sock, (void *) *buf, *size)) == -1){
            *buf = NULL;
            *size = 0;
            return -1;
        }

    }else{
        *buf = NULL;
        *size = 0;
        char* reason = NULL;
        size_t sz_r = 0;
        if((readn(fd_sock, (void *) &sz_r, sizeof(size_t))) == -1){
            return -1;
        }

        reason = (char *) malloc(sz_r);
        memset(reason, '\0', sz_r);
        if((readn(fd_sock, (void *) reason, sz_r)) == -1){
            return -1;
        }

        #ifdef PRINT_REASON
            if(reason != NULL){
                fprintf(stdout, "failure to read file '%s': %s\n", pathname, reason);
            }
        #endif
        if(reason) free(reason);
        return -1;
    }

    return 0;
}

int readNFile( int N, const char* dirname ){
    /* sending the 'readNFile' request to the server */
    /* receiving the response to the 'readNFile' request to the server */
    if(!dirname){
        errno = EINVAL;
        return -1;
    }

    operation = _RNF_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    if((writen(fd_sock, (void *) &N, sizeof(int))) == -1){
        return -1;
    }

    /***** receiving the response to the 'readFile' request to the server *****/
    int n = 0;

    if((readn(fd_sock, &n, sizeof(int))) == -1){
        return -1;
    }

    if(n <= 0){
        char* reason = NULL;
        size_t sz_r = 0;
        if((readn(fd_sock, (void *) &sz_r, sizeof(size_t))) == -1){
            return -1;
        }

        reason = (char *) malloc(sz_r);
        memset(reason, '\0', sz_r);
        if((readn(fd_sock, (void *) reason, sz_r)) == -1){
            return -1;
        }
        #ifdef PRINT_REASON
            if(reason != NULL){
                fprintf(stdout, "failure to read file: %s\n", reason);
            }
        #endif
    }else{
            char* path_r   = NULL;
            void* data_r   = NULL;
            size_t sz_pr   = 0;
            size_t sz_dr   = 0;
            int fin = 0;
            int i = 0;
            while(i < n){
                if((readn(fd_sock, (void *) &fin, sizeof(int))) == -1){
                    return -1;
                }
                if(fin) break;

                if((readn(fd_sock, (void *) &sz_pr, sizeof(size_t))) == -1){
                    return -1;
                }

                path_r = (char *) malloc(sz_pr);
                memset(path_r, '\0', sz_pr);
                if((readn(fd_sock, (void *) path_r, sz_pr)) == -1){
                    if(path_r) free(path_r);
                    return -1;
                }

                if((readn(fd_sock, (void *) &sz_dr, sizeof(size_t))) == -1){
                    return -1;
                }

                data_r = (void *) malloc(sz_dr);
                memset(data_r, '0', sz_dr);
                if((readn(fd_sock, (void *) data_r, sz_dr)) == -1){
                    if(path_r) free(path_r);
                    if(data_r) free(data_r);
                    return -1;
                }

                char* p = NULL;
                p = getNameFile(path_r);
                if(p == NULL){
                    continue;
                }
                char* final_p = NULL;
                final_p = setNameFile(dirname, p);
                if(write_file(final_p, data_r, sz_dr) == -1){

                }
                if(final_p) free(final_p);
                if(path_r) free(path_r);
                if(data_r) free(data_r);

                i++;
            }
    }

    return 0;
}

int writeFile( const char* pathname, const char* dirname ){
    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    size_t sz_p = strlen(pathname)+1;
    if(sz_p <= 1){
        errno = EFAULT;
        return -1;
    }
    char *data = NULL;
    size_t sz_d = 0;
    if(read_file(pathname, (void **) &data, &sz_d, O_RDONLY) <= 0){
        if(data) free(data);
        return -1;
    }

    if(sz_d <= 0){
        return -1;
    }

    operation = _WF_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    /* sending the 'writeFile' request to the server */
    if((write_pathname(fd_sock, (void *) pathname, sz_p)) == -1){
        return -1;
    }
    /*if((writen(fd_sock, (void *) &sz_p, sizeof(size_t))) == -1){
        return -1;
    }
    if((writen(fd_sock, (void *) pathname, sz_p)) == -1){
        return -1;
    }*/

    if((write_data(fd_sock, (void *) data, sz_d)) == -1){
        return -1;
    }

    /*if((writen(fd_sock, (void *) &sz_d, sizeof(size_t))) == -1){
        return -1;
    }
    if((writen(fd_sock, (void *) data, sz_d)) == -1){
        return -1;
    }*/

    if(data) free(data);

    /* receiving the response to the 'writeFile' request to the server */
    result = -1;

    if((readn(fd_sock, (void *) &result, sizeof(int))) == -1){
        return -1;
    }

    if(result == SUCCESS_O){
        int N = 0;

        if((readn(fd_sock, (void *) &N, sizeof(int))) == -1){
            return -1;
        }
        if(N > 0){
            #ifdef PRINT_INFORMATION
                fprintf(stderr, "Information: the write file operation caused the removal of '%d' files from the server.\n", N);
            #endif
            char* path_r   = NULL;
            char* data_r   = NULL;
            size_t sz_pr   = 0;
            size_t sz_dr   = 0;

            for(int i=0; i<N; i++){
                if((readn(fd_sock, (void *) &sz_pr, sizeof(size_t))) == -1){
                    return -1;
                }
                path_r = malloc(sz_pr);
                memset(path_r, '\0', sz_pr);
                if((readn(fd_sock, (void *) path_r, sz_pr)) == -1){
                    free(path_r);
                    return -1;
                }
                if((readn(fd_sock, (void *) &sz_dr, sizeof(size_t))) == -1){
                    free(path_r);
                    return -1;
                }
                data_r = malloc(sz_dr);
                memset(data_r, '\0', sz_dr);
                if((readn(fd_sock, (void *) data_r, sz_pr)) == -1){
                    free(path_r);
                    free(data_r);
                    return -1;
                }

                char* p = getNameFile(path_r);
                char* f = (char *) malloc(STR_LEN * sizeof(char));
                memset(f, '\0', STR_LEN);
                strncpy(f, dirname, STR_LEN-1);
                strncat(f, "/", 2);
                strncat(f, p, STR_LEN);
                write_file(f, data_r, sz_dr);
                free(path_r);
                free(data_r);
                free(p);
                free(f);
            }
        }
    }else{
        char* reason = NULL;
        size_t sz_r  = 0;
        if((readn(fd_sock, &sz_r, sizeof(size_t))) == -1){
            return -1;
        }
        #ifdef PRINT_REASON
            if(reason != NULL){
                fprintf(stdout, "failure to write file '%s': %s\n", pathname, reason);
            }
        #endif
        if(reason) free(reason);
        // errno da settare qua
        return -1;
    }

    return 0;
}

int appendToFile( const char* pathname, void* buf, size_t size, const char* dirname ){
    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    operation = _ATF_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    /* sending the 'writeFile' request to the server */
    size_t sz_p = strlen(pathname)+1;
    if(sz_p <= 1){
        errno = EFAULT;
        return -1;
    }
    if((writen(fd_sock, (void *) &sz_p, sizeof(size_t))) == -1){
        return -1;
    }
    if((writen(fd_sock, (void *) pathname, sz_p)) == -1){
        return -1;
    }
    if((writen(fd_sock, (void *) &size, sizeof(size_t))) == -1){
        return -1;
    }
    if((writen(fd_sock, (void *) buf, size)) == -1){
        return -1;
    }

    /* receiving the response to the 'writeFile' request to the server */
    result = -1;

    if((readn(fd_sock, (void *) &result, sizeof(int))) == -1){
        return -1;
    }

    if(result == SUCCESS_O){
        int N = 0;

        if((readn(fd_sock, (void *) &N, sizeof(int))) == -1){
            return -1;
        }
        if(N > 0){
            #ifdef PRINT_INFORMATION
                fprintf(stderr, "Information: the write file operation caused the removal of '%d' files from the server.\n", N);
            #endif
            char* path_r   = NULL;
            char* data_r   = NULL;
            size_t sz_pr   = 0;
            size_t sz_dr   = 0;

            for(int i=0; i<N; i++){
                if((readn(fd_sock, (void *) &sz_pr, sizeof(size_t))) == -1){
                    return -1;
                }
                path_r = malloc(sz_pr);
                memset(path_r, '\0', sz_pr);
                if((readn(fd_sock, (void *) path_r, sz_pr)) == -1){
                    free(path_r);
                    return -1;
                }
                if((readn(fd_sock, (void *) &sz_dr, sizeof(size_t))) == -1){
                    free(path_r);
                    return -1;
                }
                data_r = malloc(sz_dr);
                memset(data_r, '\0', sz_dr);
                if((readn(fd_sock, (void *) data_r, sz_pr)) == -1){
                    free(path_r);
                    free(data_r);
                    return -1;
                }

                char* p = getNameFile(path_r);
                char* f = (char *) malloc(STR_LEN * sizeof(char));
                memset(f, '\0', STR_LEN);
                strncpy(f, dirname, STR_LEN-1);
                strncat(f, "/", 2);
                strncat(f, p, STR_LEN);
                write_file(f, data_r, sz_dr);
                free(path_r);
                free(data_r);
                free(p);
                free(f);
            }
        }
    }else{
        char* reason = NULL;
        size_t sz_r  = 0;
        if((readn(fd_sock, &sz_r, sizeof(size_t))) == -1){
            return -1;
        }
        #ifdef PRINT_REASON
            if(reason != NULL){
                fprintf(stdout, "failure to write file '%s': %s\n", pathname, reason);
            }
        #endif
            if(reason) free(reason);
        // errno da settare qua
        return -1;
    }

    return 0;
}

int lockFile( const char* pathname ){
    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    operation = _LF_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    /* sending the 'lockFile' request to the server */
    size_t sz_p = strlen(pathname)+1;
    if(write_pathname(fd_sock, pathname, sz_p) == -1){

    }

    /* receiving the response to the 'lockFile' request from the server */
    int result = 0;
    char* reason = NULL;
    if(readn(fd_sock, &result, sizeof(int)) == -1){

    }
    if(result == FAILED_O){
        if(read_reason(fd_sock, &reason) == -1){

        }
        #ifdef PRINT_REASON
            if(reason != NULL){
                fprintf(stdout, "failure to write file '%s': %s\n", pathname, reason);
            }
        #endif
        if(reason) free(reason);
        return -1;
    }

    return 0;
}

int unlockFile( const char* pathname ){
    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    operation = _UF_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    /* sending the 'unlockFile' request to the server */
    size_t sz_p = strlen(pathname)+1;
    if(write_pathname(fd_sock, pathname, sz_p) == -1){

    }

    /* receiving the response to the 'unlockFile' request from the server */
    int result = 0;
    char* reason = NULL;
    if(readn(fd_sock, &result, sizeof(int)) == -1){

    }
    if(result == FAILED_O){
        if(read_reason(fd_sock, &reason) == -1){

        }
        #ifdef PRINT_REASON
            if(reason != NULL){
                fprintf(stdout, "failure to write file '%s': %s\n", pathname, reason);
            }
        #endif
        if(reason) free(reason);
        return -1;
    }

    return 0;
}

int closeFile( const char* pathname ){
    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    operation = _CF_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    /* sending the 'closeFile' request to the server */
    size_t sz_p = strlen(pathname)+1;
    if(write_pathname(fd_sock, pathname, sz_p) == -1){

    }

    /* receiving the response to the 'closeFile' request to the server */
    int result = 0;
    char* reason = NULL;
    if(readn(fd_sock, &result, sizeof(int)) == -1){

    }
    if(result == FAILED_O){
        if(read_reason(fd_sock, &reason) == -1){

        }
        #ifdef PRINT_REASON
            if(reason != NULL){
                fprintf(stdout, "failure to write file '%s': %s\n", pathname, reason);
            }
        #endif
        if(reason) free(reason);
        return -1;
    }

    return 0;
}

int removeFile( const char* pathname ){
    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    operation = _RFI_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    /* sending the 'removeFile' request to the server */
    size_t sz_p = strlen(pathname)+1;
    if(write_pathname(fd_sock, pathname, sz_p) == -1){
        return -1;
    }

    /* receiving the response to the 'removeFile' request from the server */
    int result = 0;
    char* reason = NULL;
    if(readn(fd_sock, &result, sizeof(int)) == -1){
        return -1;
    }
    if(result == FAILED_O){
        if(read_reason(fd_sock, &reason) == -1){
            if(reason) free(reason);
            return -1;
        }
        #ifdef PRINT_REASON
            if(reason != NULL){
                fprintf(stdout, "failure to write file '%s': %s\n", pathname, reason);
            }
        #endif
        if(reason) free(reason);
        return -1;
    }

    return 0;
}
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
* @file interface.c
*
* definition of utility functions for client
*
* @author adrien koumgang tegantchouang
* @version 1.0
* @date 00/05/2021
*/


// TODO: riguardare tutte le funzioni : la parte di ritorno di ogni funzione

#define _POSIX_C_SOURCE  200112L  // needed for S_ISSOCK

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>
#include <dirent.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#include <pwd.h>
#include <grp.h>
#include <stddef.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <assert.h>

//#include "utils.h"
#include "interface.h"
#include "communication.h"
#include "read_write_file.h"
#include "utils.h"

extern char* rindex(const char*, int);

#define PRINT_OPERATION
#define PRINT_INFORMATION
#define PRINT_REASON

static int fd_sock;
static struct sockaddr_un serv_addr;
static int operation = -1;
static int result = -1;
long bytes_read;
long bytes_write;



/**
* An AF_UNIX connection is opened to the socket file sockname
*
* @param sockname : name of the socket to connect to
* @param msec : time to repeat to retry the connection with
*                       the server if it fails
* @param abstime : time until you try to connect to the server
*
* @returns : 0 if successful
*            -1 if the request fails and errno is set
*
* errno :
*   EINVAL => in case of invalid parameter
*   ETIME => in case of timer expired
*/
int openConnection( const char* sockname, int msec,
                                        const struct timespec abstime ){
    if(!sockname || (msec < 0)){
        errno = EINVAL;
        return -1;
    }

    if((fd_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
        return -1;

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, sockname, strlen(sockname)+1);

    struct timespec time_sleep;
    time_sleep.tv_sec = 0;
    time_sleep.tv_nsec = msec * 1000000;
    struct timespec time_request;
    time_request.tv_sec = 0;
    time_request.tv_nsec = msec * 1000000;

    struct timespec current_time;
    memset(&current_time, '0', sizeof(current_time));

    do{
        if(connect(fd_sock, (struct sockaddr*) &serv_addr,
                                        sizeof(serv_addr)) != -1)
            return 0;

        nanosleep(&time_sleep, &time_request);

        if(clock_gettime(CLOCK_REALTIME, &current_time) == -1){
            #ifdef PRINT_INFORMATION
                fprintf(stderr, "Information: open connection operation failed, reason: problem in calculating the current time (clock_gettime)\n");
            #endif
            return -1;
        }

    }while(current_time.tv_sec < abstime.tv_sec ||
        current_time.tv_nsec < abstime.tv_nsec);

    #ifdef PRINT_INFORMATION
        fprintf(stderr, "Information: open connection operation failed, reason: connection test time passed\n");
    #endif
    errno =  ETIME;
    return -1;
}

/**
* closes the established connection between the client and the server
*
* @params sockname : the name of the socket on which the client was connected
*
* @returns : 0 if successful
*            -1 if the request fails and errno is set
*
* errno :
*   EINVAL => in case of invalid parameter
*/
int closeConnection( const char* sockname ){
    if(!sockname){
        #ifdef PRINT_INFORMATION
            fprintf(stderr, "Information: close connection operation failed, reason: wrong socket name\n");
        #endif
        errno = EINVAL;
        return -1;
    }

    if(strncmp(serv_addr.sun_path, sockname, strlen(sockname)+1) ==  0){
        operation = _CC_O;
        // I write the operation to do
        if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
            return -1;
        }

        /* receiving the response to the 'closeConnection' request to the server */
        result = -1;

        if((readn(fd_sock, (void *) &result, sizeof(int))) == -1){
            return -1;
        }

        if( result == SUCCESS_O){
            #ifdef PRINT_INFORMATION
                fprintf(stderr, "Information:the connection was successfully closed!\n");
            #endif
            errno = EFAULT;
            return -1;
        }else{
            #ifdef PRINT_INFORMATION
                fprintf(stderr, "Information: close connection operation failed!\n");
            #endif
            errno = EFAULT;
            return -1;
        }

        return close(fd_sock);
    }else{
        #ifdef PRINT_INFORMATION
            fprintf(stderr, "Information: close connection operation failed, reason: wrong socket name\n");
        #endif
        errno = EFAULT;
        return -1;
    }
}

/**
* API function that allows the client to ask the server
*       to create or open a file
*
*
*
* @returns : 0 if successful
*            -1 if the request fails and errno is set
*
* errno :
*   EINVAL => in case of invalid parameter
*   EACCES => in case of an error response from the server
*/
int openFile( const char* pathname, int flags ){
    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    switch( flags ){
        case O_CREATE:{
            #ifdef PRINT_INFORMATION
                //fprintf(stdout, "Information: open file operation with flag 'O_CREATE'\n");
            #endif
            break;
        }
        case O_LOCK:{
            #ifdef PRINT_INFORMATION
                //fprintf(stdout, "Information: open file operation with flag 'O_LOCK'\n");
            #endif
            break;
        }
        case O_CREATE_LOCK:{
            #ifdef PRINT_INFORMATION
                //fprintf(stdout, "Information: open file operation with flag 'O_CREATE_LOCK'\n");
            #endif
            break;
        }
        default:{
            #ifdef PRINT_INFORMATION
                //fprintf(stderr, "Information: open file operation failed, reason: invalid opening flag (%d)\n", flags);
            #endif
            errno = EOPNOTSUPP;
            return -1;
        }
    }

    size_t sz_p = strlen(pathname)+1;
    if(sz_p <= 1){
        errno = EINVAL;
        return -1;
    }

    /******* sending the 'openFile' request to the server ******/

    operation = _OF_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    // I specify the type of operation to be done
    if((writen(fd_sock, (void *) &flags, sizeof(int))) == -1){
        return -1;
    }

    if(sz_p <= 0){
        errno = EFAULT;
        return -1;
    }

    if((writen(fd_sock, (void *) &sz_p, sizeof(size_t))) == -1){
        return -1;
    }

    if((writen(fd_sock, (void *) pathname, sz_p)) == -1){
        return -1;
    }

    /*** receiving the response to the 'open File' request to the server ***/

    result = -1;
    if((readn(fd_sock, (void *) &result, sizeof(int))) == -1){
        return -1;
    }

    if(result == FAILED_O){
        char* reason = NULL;
        size_t sz_r = 0;
        if((readn(fd_sock, (void *) &sz_r, sizeof(size_t))) == -1){
            return -1;
        }

        reason = (char *) malloc(sz_r);
        memset(reason, '\0', sz_r);
        if((readn(fd_sock, (void *) reason, sz_r)) == -1){
            return -1;
        }

        #ifdef PRINT_REASON
            fprintf(stderr, "Information: open file operation failed, reason: %s\n", reason);
        #endif

        free(reason);
        errno = EACCES;
        return -1;
    }

    return 0;

}

int readFile( const char* pathname, void** buf, size_t* size ){
    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    operation = _RF_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    /************* sending the 'readFile' request to the server ************/
    size_t sz_p = strlen(pathname)+1;
    if(sz_p <= 1){
        buf = NULL;
        *size = 0;
        errno = EFAULT;
        return -1;
    }

    if((writen(fd_sock, (void *) &sz_p, sizeof(size_t))) == -1){
        *buf = NULL;
        *size = 0;
        return -1;
    }

    if((writen(fd_sock, (void *) pathname, sz_p)) == -1){
        *buf = NULL;
        *size = 0;
        return -1;
    }

    /***** receiving the response to the 'readFile' request to the server *****/
    result = -1;

    if((readn(fd_sock, &result, sizeof(int))) == -1){
        *buf = NULL;
        *size = 0;
        return -1;
    }
    if(result == SUCCESS_O){
        if((readn(fd_sock, (void *) size, sizeof(size_t))) == -1){
            *buf = NULL;
            *size = 0;
            return -1;
        }

        *buf = malloc(*size);
        memset(*buf, '\0', *size);
        if((readn(fd_sock, (void *) *buf, *size)) == -1){
            *buf = NULL;
            *size = 0;
            return -1;
        }

    }else{
        *buf = NULL;
        *size = 0;
        char* reason = NULL;
        size_t sz_r = 0;
        if((readn(fd_sock, (void *) &sz_r, sizeof(size_t))) == -1){
            return -1;
        }

        reason = (char *) malloc(sz_r);
        memset(reason, '\0', sz_r);
        if((readn(fd_sock, (void *) reason, sz_r)) == -1){
            return -1;
        }

        #ifdef PRINT_REASON
            if(reason != NULL){
                fprintf(stdout, "failure to read file '%s': %s\n", pathname, reason);
            }
        #endif
        if(reason) free(reason);
        return -1;
    }

    return 0;
}

int readNFile( int N, const char* dirname ){
    /* sending the 'readNFile' request to the server */
    /* receiving the response to the 'readNFile' request to the server */
    if(!dirname){
        errno = EINVAL;
        return -1;
    }

    operation = _RNF_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    if((writen(fd_sock, (void *) &N, sizeof(int))) == -1){
        return -1;
    }

    /***** receiving the response to the 'readFile' request to the server *****/
    int n = 0;

    if((readn(fd_sock, &n, sizeof(int))) == -1){
        return -1;
    }

    if(n <= 0){
        char* reason = NULL;
        size_t sz_r = 0;
        if((readn(fd_sock, (void *) &sz_r, sizeof(size_t))) == -1){
            return -1;
        }

        reason = (char *) malloc(sz_r);
        memset(reason, '\0', sz_r);
        if((readn(fd_sock, (void *) reason, sz_r)) == -1){
            return -1;
        }
        #ifdef PRINT_REASON
            if(reason != NULL){
                fprintf(stdout, "failure to read file: %s\n", reason);
            }
        #endif
    }else{
            char* path_r   = NULL;
            void* data_r   = NULL;
            size_t sz_pr   = 0;
            size_t sz_dr   = 0;
            int fin = 0;
            int i = 0;
            while(i < n){
                if((readn(fd_sock, (void *) &fin, sizeof(int))) == -1){
                    return -1;
                }
                if(fin) break;

                if((readn(fd_sock, (void *) &sz_pr, sizeof(size_t))) == -1){
                    return -1;
                }

                path_r = (char *) malloc(sz_pr);
                memset(path_r, '\0', sz_pr);
                if((readn(fd_sock, (void *) path_r, sz_pr)) == -1){
                    if(path_r) free(path_r);
                    return -1;
                }

                if((readn(fd_sock, (void *) &sz_dr, sizeof(size_t))) == -1){
                    return -1;
                }

                data_r = (void *) malloc(sz_dr);
                memset(data_r, '0', sz_dr);
                if((readn(fd_sock, (void *) data_r, sz_dr)) == -1){
                    if(path_r) free(path_r);
                    if(data_r) free(data_r);
                    return -1;
                }

                char* p = NULL;
                p = getNameFile(path_r);
                if(p == NULL){
                    continue;
                }
                char* final_p = NULL;
                final_p = setNameFile(dirname, p);
                if(write_file(final_p, data_r, sz_dr) == -1){

                }
                if(final_p) free(final_p);
                if(path_r) free(path_r);
                if(data_r) free(data_r);

                i++;
            }
    }

    return 0;
}

int writeFile( const char* pathname, const char* dirname ){
    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    size_t sz_p = strlen(pathname)+1;
    if(sz_p <= 1){
        errno = EFAULT;
        return -1;
    }
    char *data = NULL;
    size_t sz_d = 0;
    if(read_file(pathname, (void **) &data, &sz_d, O_RDONLY) <= 0){
        if(data) free(data);
        return -1;
    }

    if(sz_d <= 0){
        return -1;
    }

    operation = _WF_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    /* sending the 'writeFile' request to the server */
    if((write_pathname(fd_sock, (void *) pathname, sz_p)) == -1){
        return -1;
    }
    /*if((writen(fd_sock, (void *) &sz_p, sizeof(size_t))) == -1){
        return -1;
    }
    if((writen(fd_sock, (void *) pathname, sz_p)) == -1){
        return -1;
    }*/

    if((write_data(fd_sock, (void *) data, sz_d)) == -1){
        return -1;
    }

    /*if((writen(fd_sock, (void *) &sz_d, sizeof(size_t))) == -1){
        return -1;
    }
    if((writen(fd_sock, (void *) data, sz_d)) == -1){
        return -1;
    }*/

    if(data) free(data);

    /* receiving the response to the 'writeFile' request to the server */
    result = -1;

    if((readn(fd_sock, (void *) &result, sizeof(int))) == -1){
        return -1;
    }

    if(result == SUCCESS_O){
        int N = 0;

        if((readn(fd_sock, (void *) &N, sizeof(int))) == -1){
            return -1;
        }
        if(N > 0){
            #ifdef PRINT_INFORMATION
                fprintf(stderr, "Information: the write file operation caused the removal of '%d' files from the server.\n", N);
            #endif
            char* path_r   = NULL;
            char* data_r   = NULL;
            size_t sz_pr   = 0;
            size_t sz_dr   = 0;

            for(int i=0; i<N; i++){
                if((readn(fd_sock, (void *) &sz_pr, sizeof(size_t))) == -1){
                    return -1;
                }
                path_r = malloc(sz_pr);
                memset(path_r, '\0', sz_pr);
                if((readn(fd_sock, (void *) path_r, sz_pr)) == -1){
                    free(path_r);
                    return -1;
                }
                if((readn(fd_sock, (void *) &sz_dr, sizeof(size_t))) == -1){
                    free(path_r);
                    return -1;
                }
                data_r = malloc(sz_dr);
                memset(data_r, '\0', sz_dr);
                if((readn(fd_sock, (void *) data_r, sz_pr)) == -1){
                    free(path_r);
                    free(data_r);
                    return -1;
                }

                char* p = getNameFile(path_r);
                char* f = (char *) malloc(STR_LEN * sizeof(char));
                memset(f, '\0', STR_LEN);
                strncpy(f, dirname, STR_LEN-1);
                strncat(f, "/", 2);
                strncat(f, p, STR_LEN);
                write_file(f, data_r, sz_dr);
                free(path_r);
                free(data_r);
                free(p);
                free(f);
            }
        }
    }else{
        char* reason = NULL;
        size_t sz_r  = 0;
        if((readn(fd_sock, &sz_r, sizeof(size_t))) == -1){
            return -1;
        }
        #ifdef PRINT_REASON
            if(reason != NULL){
                fprintf(stdout, "failure to write file '%s': %s\n", pathname, reason);
            }
        #endif
        if(reason) free(reason);
        // errno da settare qua
        return -1;
    }

    return 0;
}

int appendToFile( const char* pathname, void* buf, size_t size, const char* dirname ){
    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    operation = _ATF_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    /* sending the 'writeFile' request to the server */
    size_t sz_p = strlen(pathname)+1;
    if(sz_p <= 1){
        errno = EFAULT;
        return -1;
    }
    if((writen(fd_sock, (void *) &sz_p, sizeof(size_t))) == -1){
        return -1;
    }
    if((writen(fd_sock, (void *) pathname, sz_p)) == -1){
        return -1;
    }
    if((writen(fd_sock, (void *) &size, sizeof(size_t))) == -1){
        return -1;
    }
    if((writen(fd_sock, (void *) buf, size)) == -1){
        return -1;
    }

    /* receiving the response to the 'writeFile' request to the server */
    result = -1;

    if((readn(fd_sock, (void *) &result, sizeof(int))) == -1){
        return -1;
    }

    if(result == SUCCESS_O){
        int N = 0;

        if((readn(fd_sock, (void *) &N, sizeof(int))) == -1){
            return -1;
        }
        if(N > 0){
            #ifdef PRINT_INFORMATION
                fprintf(stderr, "Information: the write file operation caused the removal of '%d' files from the server.\n", N);
            #endif
            char* path_r   = NULL;
            char* data_r   = NULL;
            size_t sz_pr   = 0;
            size_t sz_dr   = 0;

            for(int i=0; i<N; i++){
                if((readn(fd_sock, (void *) &sz_pr, sizeof(size_t))) == -1){
                    return -1;
                }
                path_r = malloc(sz_pr);
                memset(path_r, '\0', sz_pr);
                if((readn(fd_sock, (void *) path_r, sz_pr)) == -1){
                    free(path_r);
                    return -1;
                }
                if((readn(fd_sock, (void *) &sz_dr, sizeof(size_t))) == -1){
                    free(path_r);
                    return -1;
                }
                data_r = malloc(sz_dr);
                memset(data_r, '\0', sz_dr);
                if((readn(fd_sock, (void *) data_r, sz_pr)) == -1){
                    free(path_r);
                    free(data_r);
                    return -1;
                }

                char* p = getNameFile(path_r);
                char* f = (char *) malloc(STR_LEN * sizeof(char));
                memset(f, '\0', STR_LEN);
                strncpy(f, dirname, STR_LEN-1);
                strncat(f, "/", 2);
                strncat(f, p, STR_LEN);
                write_file(f, data_r, sz_dr);
                free(path_r);
                free(data_r);
                free(p);
                free(f);
            }
        }
    }else{
        char* reason = NULL;
        size_t sz_r  = 0;
        if((readn(fd_sock, &sz_r, sizeof(size_t))) == -1){
            return -1;
        }
        #ifdef PRINT_REASON
            if(reason != NULL){
                fprintf(stdout, "failure to write file '%s': %s\n", pathname, reason);
            }
        #endif
            if(reason) free(reason);
        // errno da settare qua
        return -1;
    }

    return 0;
}

int lockFile( const char* pathname ){
    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    operation = _LF_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    /* sending the 'lockFile' request to the server */
    size_t sz_p = strlen(pathname)+1;
    if(write_pathname(fd_sock, pathname, sz_p) == -1){

    }

    /* receiving the response to the 'lockFile' request from the server */
    int result = 0;
    char* reason = NULL;
    if(readn(fd_sock, &result, sizeof(int)) == -1){

    }
    if(result == FAILED_O){
        if(read_reason(fd_sock, &reason) == -1){

        }
        #ifdef PRINT_REASON
            if(reason != NULL){
                fprintf(stdout, "failure to write file '%s': %s\n", pathname, reason);
            }
        #endif
        if(reason) free(reason);
        return -1;
    }

    return 0;
}

int unlockFile( const char* pathname ){
    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    operation = _UF_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    /* sending the 'unlockFile' request to the server */
    size_t sz_p = strlen(pathname)+1;
    if(write_pathname(fd_sock, pathname, sz_p) == -1){

    }

    /* receiving the response to the 'unlockFile' request from the server */
    int result = 0;
    char* reason = NULL;
    if(readn(fd_sock, &result, sizeof(int)) == -1){

    }
    if(result == FAILED_O){
        if(read_reason(fd_sock, &reason) == -1){

        }
        #ifdef PRINT_REASON
            if(reason != NULL){
                fprintf(stdout, "failure to write file '%s': %s\n", pathname, reason);
            }
        #endif
        if(reason) free(reason);
        return -1;
    }

    return 0;
}

int closeFile( const char* pathname ){
    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    operation = _CF_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    /* sending the 'closeFile' request to the server */
    size_t sz_p = strlen(pathname)+1;
    if(write_pathname(fd_sock, pathname, sz_p) == -1){

    }

    /* receiving the response to the 'closeFile' request to the server */
    int result = 0;
    char* reason = NULL;
    if(readn(fd_sock, &result, sizeof(int)) == -1){

    }
    if(result == FAILED_O){
        if(read_reason(fd_sock, &reason) == -1){

        }
        #ifdef PRINT_REASON
            if(reason != NULL){
                fprintf(stdout, "failure to write file '%s': %s\n", pathname, reason);
            }
        #endif
        if(reason) free(reason);
        return -1;
    }

    return 0;
}

int removeFile( const char* pathname ){
    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    operation = _RFI_O;

    // I write the operation to do
    if((writen(fd_sock, (void *) &operation, sizeof(int))) == -1){
        return -1;
    }

    /* sending the 'removeFile' request to the server */
    size_t sz_p = strlen(pathname)+1;
    if(write_pathname(fd_sock, pathname, sz_p) == -1){
        return -1;
    }

    /* receiving the response to the 'removeFile' request from the server */
    int result = 0;
    char* reason = NULL;
    if(readn(fd_sock, &result, sizeof(int)) == -1){
        return -1;
    }
    if(result == FAILED_O){
        if(read_reason(fd_sock, &reason) == -1){
            if(reason) free(reason);
            return -1;
        }
        #ifdef PRINT_REASON
            if(reason != NULL){
                fprintf(stdout, "failure to write file '%s': %s\n", pathname, reason);
            }
        #endif
        if(reason) free(reason);
        return -1;
    }

    return 0;
}
