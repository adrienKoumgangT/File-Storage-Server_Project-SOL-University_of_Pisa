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

#ifndef UTILS_H_
#define UTILS_H_

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

// definition of measurement values
#ifndef MEASURE_VALUES
#define MEASURE_VALUES

// definition of the possible dimensions for the strings
#define STR_LEN 2048
#define STR_SIZE (STR_LEN * sizeof(char))
#define MAX_FILE_NAME 2048

#endif

#ifndef UTILS_FUNCTIONS
#define UTILS_FUNCTIONS

static inline long getNumber( char* s, int base ){
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

static inline char* getPath( char* filename, char* pathdir, size_t len_pathdir ){
    if(!filename) return NULL;
    size_t len = strlen(filename);
    if(len <= 0) return NULL;

    // if the path is already absolute, I return it
    if(filename[0] == '/') return filename;

    if(len + len_pathdir + 2 > MAX_FILE_NAME) return NULL;
    char* new_path = (char *) malloc((len + len_pathdir + 2) * sizeof(char));
    memset(new_path, '\0', len+len_pathdir+2);
    strncpy(new_path, pathdir, len_pathdir+1);
    strncat(new_path, "/", 2);
    strncat(new_path, filename, len+1);
    return new_path;
}

static inline char* setNameFile( const char* dirname, const char* filepath ){
    if(!dirname || !filepath) return NULL;
    size_t len_d = strlen(dirname);
    size_t len_f = strlen(filepath);
    if(len_d == 0 || len_f == 0) return NULL;

    char* pathfile = (char *) malloc((len_d + len_f + 2) * sizeof(char));
    memset(pathfile, '\0', len_d+len_f+2);
    strncpy(pathfile, dirname, len_d);
    strncat(pathfile, "/", 2);
    strncat(pathfile, filepath, len_f);
    pathfile[len_d+len_f] = '/';
    return pathfile;
}

static inline char* getNameFile( char* pathname ){
    if(!pathname) return NULL;

    char* namefile = NULL;
    char* tmp = NULL;
    char* token = strtok_r(pathname, "/", &tmp);
    while(token){
        namefile = token;
        token = strtok_r(NULL, "/", &tmp);
    }
    return namefile;
}

#endif /* UTILS_FUNCTIONS */


#ifndef SYSCALL_ERROR
#define SYSCALL_ERROR

#define SYSCALL_EXIT_EQ( name, r, sc, ve, str ) \
    if((r = (sc)) == ve){                       \
        perror(#name);                          \
        int errno_copy = errno;                 \
        print_error(str);                       \
        exit(errno_copy);                       \
    }

#define SYSCALL_EXIT_NEQ( name, r, sc, ve, str ) \
    if((r = (sc)) != ve){                       \
        perror(#name);                          \
        int errno_copy = errno;                 \
        print_error(str);                       \
        exit(errno_copy);                       \
    }


#define SYSCALL_RETURN_EQ( name, r, sc, ve, str ) \
    if((r = (sc)) == ve){                       \
        perror(#name);                          \
        int errno_copy = errno;                 \
        print_error(str);                       \
        errno = errno_copy;                     \
        return r;                               \
    }


#define SYSCALL_RETURN_NEQ( name, r, sc, ve, str ) \
    if((r = (sc)) != ve){                       \
        perror(#name);                          \
        int errno_copy = errno;                 \
        print_error(str);                       \
        errno = errno_copy;                     \
        return r;                               \
    }


#define SYSCALL_RETURN_VAL_EQ( name, r, sc, ve, vr, str ) \
    if((r = (sc)) == ve){                       \
        perror(#name);                          \
        int errno_copy = errno;                 \
        print_error(str);                       \
        errno = errno_copy;                     \
        return vr;                              \
    }


#define SYSCALL_RETURN_VAL_NEQ( name, r, sc, ve, vr, str ) \
    if((r = (sc)) != ve){                       \
        perror(#name);                          \
        int errno_copy = errno;                 \
        print_error(str);                       \
        errno = errno_copy;                     \
        return vr;                              \
}


#define SYSCALL_PRINT_EQ( name, r, sc, ve, str ) \
    if((r = (sc)) == ve){                       \
        perror(#name);                          \
        int errno_copy = errno;                 \
        print_error(str);                       \
        errno = errno_copy;                     \
    }


#define SYSCALL_PRINT_NEQ( name, r, sc, ve, str ) \
    if((r = (sc)) != ve){                       \
        perror(#name);                          \
        int errno_copy = errno;                 \
        print_error(str);                       \
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

#endif /* SYSCALL_ERROR */


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

#define SIGNAL(c) \
    if(pthread_cond_signal(c) != 0){                \
        fprintf(stderr, "FATAL ERROR: signal\n");   \
        pthread_exit((void*)EXIT_FAILURE);          \
    }

#define BCAST(c) \
    if(pthread_cond_broadcast(c) != 0){                 \
        fprintf(stderr, "FATAL ERROR: broadcast\n");    \
        pthread-exit((void*)EXIT_FAILURE);              \
    }

static inline int TRYLOCK( pthread_mutex_t* l ){
    int r=0;
    if((r = pthread_mutex_trylock(l)) != 0 && r!=EBUSY){
        fprintf(stderr, "FATAL ERROR: trylock\n");
        pthread_exit((void*)EXIT_FAILURE);
    }
    return r;
}

#endif /* THREADS_MANAGEMENT */


#endif /* UTILS_H_ */
