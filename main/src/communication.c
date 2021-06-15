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
* @file communication.c
*
* definition of utility functions for read and write by socket
*
* @author adrien koumgang tegantchouang
* @version 1.0
* @date 00/05/2021
*/


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <time.h>


#include "communication.h"
#include "my_file.h"


/***  utility functions for communication functions between client servers  ***/

/****** */

static int write_data( const int fd, const char* data, const size_t size ){
    int err;

    // I write the size of file content : size
    if((err = write(fd, &size, sizeof(size_t))) == -1){
        if(errno != EINTR)
            return -1;
    }

    // I write the contents of the file : data
    if( size > 0 ){
        size_t left = size;
        char* bufptr = (char *) malloc(size);
        memset(bufptr, '\0', size);
        strncpy(bufptr, data, size);
        while(left>0){
            if((err = write(fd, bufptr, left)) == -1){
                if(errno == EINTR) continue;
                return -1;
            }
            if(err == 0) break;
            left -= err;
        }
    }

    return 0;
}

static int read_data( const int fd, char* data, size_t* size ){
    int err;

    // I read the size of file content
    if((read(fd, size, sizeof(size_t))) == -1){
        return -1;
    }

    // I read the contents of the file
    if(*size > 0){
        size_t left = *size;
        if(data) free(data);
        data = (char *) malloc(*size * sizeof(char));
        char* bufptr = data;
        while(left>0){
            if((err = read(fd, bufptr, left)) == -1){
                if(errno == EINTR) continue;
                return -1;
            }
            if(err == 0) break;
            left -= err;
        }
    }else{
        data = NULL;
    }

    return 0;
}

/****  write function  *****/

static int write_function( const int fd, const char* pathname, const size_t size_p,
                                            const char* data, const size_t size_d ){

    if((write_data(fd, pathname, size_p)) == -1)
        return -1;

    if((write_data(fd, data, size_d)) == -1)
        return -1;

    return 0;
}

/**** read function *****/

static int read_function( const int fd, char* pathname, size_t* size_p,
                                            char* data, size_t* size_d ){

    if((read_data(fd, pathname, size_p)) == -1)
        return -1;

    if((read_data(fd, data, size_d)) == -1)
        return -1;

    return 0;
}

/******************************* client  *************************/


/*************************** write request **********************/


/**
* makes an open file request to the server
*
* @params fd : server descriptor file
* @params pathname : pathname of the file to read
* @params flags : flag to specify the operation to be performed
*                       on the file to be opened
*
* @returns :
*/
int write_request_OF( const int fd, const char* pathname, int flags ){

    if(fd < 0){
        errno = EFAULT;
        return -1;
    }

    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    if( !(flags == O_CREATE) && !(flags == O_LOCK) &&
                                    !(flags == O_CREATE_LOCK)){
        errno = EOPNOTSUPP;
        return -1;
    }

    int operation = _OF_O;

    // I write the operation to do
    if((write(fd, &operation, sizeof(int))) == -1){
        return -1;
    }

    // I specify the type of operation to be done
    if((write(fd, &flags, sizeof(int))) == -1){
        return -1;
    }

    size_t sz_p = sizeof(pathname);
    if(sz_p <= 0){
        errno = EFAULT;
        return -1;
    }

    if((write_data(fd, pathname, sz_p)) == -1)
        return -1;

    return sz_p;
}

/**
*
*
* @params fd : file descriptor of the server
*                   where to read the request
* @params pathname : pathname of the file to read on the server
*
* @returns : -1 in case of error with errno set
*             0 if successful
*/
int write_request_RF( const int fd, const char* pathname ){

    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    size_t sz_p = sizeof(pathname);
    if(sz_p <= 0){
        errno = EFAULT;
        return -1;
    }

    if((write_data(fd, pathname, sz_p)) == -1)
        return -1;

    return 0;
}

/**
*
*
* @params fd : file descriptor of the server
*                   where to read the request
* @params N : Number of files to read on the server
*
* @returns : -1 in case of error with errno set
*             0 if successful
*/
int write_request_RNF( const int fd, const int N ){
    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    int operation = _RNF_O;

    // I write the operation to do
    if((write(fd, &operation, sizeof(int))) == -1){
        return -1;
    }

    if((write(fd, &N, sizeof(int))) == -1){
        return -1;
    }

    return 0;
}

