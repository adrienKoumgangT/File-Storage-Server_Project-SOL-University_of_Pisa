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
#include <limits.h>

#ifndef UTILS_H
#define UTILS_H

/*
int isNumber( const char* s, long* n ){

    if(s == NULL) return 1;
    if(strlen(s) == 0) return 1;

    char *e = NULL;
    errno = 0;

    long val = strtol(s, &e, 10);
    if(errno == ERANGE) return 2; // overflow / underflow
    if(e != NULL && *e == (char)0){
        *n = val;
        return 0;
    }

    return 1;
}
*/

long getNumber( char* s ){

    int base;
    long val;
    char *endptr, *str;

    str = s;
    base = 10;

    errno = 0; /* To distinguish success/failure after call */
    val = strtol(str, &endptr, base);

    // Check for various errors
    if((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
            || (errno != 0 && val == 0)){
        perror("strtol");
        exit(EXIT_FAILURE);
    }

    if(endptr == str){
        fprintf(stderr, "No digits were found\n");
        exit(EXIT_FAILURE);
    }

    // If we got here, strtol() successfully parsed a number
    return val;

}


#endif

#ifndef CALL_EXIT
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

#ifndef CALL_PRINT_ERROR
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
    }

#endif


#ifndef PRINT_ERROR_AND_EXIT
#define PRINT_ERROR_AND_EXIT

#define IS_NULL_ERROR(r, f, s) \
    if((r = (f)) == NULL){ \
        fprintf(stderr, "FATAL ERROR : %s\n", s); \
        exit(EXIT_FAILURE); \
    }

#define IS_ZERO_ERROR(r, f, s) \
    if((r = (f)) == 0){ \
        fprintf(stderr, "FATAL ERROR : %s\n", s); \
        exit(EXIT_FAILURE); \
    }

#define IS_MINUS_ONE_ERROR(r, f, s) \
    if((r = (f)) == -1){ \
        fprintf(stderr, "FATAL ERROR : %s\n", s); \
        exit(EXIT_FAILURE); \
    }

#endif
