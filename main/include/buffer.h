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
 * @file queue.h
 *
 * Definition of type Queue_t
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


#ifndef BUFFER_H_
#define BUFFER_H_

#include <pthread.h>

/**
*
*/
typedef struct NodeB {
    void*           data;
    struct NodeB*    next;
} NodeB_t;

/**
*
*/
typedef struct Buffer {
    NodeB_t*             head;
    NodeB_t*             tail;
    unsigned long       blen;
    pthread_mutex_t     block;
    pthread_cond_t      bcond;
} Buffer_t;


Buffer_t* initBuffer( void );

void deleteBuffer( Buffer_t* q );

int pushBuffer( Buffer_t* q, void* data, size_t );

void* popBuffer( Buffer_t* q );

unsigned long lengthBuffer( Buffer_t* q );

#endif /* BUFFER_H_ */
