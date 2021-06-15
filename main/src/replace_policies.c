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

#include "replace_policies.h"
#include "utils.h"

/***************************** utility functions ****************************/

static inline Node_p* allocNodeP( void ){
    return (Node_p *) malloc(sizeof(Node_p));
}

static inline Queue_p* allocQueueP( void ){
    return (Queue_p *) malloc(sizeof(Queue_p));
}

static inline void freeNodeP( Node_p* node ){
    free((void *) node);
}

static inline void lockQueueP( Queue_p* qp ){
    LOCK(&qp->qplock);
}

static inline void unlockQueueP( Queue_p* qp){
    UNLOCK(&qp->qplock);
}

static inline void unlockQueuePAndWait( Queue_p* qp ){
    WAIT(&qp->qpcond, &qp->qplock);
}

static inline void unlockQueuePAndSignal( Queue_p* qp ){
    SIGNAL(&qp->qpcond);
    UNLOCK(&qp->qplock)
}


Queue_p* initQueueP( void ){
    Queue_p* qp = allocQueueP();
    if(!qp) return NULL;
    qp->head = allocNodeP();
    if(!qp->head) return NULL;
    qp->head->p_key     = NULL;
    qp->head->p_sz      = NULL;
    qp->head->next      = NULL;
    qp->tail = qp->head;
    qp->qplen = 0;
    if(pthread_mutex_init(&qp->qplock, NULL) != 0){
        perror("pthread_mutex_init");
        return NULL;
    }
    if(pthread_cond_init(&qp->qpcond, NULL) != 0){
        perror("pthread_cond_init");
        return NULL;
    }
    return qp;
}

void deleteQueueP( Queue_p* qp ){
    while(qp->head != qp->tail){
        Node_p* p   = qp->head;
        qp->head    = qp->head->next;
        freeNodeP(p);
    }
    if(qp->head) freeNodeP(qp->head);
    if(&qp->qplock) pthread_mutex_destroy(&qp->qplock);
    if(&qp->qpcond) pthread_cond_destroy(&qp->qpcond);
    free((void *) qp);
}

int push_qp( Queue_p* qp, char** p_key, size_t* p_sz ){
    if((qp == NULL) || (p_key == NULL) || (p_sz == NULL)){
        errno = EINVAL;
        return -1;
    }

    Node_p* n = allocNodeP();
    if(!n)
        return -1;

    n->p_key    = p_key;
    n->p_sz     = p_sz;
    n->next     = NULL;

    lockQueueP(qp);
    qp->tail->next  = n;
    qp->tail        = n;
    qp->qplen       += 1;
    unlockQueuePAndSignal(qp);
    return 0;
}

int reset_sz_qp(Queue_p* qp, char** key, size_t* p_sz){
    if(qp == NULL || key == NULL){
        errno = EINVAL;
        return -1;
    }

    lockQueueP(qp);
    Node_p* p = qp->head->next;
    while(p != NULL){
        if(strcmp(*key, *(p->p_key)) == 0){
            p->p_sz = p_sz;
            unlockQueuePAndSignal(qp);
            return 0;
        }else{
            p = p->next;
        }
    }
    unlockQueuePAndSignal(qp);
    return 0;
}

char* pop_qp( Queue_p* qp ){
    if(qp == NULL){
        errno = EINVAL;
        return NULL;
    }

    lockQueueP(qp);
    while(qp->head == qp->tail){
        unlockQueuePAndWait(qp);
    }
    assert(qp->head->next);
    Node_p* n   = qp->head;
    char* key   = *((qp->head->next)->p_key);
    qp->head    = qp->head->next;
    qp->qplen   -= 1;
    assert(qp->qplen >= 0);
    unlockQueueP(qp);
    freeNodeP(n);
    return key;
}

Node_p* findNodeP( Queue_p* qp, char* key ){
    if(qp == NULL || key == NULL){
        errno = EINVAL;
        return NULL;
    }

    lockQueueP(qp);
    Node_p* prev = qp->head;
    Node_p* p = qp->head->next;
    while(p != NULL){
        if(strcmp(key, *(p->p_key)) == 0){
            prev->next = p->next;
            unlockQueuePAndSignal(qp);
            return p;
        }else{
            prev = p;
            p = p->next;
        }
    }
    unlockQueuePAndSignal(qp);
    return NULL;
}

void deleteNodeP( Queue_p* qp, char* key ){
    Node_p* n = NULL;
    if((n = findNodeP(qp, key)) != NULL){
        freeNodeP(n);
    }
}

unsigned long length_qp( Queue_p* qp ){
    lockQueueP(qp);
    unsigned long len = qp->qplen;
    unlockQueueP(qp);
    return len;
}

int repositionNodeP( Queue_p* qp, char* key ){
    if(qp == NULL || key == NULL){
        errno = EINVAL;
        return -1;
    }

    Node_p* n = NULL;
    if((n = findNodeP(qp, key)) != NULL){
        push_qp(qp, n->p_key, n->p_sz);
        freeNodeP(n);
        return 0;
    }else{
        return -1;
    }
}
