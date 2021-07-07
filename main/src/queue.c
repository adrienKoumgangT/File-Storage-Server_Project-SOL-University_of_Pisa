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
#include "queue.h"



/************************** utility functions ************************/

static inline Node_t* allocNode( void ){
    return malloc(sizeof(Node_t));
}

static inline Queue_t* allocQueue( void ){
    return malloc(sizeof(Queue_t));
}

static inline void freeNode( Node_t* node ){
    if(node->data) free(node->data);
    free((void*)node);
}

static inline void lockQueue( Queue_t* q ){
    LOCK(&q->qlock);
}

static inline void unlockQueue( Queue_t* q ){
    UNLOCK(&q->qlock);
}

static inline void unlockQueueAndWait( Queue_t* q ){
    WAIT(&q->qcond, &q->qlock);
}

static inline void unlockQueueAndSignal( Queue_t* q ){
    SIGNAL(&q->qcond);
    UNLOCK(&q->qlock);
}


/**************************** tail interface *************************/

Queue_t *initQueue( void ) {
    Queue_t *q = allocQueue();
    if (!q) return NULL;
    q->head = allocNode();
    if (!q->head) return NULL;
    q->head->data = NULL;
    q->head->next = NULL;
    q->tail = q->head;
    q->qlen = 0;
    if (pthread_mutex_init(&q->qlock, NULL) != 0) {
	perror("mutex init");
	return NULL;
    }
    if (pthread_cond_init(&q->qcond, NULL) != 0) {
	perror("mutex cond");
	if (&q->qlock) pthread_mutex_destroy(&q->qlock);
	return NULL;
    }
    return q;
}

void deleteQueue(Queue_t *q) {
    while(q->head != q->tail) {
	Node_t *p = (Node_t*)q->head;
	q->head = q->head->next;
	freeNode(p);
    }
    if (q->head) freeNode((void*)q->head);
    if (&q->qlock)  pthread_mutex_destroy(&q->qlock);
    if (&q->qcond)  pthread_cond_destroy(&q->qcond);
    free(q);
}

int push(Queue_t *q, void *data) {
    if ((q == NULL) || (data == NULL)){
        errno= EINVAL;
        return -1;
    }

    Node_t *n = allocNode();
    if (!n)
        return -1;

    n->data = data;
    n->next = NULL;

    lockQueue(q);
    q->tail->next = n;
    q->tail       = n;
    q->qlen      += 1;
    unlockQueueAndSignal(q);
    return 0;
}

void *pop(Queue_t *q) {
    if (q == NULL) { errno= EINVAL; return NULL;}
    lockQueue(q);
    while(q->head == q->tail) {
	unlockQueueAndWait(q);
    }
    // locked
    assert(q->head->next);
    Node_t *n  = (Node_t *)q->head;
    void *data = (q->head->next)->data;
    q->head    = q->head->next;
    q->qlen   -= 1;
    assert(q->qlen>=0);
    unlockQueue(q);
    freeNode(n);
    return data;
}

unsigned long length(Queue_t *q) {
    lockQueue(q);
    unsigned long len = q->qlen;
    unlockQueue(q);
    return len;
}
