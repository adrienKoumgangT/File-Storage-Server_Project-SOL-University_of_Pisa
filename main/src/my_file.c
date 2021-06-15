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
 * @file my_file.c
 *
 * Definition of the functions used for the hash table
 *
 * @author adrien koumgang tegantchouang
 * @version 1.0
 * @date 00/05/2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my_file.h"
#include "utils.h"



static inline void lockFile( file_t* ft ){
    LOCK(&ft->flock);
}

static inline void unlockFile( file_t* ft ){
    UNLOCK(&ft->flock);
}

static inline void unlockFileAndWait( file_t* ft ){
    WAIT(&ft->fcond, &ft->flock);
}

static inline void unlockFileAndSignal( file_t* ft ){
    SIGNAL(&ft->fcond);
    UNLOCK(&ft->flock);
}

 // simple hash function
 /**
 *  hash function that computes the hash value given to key
 *
 * @params key : key to find its hash value
 *
 * @returns : -1 if not valid key
 *              hash value it is valid key
 */
 unsigned int hash_function_for_file_t(char* key){
     char *str_key = (char *) key;
     if(!str_key)
        return -1;

    unsigned int key_value = 0;
    for(int i=0; str_key[i] != '\0'; i++)
        key_value += (unsigned int) str_key[i];

    return key_value;
 }

 // compare function
 /**
 *  function that allows you to compare two keys
 *
 * @params first_key : first key to compare
 * @params second_key : second value to compare
 *
 * @returns : 0 if egual
 *             -1, 1 otherwise
 */
 int hash_key_compare_for_file_t(char* first_key, char* second_key){
     return strcmp(first_key , second_key);
 }

/**
* print the file with its information and content
*
*   @params f : file to print the contents of
*/
 void file_print(file_t* f){
     if(f){
         fprintf(stdout, "%s  %ld  %s", f->key, f->size_data, f->data);
     }
 }

 void file_free(file_t* f){
     if(f){
         if(f->key) free(f->key);
         if(f->data) free(f->data);
         pthread_mutex_destroy(&(f->flock));
         pthread_cond_destroy(&(f->fcond));
         free(f);
     }
 }

int file_take_lock( file_t* ft, int fd_lock ){
    if(fd_lock <= 0) return -1;

    lockFile(ft);
    if(ft->log >= 0) unlockFileAndWait(ft);
    ft->log = fd_lock;
    FD_SET(fd_lock, &ft->set);
    unlockFile(ft);
    return 0;
}

int file_leave_lock( file_t* ft, int fd_lock ){
    if(fd_lock <= 0) return -1;

    lockFile(ft);
    if(ft->log != fd_lock){
        unlockFileAndSignal(ft);
        return -2;
    }
    ft->log = -1;
    unlockFileAndSignal(ft);
    return 0;
}

int file_has_lock( file_t* ft, int fd ){
    int r = 0;
    lockFile(ft);
    if(ft->log == fd) r = 1;
    unlockFile(ft);
    return r;
}

int file_read_content( file_t * ft, char* content, size_t* size_content ){
    lockFile(ft);
    if(content) free(content);
    content = (char *) malloc((ft->size_data+1) * sizeof(char));
    memset(content, '\0', ft->size_data+1);
    strncpy(content, ft->data, ft->size_data);
    *size_content = ft->size_data;
    unlockFileAndSignal(ft);
    return 0;
}

int file_write_content( file_t* ft, char* content, size_t size_content ){
    if(!content || size_content <= 0) return -1;

    lockFile(ft);
    if(ft->data) free(ft->data);
    ft->data = (char*) malloc((size_content+1) * sizeof(char));
    memset(ft->data, '\0', size_content+1);
    strncpy(ft->data, content, size_content);
    ft->size_data = size_content;
    unlockFileAndSignal(ft);
    return 0;
}

int file_append_content( file_t* ft, char* content, size_t size_content ){
    if(!content || size_content <= 0) return -1;

    lockFile(ft);
    if(ft->data) free(ft->data);
    ft->data = (char*) realloc(ft->data, (ft->size_data + size_content+1) * sizeof(char));
    strncat(ft->data, content, size_content);
    ft->size_data += size_content;
    unlockFileAndSignal(ft);
    return 0;
}

void file_add_fd( file_t* ft, int fd ){
    lockFile(ft);
    FD_SET(fd, &(ft->set));
    unlockFile(ft);
}

void file_remove_fd( file_t* ft, int fd ){
    lockFile(ft);
    FD_CLR(fd, &(ft->set));
    unlockFile(ft);
}

int file_has_fd( file_t* ft, int fd ){
    int r = 0;
    lockFile(ft);
    if(FD_ISSET(fd, &(ft->set))) r = 1;
    unlockFile(ft);
    return r;
}
