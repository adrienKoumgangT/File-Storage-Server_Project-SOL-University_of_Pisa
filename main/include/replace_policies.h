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
 * @file queue.c
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

#ifndef _REPLACE_POLICIES_
#define _REPLACE_POLICIES_

#include <pthread.h>

typedef struct _Node_p{
    char* p_key;
    size_t p_sz;
    struct _Node_p *next;
} Node_p;

typedef struct _Queue_p{
    Node_p*     head;
    Node_p*     tail;
    unsigned long qplen;
    pthread_mutex_t qplock;
    pthread_cond_t qpcond;
} Queue_p;

Queue_p* initQueueP( void );

void deleteQueueP( Queue_p* );

int push_qp( Queue_p*, char*, size_t );

char* pop_qp( Queue_p* );

char* get_put_last_qp( Queue_p* );

Node_p* findNodeP( Queue_p*, char* );

void deleteNodeP( Queue_p*, char* );

unsigned long length_qp( Queue_p* );

int repositionNodeP( Queue_p*, char*, size_t );

int take_lock_queueP( Queue_p* );

int release_lock_queueP( Queue_p* );

#endif
