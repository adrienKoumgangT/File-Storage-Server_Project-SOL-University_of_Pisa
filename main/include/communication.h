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
* @file communication.h
*
* definition of utility functions for read and write by socket
*
* @author adrien koumgang tegantchouang
* @version 1.0
* @date 00/05/2021
*/


#ifndef COMMUNICATION_H
#define COMMUNICATION_H


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

// how to open files
#define O_NORMAL            (0)
#define O_CREATE            (1)
#define O_LOCK              (2)
#define O_CREATE_LOCK       (3)

// success or failed operation
#define SUCCESS_O (0)
#define FAILED_O (-1)



/***  utility functions for communication functions between client servers  ***/

/** Evita letture parziali
 *
 *   \retval -1   errore (errno settato)
 *   \retval  0   se durante la lettura da fd leggo EOF
 *   \retval size se termina con successo
 */
static inline int readn(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
	if ((r=read((int)fd ,bufptr,left)) == -1) {
	    if (errno == EINTR) continue;
	    return -1;
	}
	if (r == 0) return 0;   // EOF
        left    -= r;
	bufptr  += r;
    }
    return size;
}

/** Evita scritture parziali
 *
 *   \retval -1   errore (errno settato)
 *   \retval  0   se durante la scrittura la write ritorna 0
 *   \retval  1   se la scrittura termina con successo
 */
static inline int writen(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
	if ((r=write((int)fd ,bufptr,left)) == -1) {
	    if (errno == EINTR) continue;
	    return -1;
	}
	if (r == 0) return 0;
        left    -= r;
	bufptr  += r;
    }
    return 1;
}

static inline int read_pathname(int fd, char** pathname, size_t* sz_p){
    if(readn(fd, (void *) sz_p, sizeof(size_t)) == -1){
        return -1;
    }
    *pathname = (char *) malloc(*sz_p);
    memset(*pathname, '\0', *sz_p);
    if((readn(fd, (void *) *pathname, *sz_p)) == -1){
        return -1;
    }
    return 0;
}

static inline int write_pathname(int fd, const char* pathname, size_t sz_p){
    if((writen(fd, (void *) &sz_p, sizeof(size_t))) == -1){
        return -1;
    }
    if((writen(fd, (void *) pathname, sz_p)) == -1){
        return -1;
    }
    return 0;
}

static inline int read_reason( int fd, char** reason ){
    size_t sz_r  = 0;
    if((readn(fd, &sz_r, sizeof(size_t))) == -1){
        return -1;
    }
    *reason = (char *) malloc(sz_r);
    if((readn(fd, *reason, sz_r)) == -1){
        return -1;
    }
    return 0;
}

static inline int write_reason( int fd, char* reason ){
    size_t sz_r = sizeof(reason);
    if((writen(fd, (void *) &sz_r, sizeof(size_t))) == -1){
        return -1;
    }
    if((writen(fd, (void *) reason, sz_r)) == -1){
        return -1;
    }
    return 0;
}

static inline int write_data( int fd, void* data, size_t sz_d ){
    if((writen(fd, (void *) &sz_d, sizeof(size_t))) == -1){
        return -1;
    }
    if((writen(fd, (void *) data, sz_d)) == -1){
        return -1;
    }
    return 0;
}

static inline int read_data( int fd, void** data, size_t* size ){
    if((readn(fd, size, sizeof(size_t))) == -1){
        return -1;
    }
    *data = malloc(*size);
    if((readn(fd, *data, *size)) == -1){
        return -1;
    }
    return 0;
}

static inline int write_file_eject( int fd, int n, char** pathname, size_t* size_p, void** data, size_t* size_d ){
    if((writen(fd, &n, sizeof(int))) == -1){
        return -1;
    }
    if(n > 0){
        for(int i=0; i<n; i++){
            if((write_pathname(fd, pathname[i], size_p[i])) == -1){
                return -1;
            }
            if((write_data(fd, data[i], size_d[i])) == -1){
                return -1;
            }
        }
    }
    return 0;
}

static inline int read_file_eject( int fd, int* n, char*** pathname, size_t** size_p, void*** data, size_t** size_d){
    if((readn(fd, n, sizeof(int))) == -1){
        return -1;
    }
    if(*n > 0){
        for(int i=0; i<*n; i++){
            if((read_pathname(fd, &(*pathname[i]), size_p[i])) == -1){
                return -1;
            }
            if((read_data(fd, &(*data[i]), size_d[i])) == -1){
                return -1;
            }
        }
    }
    return 0;
}

#endif
