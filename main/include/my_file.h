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
 * @file my_file.h
 *
 * Definition of type file_t
 *
 * A generic file has :
 * 		- a name (key) : a string representing the key of the file
 *		- a content (data) : the contents of the file
 *		- a size of content (size) : the size of the content
 *		- a pointer to another file (next) : to use to create a list of files
 *
 * @author adrien koumgang tegantchouang
 * @version 1.0
 * @date 00/05/2021
 */

 #if !defined(FILE_T)
 #define FILE_T

#include <pthread.h>

/**
* format of a generic file
*
* key : the unique identification key of a file (also its pathname)
* data : string containing the contents of the file
* size : the size of the file
* next : pointer to a possible file
*/
 typedef struct _file_t { // TODO: da completare sugli altri file
 	char*                   key;
    size_t                  size_key;
 	void*                   data;
    size_t                  size_data;
    fd_set                  set;
    int                     log;
    pthread_mutex_t         flock;
    pthread_cond_t          fcond;
 	struct _file_t*         next;
 } file_t;


 // simple hash function
 unsigned int hash_function_for_file_t(char*);

 // compare function
 int hash_key_compare_for_file_t(char*, char*);

 // print file
 void file_print(file_t *);

// create file
file_t* file_create( char*, size_t, void*, size_t, int );

// free file
void file_free( file_t *);

// update data file
file_t* file_update_data( file_t*, void*, size_t );

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

#endif
