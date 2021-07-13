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
    if(node->p_key) free(node->p_key);
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
    qp->head        = NULL;
    qp->head        = NULL;
    qp->tail        = NULL;
    qp->qplen       = 0;
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
    while(qp->head != NULL){
        Node_p* p   = qp->head;
        qp->head    = qp->head->next;
        freeNodeP(p);
    }
    qp->qplen = 0;
    if(&qp->qplock) pthread_mutex_destroy(&qp->qplock);
    if(&qp->qpcond) pthread_cond_destroy(&qp->qpcond);
    free((void *) qp);
}

int push_qp( Queue_p* qp, char* p_key, size_t p_sz ){
    if((qp == NULL) || (p_key == NULL)){
        errno = EINVAL;
        return -1;
    }

    Node_p* n = allocNodeP();
    if(!n)
        return -1;

    n->p_sz     = p_sz;
    n->p_key = malloc(p_sz);
    memset(p_key, '\0', p_sz);
    strncpy(n->p_key,p_key, p_sz);
    n->next     = NULL;

    lockQueueP(qp);
    if(qp->head == NULL){
        qp->head = n;
        qp->tail = n;
        qp->qplen = 1;
    }else{
        qp->tail->next  = n;
        qp->tail        = n;
        qp->qplen       += 1;
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
    while(qp->head == NULL){
        unlockQueuePAndWait(qp);
    }
    Node_p* n   = qp->head;
    qp->head    = qp->head->next;
    qp->qplen   -= 1;
    unlockQueueP(qp);

    char* key   = (char *) malloc(n->p_sz);
    memset(key, '\0', n->p_sz);
    strncpy(key, n->p_key, n->p_sz);
    freeNodeP(n);
    return key;
}

char* get_put_last_qp( Queue_p* qp ){
    char* str_get = NULL;
    lockQueueP(qp);
    if(qp->head == NULL){
        unlockQueuePAndSignal(qp);
        return NULL;
    }
    str_get = (char *) malloc(qp->head->p_sz);
    memset(str_get, '\0', qp->head->p_sz);
    strncpy(str_get, qp->head->p_key, qp->head->p_sz);
    if(qp->head->next == NULL){
        unlockQueuePAndSignal(qp);
        fprintf(stdout, ">>> str_get1 = %s\n", str_get);
        return str_get;
    }
    Node_p* np = qp->head;
    qp->head = qp->head->next;
    np->next  = NULL;
    qp->tail->next = np;
    qp->tail = qp->tail->next;
    unlockQueuePAndSignal(qp);
    fprintf(stdout, ">>> str_get2 = %s\n", str_get);
    return str_get;
}

Node_p* findNodeP( Queue_p* qp, char* key ){
    if(qp == NULL || key == NULL){
        errno = EINVAL;
        return NULL;
    }

    lockQueueP(qp);
    Node_p* prev = NULL;
    Node_p* p = qp->head;
    while(p != NULL){
        if(strcmp(key, p->p_key) == 0){
            if(prev != NULL) prev->next = p->next;
            else qp->head = p->next;
            qp->qplen--;
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
        lockQueueP(qp);
        qp->qplen--;
        unlockQueuePAndSignal(qp);
    }
}

unsigned long length_qp( Queue_p* qp ){
    lockQueueP(qp);
    unsigned long len = qp->qplen;
    unlockQueueP(qp);
    return len;
}

int repositionNodeP( Queue_p* qp, char* key, size_t p_sz ){
    if(qp == NULL || key == NULL){
        errno = EINVAL;
        return -1;
    }

    Node_p* n = NULL;
    if((n = findNodeP(qp, key)) != NULL){
        push_qp(qp, n->p_key, p_sz);
        freeNodeP(n);
        return 0;
    }else{
        return -1;
    }
}

int take_lock_queueP(Queue_p* qp){
    lockQueueP(qp);
    return 0;
}

int release_lock_queueP(Queue_p* qp){
    unlockQueuePAndSignal(qp);
    return 0;
}
