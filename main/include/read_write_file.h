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

static inline int read_file( const char *pathname, char* str, size_t* sz, int flag ){

    int fd;
    if((fd = open(pathname, O_RDONLY)) == -1){ // TODO: cambiare O_RDONLY con flag
        return -1;
    }
    struct stat info;
    fstat(fd, &info);
    *sz = info.st_size;
    if(str)
        free(str);
    str = malloc(*sz);
    if( read(fd, str, *sz) == -1 ){
        return -1;
    }

    close(fd);

    return *sz;
}


static inline int write_file( char *pathname, char* str, size_t sz ){

    if(!str)
        return -1;

    int fd;
    if((fd = open(pathname, O_RDONLY)) == -1){
        return -1;
    }

    if(write(fd, str, sz) == -1){
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
