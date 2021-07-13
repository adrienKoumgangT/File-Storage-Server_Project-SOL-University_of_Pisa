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
* @file read_write.h
*
* definition of utility functions
*
* @author adrien koumgang tegantchouang
* @version 1.0
* @date 00/05/2021
*/

#ifndef READ_WRITE_H
#define READ_WRITE_H

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <time.h>

/** Evita scritture parziali
 *
 *   \retval -1   errore (errno settato)
 *   \retval  0   se durante la scrittura la write ritorna 0
 *   \retval  1   se la scrittura termina con successo
 */
static inline int writen_f(int fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
        if ((r=write((int)fd ,bufptr,left)) == -1) {
            if (errno == EINTR) continue;
            perror("write");
            return -1;
        }
        if (r == 0) return 0;
            left    -= r;
        bufptr  += r;
    }
    return 1;
}

static inline int read_file( const char *pathname, void** str, size_t* sz, int flag ){

    int fd;
    if((fd = open(pathname, flag)) == -1){
        return -1;
    }
    struct stat info;
    fstat(fd, &info);
    *sz = (size_t) info.st_size;
    //if(*str)
        //free(*str);
    *str = malloc(*sz);
    if( read(fd, *str, *sz) == -1 ){
        return -1;
    }

    close(fd);

    return *sz;
}


static inline int write_file( char *pathname, void* str, size_t sz ){

    if(!str)
        return -1;

    int fd;
    if((fd = open(pathname, O_CREAT | O_TRUNC | O_WRONLY, 0666)) == -1){
        fprintf(stdout, ">>> echec de création du fichier: %s\n", pathname);
        return -1;
    }

    if(writen_f(fd, str, sz) == -1){
        fprintf(stdout, ">> échec d'écriture du fihcier");
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}

static inline int write_file_in_dir( char* dirname, char* namefile, char* str, size_t sz ){
    DIR *dir;
    if((dir = opendir(dirname)) == NULL){
        perror("opendir");
        fprintf(stderr, "ERROR: error opening directory '%s'\n", dirname);
        return -1;
    }else{

    }

    return 0;
}

#endif