/**
* function that allows me to tell the server that
* the client wants to write a file in the database
*
* @params fd : file descriptor of the server
*                   where to read the request
* @params operation : WriteFile or appendToFile
* @params pathname : key of the file to be written to the server
* @params size : size of the file to write to the server
* @params t : creation date of the file to be written to the server
* @params data : contents of the file to be written to the server
*
* @returns : -1 in case of error with errno set
*             0 if successful
*/
int write_request_WF_ATF( const int fd, const int operation, const char* pathname,
                                        const char* data, size_t size ){
    if(fd <= 0 || !pathname || !data){
        errno = EFAULT;
        return -1;
    }

    // I write the operation to do
    if((write(fd, &operation, sizeof(int))) == -1){
        return -1;
    }

    size_t sz_p = sizeof(pathname);
    if(sz_p <= 0){
        errno = EFAULT;
        return -1;
    }
    if((write_function(fd, pathname, sz_p, data, size)) == -1){
        return -1;
    }

    return 0;
}

int write_request_LF_UF_CF_RFI( const int fd, const int operation, const char* pathname ){
    if(fd <= 0 || !pathname){
        errno = EFAULT;
        return -1;
    }

    // I write the operation to do
    if((write(fd, &operation, sizeof(int))) == -1){
        return -1;
    }

    size_t sz_p = sizeof(pathname);
    if(sz_p <= 0){
        errno = EFAULT;
        return -1;
    }
    if((write_data(fd, pathname, sz_p)) == -1)
        return -1;

    return 0;
}


/****************************** read response ************************/

/***/
int read_response_OF( const int fd, int* response, char* reason ){
    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((read(fd, response, sizeof(response))) == -1){
        return -1;
    }

    if(*response == FAILED_O){
        size_t* sz_r = NULL;
        if(read_data(fd, reason, sz_r) == -1){
            return -1;
        }
    }

    return 0;
}

/***/
int read_response_RF( const int fd, int* resp, char** buf, size_t* size, char* reason ){
    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((read(fd, resp, sizeof(int))) == -1){
        return -1;
    }
    if(*resp == SUCCESS_O){
        reason = NULL;
        if((read_data(fd, *buf, size)) == -1){
            return -1;
        }
    }else{
        size_t sz_r = 0;
        if((read_data(fd, reason, &sz_r)) == -1){
            return -1;
        }
    }

    return 0;
}

/***/
int read_response_RNF( const int fd, int* N, char** pathname, size_t* size_p,
                                                char** data, size_t* size_d ){

    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((read(fd, N, sizeof(int))) == -1){
        return -1;
    }

    if(pathname) free(pathname);
    pathname = (char **) malloc(*N * sizeof(char*));
    if(size_p) free(size_p);
    if(size_d) free(size_d);
    size_p = (size_t *) malloc(*N * sizeof(size_t));
    size_d = (size_t *) malloc(*N * sizeof(size_t));

    for(int i=0; i < *N; i++){
        if((read_function(fd, pathname[i], &size_p[i], data[i], &size_d[i])) == -1)
            return -1;
    }

    return 0;
}

int read_response_WF_ATF( const int fd, int* result, char* reason, int* N,
                                char** pathname, size_t* size_pathname, char** data, size_t* size_data ){
    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((read(fd, result, sizeof(int))) == -1){
        return -1;
    }

    if(*result == SUCCESS_O){
        if((read(fd, N, sizeof(int))) == -1){
            return -1;
        }

        if(*N > 0){
            if(pathname)        free(pathname);
            pathname            = (char **) malloc(*N * sizeof(char*));
            if(size_pathname)   free(size_pathname);
            size_pathname       = (size_t *) malloc(*N * sizeof(size_t));
            if(data)            free(data);
            data                = (char **) malloc(*N * sizeof(char*));
            if(size_data)       free(size_data);
            size_data           = (size_t *) malloc(*N * sizeof(size_t));
            if(!pathname || !size_pathname || !data || !size_data) return -1;
            for(int i=0; i<*N; i++){
                if((read_function(fd, pathname[i], &size_pathname[i], data[i], &size_data[i])) == -1)
                    return -1;
            }
        }else{
            pathname        = NULL;
            size_pathname   = NULL;
            data            = NULL;
            size_data       = NULL;
        }
    }else{
        size_t* sz_r = NULL;
        if((read_data(fd, reason, sz_r)) == -1){
            return -1;
        }
    }

    return 0;
}

int read_response_LF_UF_CF_RFI( const int fd, int* result, char* reason ){
    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((read(fd, result, sizeof(int))) == -1){
        return -1;
    }

    if(*result != SUCCESS_O){
        size_t* sz_r = NULL;
        if((read_data(fd, reason, sz_r)) == -1){
            return -1;
        }
    }

    return 0;
}


/*********************************** server ***************************/


