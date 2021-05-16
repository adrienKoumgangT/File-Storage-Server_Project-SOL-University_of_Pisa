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
* definition of utility functions
*
* @author adrien koumgang tegantchouang
* @version 1.0
* @date 00/05/2021
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if !defined(UTILS_H)
#define UTILS_H

int getNumber( const char* s, unsigned long* n ){
    if(!s || (strlen(s) == 0))
        return -1;

    char *e = NULL;
    errno = 0;
    long val = strtol(s, &e, 10);
    if(errno == ERANGE) return 1; // overflow / underflow
    if(e != NULL && *e == (char)0){
        *n = (unsigned long) val;
        return 0;
    }

    return -1;
}



#endif

#if !defined(CALL_EXIT)
#define CALL_EXIT

#define VALUE_EXIT(v, r, f, s) \
    if((r = (f)) == v){ \
        perror(#s); \
        exit(EXIT_FAILURE); \
    }

#define NULL_EXIT(r, f, s) \
    if((r = (f)) == NULL){ \
        perror(#s); \
        exit(EXIT_FAILURE); \
    }

#define ZERO_EXIT(r, f, s) \
    if((r = (f)) == 0){ \
        perror(#s); \
        exit(EXIT_FAILURE); \
    }

#define MINUS_ONE_EXIT(r, f, s) \
    if((r = (f)) == -1){ \
        perror(#s); \
        exit(EXIT_FAILURE); \
    }

#endif

#if !defined(CALL_PRINT_ERROR)
#define CALL_PRINT_ERROR

#define IS_NULL_PRNT_ERROR(r, f, s) \
    if((r = (f)) == NULL){ \
        fprintf(stderr, "NULL_PRNT_ERROR : %s\n", s); \
    }

#define IS_ZERO_PRINT_ERROR(r, f, s) \
    if((r = (f)) == 0){ \
        fprintf(stderr, "ZERO_PRINT_ERROR : %s\n", s); \
    }

#define IS_MINUS_ONE_PRINT_ERROR(r, f, s) \
    if((r = (f)) == -1){ \
        fprintf(stderr, "MINUS_PRINT_ERROR : %s\n", s); \

#endif