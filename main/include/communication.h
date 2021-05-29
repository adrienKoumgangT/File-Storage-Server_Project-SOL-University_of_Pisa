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
* @file conn.h
*
* definition of utility functions for read and write by socket
*
* @author adrien koumgang tegantchouang
* @version 1.0
* @date 00/05/2021
*/


#ifndef COMUNICATION_H
#define COMUNICATION_H

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



#include "my_file.h"


// flags that specify the operation requested by the client
#define OF      "OF";
#define RF      "RF";
#define RNF     "RNF";
#define WF      "WF";
#define ATF     "ATF";
#define LF      "LF";
#define UF      "UF";
#define CF      "CF";
#define RF      "RF";

// operation
#define _OF_O       (1)
#define _RF_O       (2)
#define _RNF_O      (3)
#define _WF_O       (4)
#define _ATF_O      (5)
#define _LF_O       (6)
#define _UF_O       (7)
#define _CF_O       (8)
#define _RFI_O      (9)

#define O_CREATE            (1)
#define O_LOCK              (2)
#define O_CREATE_AND_LOCK   (3)

// success or failed operation
#define SUCCESS_O (0)
#define FAILED_O (-1)



/***  utility functions for communication functions between client servers  ***/

/****  write function  *****/

int write_function( const int fd, const char* pathname, const char* data,
                                            const size_t size ){

    int err;

    if(!pathname){
        errno = EINVAL;
        return -1;
    }

    size_t sz_p = sizeof(pathname);
    // I write the pathname size of the file : sizeof(pathname)
    if((err = write(fd, &sz_p, sizeof(size_t))) == -1){
        if(errno == EINTR)
            return -1;
    }

    // I write the pathname of the file : pathname
    if((err = write(fd, pathname, sz_p)) == -1){
        if(errno != EINTR)
            return -1;
    }

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

/**** read function *****/

int read_function( const int fd, char* pathname, char* data, size_t* size ){
    int err;

    size_t sz_p;
    // I read the pathname size of the file
    if((read(fd, &sz_p, sizeof(size_t))) == -1){
        return -1;
    }

    // I read the pathname of the file
    if(pathname) free(pathname);
    pathname = (char *) malloc(sz_p * sizeof(char));
    if((read(fd, pathname, sz_p)) == -1){
        return -1;
    }

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
                                    !(flags == O_CREATE_AND_LOCK)){
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
    // I write the pathname size of the file
    if((write(fd, &sz_p, sizeof(sz_p))) == -1){
        return -1;
    }

    // I write the pathname of the file
    if((write(fd, pathname, sizeof(pathname))) == -1){
        return -1;
    }

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
    // I write the pathname size
    if((write(fd, &sz_p, sizeof(sz_p))) == -1){
        if(errno != EINTR)
            return -1;
    }

    // I write the pathname of the file
    if((write(fd, pathname, sizeof(pathname))) == -1){
        if(errno != EINTR)
            return -1;
    }

    return 0;
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
int write_request_RNF( const int fd, const int N ){
    if(fd <= 0){
        errno = EFAULT;
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
* @params pathname : key of the file to be written to the server
* @params size : size of the file to write to the server
* @params t : creation date of the file to be written to the server
* @params data : contents of the file to be written to the server
*
* @returns : -1 in case of error with errno set
*             0 if successful
*/
int write_request_WF( const int fd, const char* pathname, const size_t size,
                            const char* data ){

    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((write_function(fd, pathname, data, size)) == -1)
        return -1;

    return 0;

}

int write_request_ATF( const int fd, const char* pathname, const char* data,
                                        size_t size, const char* dirname ){
    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    //int err;



    return 0;
}

int write_request_LF();

int write_request_UF();

int write_request_CF();

int write_request_RFI();


/****************************** read response ************************/

/***/
int read_response_OF( const int fd, int* response ){
    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((read(fd, &response, sizeof(response))) == -1){
        return -1;
    }

    return 0;
}

/***/
int read_response_RF( const int fd, char** buf, size_t* size ){
    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((read(fd, size, sizeof(size_t))) == -1){
        return -1;
    }

    if((read(fd, *buf, *size)) == -1){
        return -1;
    }

    return 0;
}

/***/
int read_response_RNF( const int fd, int* N, char** pathname,
                                                char** data, size_t* size ){

    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((read(fd, N, sizeof(int))) == -1){
        return -1;
    }

    if(pathname) free(pathname);
    pathname = (char **) malloc(*N * sizeof(char*));
    if(size) free(size);
    size = (size_t *) malloc(*N * sizeof(size_t));

    for(int i=0; i < *N; i++){
        if((read_function(fd, pathname[i], data[i], &size[i])) == -1)
            return -1;
    }

    return 0;
}

int read_response_WF( const int fd, int* result, char* pathname,
                                                char* data, size_t* size ){

    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((read(fd, result, sizeof(int))) == -1){
        return -1;
    }

    if(*result == SUCCESS_O){
        int v;
        if((read(fd, &v, sizeof(int))) == -1){
            return -1;
        }

        if(v == 1){
            //if((read_function(fd, pathname, data, size)))
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
int read_request_OF( const int fd, char* pathname ){

    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    int err;
    int flags;

    // I read the flag associated with the operation to be done
    if((err = read(fd, &flags, sizeof(int))) == -1){
        return -1;
    }

    size_t sz_p;
    // I read the size of the pathname
    if((err = read(fd, &sz_p, sizeof(size_t))) == -1){
        return -1;
    }

    // I read the pathname of file
    if((err = read(fd, pathname, sz_p)) == -1){
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
int read_request_RF( const int fd, char* pathname ){

    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if(pathname) free(pathname);

    size_t sz_p = 0;
    // I read the pathname size of the file
    if((read(fd, &sz_p, sizeof(sz_p))) == -1){
        if(errno != EINTR)
        return -1;
    }

    // I read the key of the file
    pathname = (char *) malloc(sz_p);
    if((read(fd, pathname, sz_p)) == -1){
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
int read_request_WF( const int fd, file_t* file ){

    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    //int err;

    if(file) free(file);
    //size_t sz_k = 0;
    file = (file_t *) malloc(sizeof(file_t));
    memset(file, '0', sizeof(file_t));

    if((read_function(fd, file->key, file->data, &(file->size))) == -1)
        return -1;





    return 0;

}


/********************************* write response ***************************/

/***/
int write_response_OF( const int fd, int response ){
    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((write(fd, &response, sizeof(int))) == -1){
        return -1;
    }

    return 0;
}

/***/
int write_response_RF(const int fd, char* data, size_t size){

    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((write(fd, &size, sizeof(size_t))) == -1){
        return -1;
    }

    size_t left = size;
    int err;
    char* bufptr = data;
    while(left>0){
        if((err = write(fd, bufptr, left)) == -1){
            if(errno == EINTR) continue;
            return -1;
        }
        if(err == 0) break;
        left -= err;
    }

    return 0;
}

/***/
int write_response_RNF( const int fd, const int N, char** pathname,
                                        char** data, size_t* size ){

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
        if((write_function(fd, pathname[i], data[i], size[i])) == -1)
            return -1;
    }

    return 0;
}

/**
*
*/
int write_response_WF( const int fd, const int result, const char* pathname,
                                        const char* data, const size_t size ){


    if(fd <= 0){
        errno = EFAULT;
        return -1;
    }

    if((write(fd, &result, sizeof(int))) == -1){
        return -1;
    }

    if(result == SUCCESS_O){
        if(pathname){
            int v = 1; // if a file has been thrown away
            if((write(fd, &v, sizeof(int))) == -1){
                return -1;
            }
        }else{
            int v = 0;
            if((write(fd, &v, sizeof(int))) == -1){
                return -1;
            }

            if((write_function(fd, pathname, data, size)) == -1)
                return -1;

        }
    }

    return 0;
}

#endif
