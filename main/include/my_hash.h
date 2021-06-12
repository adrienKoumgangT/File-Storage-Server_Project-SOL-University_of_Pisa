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
 * @file my_hash.h
 *
 * Definition of type hash_t
 *
 * A generic hash_t has :
 * 		- a name (filename) : a string representing the key of the file
 *		- a content (content) : the contents of the file
 *		- a size of content (size) : the size of the content
 *		- a pointer to another file (next) : to use to create a list of files
 *
 * @author adrien koumgang tegantchouang
 * @version 1.0
 * @date 00/05/2021
 */


 #if !defined(HASH_T)
 #define HASH_T

#include "my_file.h"

 // definition of element to be inserted in the hash table
 typedef struct _file_t data_hash_t;

 /* the structure of hash table */
 typedef struct _hash_t {
 	int size;
 	int number_of_item;
 	data_hash_t **table;
 	unsigned int (*hash_function)(char *);
 	int (*hash_key_compare)(char *, char *);
 } hash_t;

 /* Create a new hash table */
 hash_t* hash_create( const int, unsigned int (*hash_function)(char *),
                        int (*hash_key_compare)(char *, char *) );


 data_hash_t* hash_find( const hash_t*, char* );


 data_hash_t* hash_insert( hash_t*, char*, char* );


 data_hash_t* hash_update_insert( hash_t*, char*, char* );


 data_hash_t* hash_remove( hash_t*, char* );


 int hash_delete( hash_t*, char* );


 int hash_destroy( hash_t* );


 #endif
