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
 * @file my_hash.c
 *
 * Implementation of utility functions for the hash_t data type
 *
 *
 * @author adrien koumgang tegantchouang
 * @version 1.0
 * @date 00/05/2021
 */


#ifndef DB_FILES_H
#define DB_FILES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>

typedef struct _file_t {
    char*                   key;
    size_t                  size_key;
    void*                   data;
    size_t                  size_data;
    fd_set                  set;
    int                     log;
    pthread_mutex_t         flock;
    pthread_cond_t          fcond;
    struct _file_t*    next;
} file_t;

typedef struct _node_hash_t {
    long n;
    size_t total_size;
    pthread_mutex_t nhlock;
    pthread_cond_t nhcond;
    file_t* list;
} node_hash_t;

typedef struct _hash_t {
   size_t size;
   long number_of_item;
   node_hash_t **table;
   pthread_mutex_t hlock;
   pthread_cond_t hcond;
} hash_t;

/**************** operations performed on the file type *******************/

// print file
 void file_print(file_t *);

// create file
file_t* file_create( char*, size_t, void*, size_t, int );

// free file
void file_free( file_t *);

// update data file
// file_t* file_update_data( file_t*, void*, size_t );

// take lock
int file_take_lock( file_t*, int  );

// leave lock
int file_leave_lock( file_t*, int );

//
int file_has_lock( file_t*, int );

// read the contents of a file
int file_read_content( file_t *, void*, size_t* );

// write the contents of a file
int file_write_content( file_t*, void*, size_t );

// append to the contents of a file
int file_append_content( file_t*, void*, size_t );

//
void file_add_fd( file_t*, int );

//
void file_remove_fd( file_t*, int );

//
int file_has_fd( file_t*, int );

/*************** operations performed on the hash table *******************/

// Create a new hash table
hash_t* hash_create (const int );

//
file_t* hash_find( const hash_t*, char* );

//
file_t* hash_insert( hash_t*, char*, size_t, void*, size_t, int );

//
file_t* hash_update_insert( hash_t*, char*, size_t, void*, size_t, int );

//
file_t* hash_update_insert_append( const hash_t*, char*, size_t, void*, size_t );

//
file_t* hash_remove( hash_t*, char* );

//
int hash_delete( hash_t*, char* );

//
int hash_destroy( hash_t* );

#endif
