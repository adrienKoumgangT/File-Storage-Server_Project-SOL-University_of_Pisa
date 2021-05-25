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
* @file interface.h
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
#include "comunication.h"
#include "read_write_file.h"


static int fd_sock;
static struct sockaddr_un serv_addr;
int bytes_read;
int bytes_write;



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
*/
// TODO: da verificare fino in fondo
// (da verificare e fare concodare con il progetto)
int openConnection( const char* sockname, int msec,
                                        const struct timespec abstime ){
//    int err;

    if(!sockname || (msec < 0)){
        errno = EINVAL;
        return -1;
    }

    // creazione di un socket di dominio AF_UNIX,
    // tipo di connessione SOCK_STREAM e
    // di protocollo di default (0)
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
            close(fd_sock);
            return -1;
        }

    }while(current_time.tv_sec < abstime.tv_sec ||
        current_time.tv_nsec < abstime.tv_nsec);

    errno =  ETIMEDOUT;
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
        errno = EINVAL;
        return -1;
    }

    if(strncmp(serv_addr.sun_path, sockname, strlen(sockname)+1) ==  0){
        return close(fd_sock);
    }else{
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

    if( !(flags == O_CREATE) && !(flags == O_LOCK) &&
                                    !(flags == O_CREATE_AND_LOCK)){
        errno = EOPNOTSUPP;
        return -1;
    }

    int len_pathname = strlen(pathname);
    if(len_pathname <= 0){
        errno = EINVAL;
        return -1;
    }

    if( write_request_OF(fd_sock, pathname, flags) == -1 ){
        return -1;
    }

    int *resp = NULL;
    if( read_response_OF(fd_sock, resp) == -1 ){
        return -1;
    }

    if( *resp == FAILED_O){
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

    if( write_request_RF(fd_sock ,pathname) == -1 ){
        buf = NULL;
        *size = 0;
        return -1;
    }

    if( read_response_RF(fd_sock, (char**) buf, size ) == -1){
        buf = NULL;
        *size = 0;
        return -1;
    }

    return 0;
}

int readNFile( int N, const char* dirname );

int writeFile( const char* pathname, const char* dirname ){
    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    char *str = NULL;
    if(read_file(pathname, str, 0) <= 0){

    }


    /*
    if(write_WF(fd_sock) == -1){

    }*/

    return 0;
}

int appendToFile( const char* pathname, void* buf, size_t size,
                                                const char* dirname );

int lockFile( const char* pathname );

int unlockFile( const char* pathname );

int closeFile( const char* pathname );

int removeFile( const char* pathname );
