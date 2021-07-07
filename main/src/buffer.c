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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>

#include "utils.h"
#include "buffer.h"



/************************** utility functions ************************/

static inline NodeB_t* allocNodeB( void ){
    return malloc(sizeof(NodeB_t));
}

static inline Buffer_t* allocBuffer( void ){
    return malloc(sizeof(Buffer_t));
}

static inline void freeNodeB( NodeB_t* node ){
    free((void*)node);
}

static inline void lockBuffer( Buffer_t* b ){
    LOCK(&b->block);
}

static inline void unlockBuffer( Buffer_t* b ){
    UNLOCK(&b->block);
}

static inline void unlockBufferAndWait( Buffer_t* b ){
    WAIT(&b->bcond, &b->block);
}

static inline void unlockBufferAndSignal( Buffer_t* b ){
    SIGNAL(&b->bcond);
    UNLOCK(&b->block);
}


/**************************** tail interface *************************/

Buffer_t *initBuffer( void ) {
    Buffer_t *b = allocBuffer();
    if (!b) return NULL;
    b->head = NULL;
    b->tail = NULL;
    b->blen = 0;
    if (pthread_mutex_init(&b->block, NULL) != 0) {
    	perror("mutex init");
    	return NULL;
    }
    if (pthread_cond_init(&b->bcond, NULL) != 0) {
    	perror("mutex cond");
    	if (&b->block) pthread_mutex_destroy(&b->block);
    	return NULL;
    }
    return b;
}

void deleteBuffer(Buffer_t *b) {
    while(b->head != NULL) {
    	NodeB_t *nb = (NodeB_t*)b->head;
    	b->head = b->head->next;
        if(nb->data) free(nb->data);
    	freeNodeB(nb);
    }
    if (&b->block)  pthread_mutex_destroy(&b->block);
    if (&b->bcond)  pthread_cond_destroy(&b->bcond);
    free(b);
}

int pushBuffer(Buffer_t *b, void *data, size_t size_data) {
    if ((b == NULL) || (data == NULL)){
        errno= EINVAL;
        return -1;
    }

    NodeB_t *nb = allocNodeB();
    if (!nb)
        return -1;

    nb->data = malloc(sizeof(size_data));
    memcpy(nb->data, data, size_data);
    nb->next = NULL;

    lockBuffer(b);
    if(b->head == NULL){
        b->head = nb;
        b->tail = nb;
    }else{
        b->tail->next = nb;
        b->tail       = nb;
    }
    b->blen      += 1;
    unlockBufferAndSignal(b);
    return 0;
}

void *popBuffer(Buffer_t *b) {
    if (b == NULL) {
        errno= EINVAL;
        return NULL;
    }
    lockBuffer(b);
    while(b->head == NULL) {
	       unlockBufferAndWait(b);
    }
    // locked
    assert(b->head);
    NodeB_t *nb  = (NodeB_t *)b->head;
    void *data = b->head->data;
    b->head    = b->head->next;
    b->blen   -= 1;
    assert(b->blen>=0);
    unlockBuffer(b);
    freeNodeB(nb);
    return data;
}

unsigned long lengthBuffer(Buffer_t *b) {
    lockBuffer(b);
    unsigned long len = b->blen;
    unlockBuffer(b);
    return len;
}