/*************************** read request **********************/

/**
* receives an open file request from the client
*
* @params fd : client descriptor file
* @params pathname : the buffer where to store the access pathname
*                           to the file to be read
*
* @returns :
*/
int read_request_OF( const int fd, char* pathname, int* flags ){

    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    int err;

    // I read the flag associated with the operation to be done
    if((err = read(fd, flags, sizeof(int))) == -1){
        return -1;
    }

    size_t sz_p;

    // I read the pathname of file
    if((err = read_data(fd, pathname, &sz_p)) == -1){
        return -1;
    }

    return 0;

}


/**
*
*
* @params fd : file descriptor of the client from whom
*                   to receive the request
* @params pathname : pathname of the file to read on the server
*
* @returns : -1 in case of error with errno set
*             0 if successful
*/
int read_request_RF( const int fd, char* pathname, size_t* sz_p ){

    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if(pathname) free(pathname);

    // I read the pathname size of the file
    if(read(fd, sz_p, sizeof(size_t)) == -1){
        if(errno != EINTR)
        return -1;
    }

    // I read the key of the file
    pathname = (char *) malloc(*sz_p);
    if((read(fd, pathname, *sz_p)) == -1){
        if(errno != EINTR)
            return -1;
    }

    return 0;
}

/**
*
*/
int read_request_RNF( const int fd, int* N ){
    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((read(fd, N, sizeof(int))) == -1){
        return -1;
    }

    return 0;
}

/**
* function that allows a client to read the data requested
* to write a file in the database
*
* @params fd : file descriptor of the client from whom
*                   to receive the request
* @params file : buffer file where the data of the file
*               to be written to the database will be written
*
* @returns : -1 in case of error with errno set
*               size of data if successful
*/
int read_request_WF_ATF( const int fd, char* pathname, size_t* sz_p, char* data, size_t* sz_d ){

    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((read_function(fd, pathname, sz_p, data, sz_d)) == -1)
        return -1;

    return 0;
}

/**
*/
int read_request_LF_UF_CF_RFI( const int fd, char* pathname ){
    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    size_t* sz_p = NULL;
    if((read_data(fd, pathname, sz_p)) == -1)
        return -1;

    return 0;
}


/********************************* write response ***************************/

/***/
int write_response_OF( const int fd, int response, char* reason ){
    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((write(fd, &response, sizeof(int))) == -1){
        return -1;
    }

    if(response == FAILED_O){
        if(write_data(fd, reason, sizeof(reason)) == -1){
            return -1;
        }
    }
    return 0;
}

/***/
int write_response_RF(const int fd, int resp, char* data, size_t size, char* reason ){

    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((write(fd, &resp, sizeof(int))) == -1){
        return -1;
    }

    if(resp == SUCCESS_O){
        if((write_data(fd, data, size)) == -1){
            return -1;
        }
    }else{
        if((write_data(fd, reason, sizeof(reason))) == -1){
            return -1;
        }
    }

    return 0;
}

/***/
int write_response_RNF( const int fd, const int N, char** pathname, size_t* size_p,
                                        char** data, size_t* size_d ){

    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if(!pathname || !data){
        return -1;
    }

    if((write(fd, &N, sizeof(int))) == -1){
        return -1;
    }

    for(int i=0; i<N; i++){
        if((write_function(fd, pathname[i], size_p[i], data[i], size_d[i])) == -1)
            return -1;
    }

    return 0;
}

/**
*
*/
int write_response_WF_ATF( const int fd, const int result, const char* reason, int N,
                char** pathname, size_t* size_p, char** data, size_t* size_d ){

    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((write(fd, &result, sizeof(int))) == -1){
        return -1;
    }

    if(result == SUCCESS_O){
        if(N <= 0){
            // if a file has been thrown away
            if((write(fd, &N, sizeof(int))) == -1){
                return -1;
            }
        }else{
            if((write(fd, &N, sizeof(int))) == -1){
                return -1;
            }

            for(int i=0; i<N; i++){
                if((write_function(fd, pathname[i], size_p[i], data[i], size_d[i])) == -1)
                    return -1;
            }
        }
    }else{
        if((write_data(fd, reason, sizeof(reason))) == -1){
            return -1;
        }
    }

    return 0;
}

/***/
int write_response_LF_UF_CF_RFI( const int fd, const int result, const char* reason ){
    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((write(fd, &result, sizeof(int))) == -1){
        return -1;
    }

    if(result != SUCCESS_O){
        if((write_data(fd, reason, sizeof(reason))) == -1){
            return -1;
        }
    }

    return 0;
}
