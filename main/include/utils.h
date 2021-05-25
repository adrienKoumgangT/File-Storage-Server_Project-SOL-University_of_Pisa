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
* @file utils.h
*
* definition of utility functions and macro
*
* @author adrien koumgang tegantchouang
* @version 1.0
* @date 00/05/2021
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#if !defined(EXTRA_LEN_PRINT_ERROR)
#define EXTRA_LEN_PRINT_ERROR 512
#endif


#ifndef FUNCTION_UTILS
#define FUNCTION_UTILS

long getNumber( char* s, int base ){
    long val;
    char *endptr, *str;

    str = s;
    errno = 0; /* To distinguish success/failure after call */
    val = strtol(str, &endptr, base);

    // check for various erros
    if((errno = ERANGE && (val == LONG_MAX || val == LONG_MIN))
            || (errno != 0 && val == 0)){
                perror("strtol");
                exit(EXIT_FAILURE);
    }

    // If we got here, strtol() successfully parsed a number
    return val;
}

#endif

#ifndef UTILS_H
#define UTILS_H

#endif

#ifndef SYSCALL_ERROR
#define SYSCALL_ERROR

#define SYSCALL_EXIT_EQ( name, r, sc, ve, str ) \
    if((r = (sc)) == ve){                       \
        perror(#name);                          \
        int errno_copy = errno;                 \
        print_error(str);          \
        exit(errno_copy);                       \
    }

#define SYSCALL_EXIT_NEQ( name, r, sc, ve, str ) \
    if((r = (sc)) != ve){                       \
        perror(#name);                          \
        int errno_copy = errno;                 \
        print_error(str);          \
        exit(errno_copy);                       \
    }


#define SYSCALL_RETURN_EQ( name, r, sc, ve, str ) \
    if((r = (sc)) == ve){                       \
        perror(#name);                          \
        int errno_copy = errno;                 \
        print_error(str);          \
        errno = errno_copy;                     \
        return r;                               \
    }


#define SYSCALL_RETURN_NEQ( name, r, sc, ve, str ) \
    if((r = (sc)) != ve){                       \
        perror(#name);                          \
        int errno_copy = errno;                 \
        print_error(str);          \
        errno = errno_copy;                     \
        return r;                               \
    }


#define SYSCALL_PRINT_EQ( name, r, sc, ve, str ) \
    if((r = (sc)) == ve){                       \
        perror(#name);                          \
        int errno_copy = errno;                 \
        print_error(str);          \
        errno = errno_copy;                     \
    }


#define SYSCALL_PRINT_NEQ( name, r, sc, ve, str ) \
    if((r = (sc)) != ve){                       \
        perror(#name);                          \
        int errno_copy = errno;                 \
        print_error(str);          \
        errno = errno_copy;                     \
    }

/**
* Brief utility procedure for printing errors
*
* @params str : string containing the number to extract
* @params ... : any other arguments to the print errors function
*/
static inline void print_error( const char* str ){
    const char err[] = "ERROR: ";
    //va_list argp;
    char* p = (char *) malloc(strlen(str) + strlen(err) + EXTRA_LEN_PRINT_ERROR);
    if(!p){
        perror("malloc");
        fprintf(stderr, "FATAL ERROR: in function 'print_error' \n");
        return;
    }
    strcpy(p, err);
    strcpy(p + strlen(err), str);
    //va_start(argp, str);
    //vfprintf(stderr, p, argp);
    //va_end(argp);
    free(p);
}

#endif


#ifndef THREADS_MANAGEMENT
#define THREADS_MANAGEMENT

#define LOCK(l) \
    if(pthread_mutex_lock(l) != 0){                 \
        fprintf(stderr, "FATAL ERROR: lock\n");     \
        pthread_exit((void*) EXIT_FAILURE);         \
    }

#define UNLOCK(l) \
    if(pthread_mutex_unlock(l) != 0){               \
        fprintf(stderr, "FATAL ERROR: unlock\n");   \
        pthread_exit((void*) EXIT_FAILURE);         \
    }

#define WAIT(c, l) \
    if(pthread_cond_wait(c, l) != 0){                   \
        fprintf(stderr, "FATAL ERROR: timed wait\n");   \
        pthread_exit((void*) EXIT_FAILURE);             \
    }

#endif
