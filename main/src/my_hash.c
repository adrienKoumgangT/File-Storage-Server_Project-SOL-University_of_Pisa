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

// #define _POSIX_C_SOURCE 200112L

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <errno.h>
 #include <limits.h>
 #include <sys/types.h>
 #include <pthread.h>

 #include "my_hash.h"
 #include "my_file.h"
 #include "utils.h"

/* for hash_t */
static inline void lockHash( hash_t* ht ){
    LOCK(&ht->hlock);
}

static inline void unlockHash( hash_t* ht ){
    UNLOCK(&ht->hlock);
}

static inline void unlockHashAndWait( hash_t* ht ){
    WAIT(&ht->hcond, &ht->hlock);
}

static inline void unlockHashAndSignal( hash_t* ht ){
    SIGNAL(&ht->hcond);
    UNLOCK(&ht->hlock);
}

/* for node_h */
static inline void lockNodeHash( node_h* nh ){
    LOCK(&nh->nhlock);
}

static inline void unlockNodeHash( node_h* nh ){
    UNLOCK(&nh->nhlock);
}

static inline void unlockNodeHashAndWait( node_h* nh ){
    WAIT(&nh->nhcond, &nh->nhlock);
}

static inline void unlockNodeHashAndSignal( node_h* nh ){
    SIGNAL(&nh->nhcond);
    UNLOCK(&nh->nhlock);
}


/**
 * Create a new hash table
 *
 * @param size : dimention of table to create
 * @param hash_function : pointer to the hashing function to be used
 * @param hash_key_compare : pointer to the hash key comparison function to be used
 *
 * @returns : - pointer to new hash table
 *            - NULL if size <= 0
 *
 * @exceptions: if there are problems allocating the table in memory, it returns NULL
 */
hash_t* hash_create( const int size, unsigned int (*hash_function)(char *),
                    int (*hash_key_compare)(char *, char *) ){
     if(size <= 0)
        return NULL;

     hash_t *ht;
     int i;

     ht = (hash_t *) malloc(sizeof(hash_t));
     if(!ht)
         return NULL;
     memset(ht, '\0', sizeof(hash_t));
     ht->size = size;
     ht->number_of_item = 0;
     ht->table = (node_h **) malloc(size * sizeof( node_h* ));
     if(!ht->table)
         return NULL;
    if(pthread_mutex_init(&(ht->hlock), NULL) != 0){
        perror("pthread_mutex_init");
        return NULL;
    }
    if(pthread_cond_init(&(ht->hcond), NULL) != 0){
        perror("pthread_cond_init");
        pthread_mutex_destroy(&ht->hlock);
        return NULL;
    }
    for(i=0; i<ht->size; i++){
        ht->table[i] = (node_h *) malloc(sizeof(node_h));
        ht->table[i]->n = 0;
        ht->table[i]->list = NULL;
        if(pthread_mutex_init(&(ht->table[i]->nhlock), NULL) != 0){
            perror("pthread_mutex_init");
            return NULL;
        }
        if(pthread_cond_init(&(ht->table[i]->nhcond), NULL) != 0){
            perror("pthread_cond_init");
            pthread_mutex_destroy(&(ht->table[i]->nhlock));
            return NULL;
        }
    }
    ht->hash_function = hash_function;
    ht->hash_key_compare = hash_key_compare;

    return ht;
 }


/**
 * Searches for a specific item in the hash table
 *
 * @param ht : hash table
 * @param key : the key of the element to search for in the hash table
 *
 * @returns : - pointer to the data corresponding to the key.
 *            - If the key was not found, return NULL.
 */
data_hash_t* hash_find( const hash_t* ht, char* key ){
      if(!ht || !key)
         return NULL;

     node_h *ptr_n;
     data_hash_t* ptr;
     unsigned int key_hash;

     key_hash = (* ht->hash_function)(key) % ht->size;

     ptr_n = ht->table[key_hash];
     lockNodeHash(ptr_n);
     if(ptr_n->n <= 0){
         unlockNodeHash(ptr_n);
         return NULL;
     }
     ptr = ptr_n->list;
     while(ptr != NULL){
        if(ht->hash_key_compare(ptr->key, key) == 0){
            unlockNodeHash(ptr_n);
            return ptr;
        }
         ptr = ptr->next;
     }
     unlockNodeHash(ptr_n);

     return NULL;
  }

 /**
 * Insert new data into the hash table
 *
 * @param ht : the hash table
 * @param key : the key of then new item
 * @param data : then data of to the new item
 *
 * @returns : - NULL if the data is already present in the table
 *            -  data otherwise
 *
 * @exceptions : if one of the given parameters is NULL it returns NULL
 */
data_hash_t* hash_insert( hash_t* ht, char* key, size_t size_key, void* data, size_t size_data, int fd ){
      if(!ht || !key)
         return NULL;

     unsigned int key_hash;

     key_hash = (* ht->hash_function)(key) % ht->size;
 fprintf(stdout, "key = %s and hash_key = %d\n", key, key_hash);

    node_h* ptr_n = ht->table[key_hash];
    lockNodeHash(ptr_n);
     // check if the data is already present in the table
     data_hash_t *ptr = NULL;
     if(ptr_n->n > 0){
         ptr = ptr_n->list;
         while( ptr != NULL ){
             if(ht->hash_key_compare(ptr->key, key) == 0){
                 unlockNodeHash(ptr_n);
                 return NULL;
             }
             ptr=ptr->next;
         }
    }

     // if data was not found, add at start
    data_hash_t* new_item = file_create(key, size_key, data, size_data, fd);
    if(new_item == NULL){
        unlockNodeHash(ptr_n);
        return NULL;
    }
    new_item->next = ptr_n->list;
    ptr_n->list = new_item;
    ptr_n->n++;
    unlockNodeHash(ptr_n);
    lockHash(ht);
    ht->number_of_item++;
    unlockHash(ht);

    return new_item;
}

/**
 * Replace entry in hash table with the given entry
 *
 * @param ht: the hash table
 * #param data : pointer to the new item's data
 *
 * @returns : - pointer to the old item's data
 *            - NULL otherwise
 *
 * @exceptions : if one of the given parameters is NULL it returns NULL
 */
data_hash_t* hash_update_insert( hash_t* ht, char* key, size_t size_key, void* data, size_t size_data, int fd ){
      if(!ht || !data)
         return NULL;

      data_hash_t *curr = NULL, *prev = NULL;
      data_hash_t *old_data = NULL;
      unsigned int key_hash;

      key_hash = (* ht->hash_function)(key) % ht->size;

      node_h* ptr_n = ht->table[key_hash];
      lockNodeHash(ptr_n);
      // I look for the value to replace
      prev=NULL;
      curr=ptr_n->list;
      while( curr!=NULL ){
         if(ht->hash_key_compare(curr->key, key) == 0){

             if(prev == NULL) // if the item searched for at the top of the list
                 ptr_n->list = curr->next;
             else
                 prev->next = curr->next;

             old_data = curr;
             curr = NULL;
         }else{
            prev=curr;
            curr=curr->next;
        }
     }

    data_hash_t* new_item = NULL;
    if(old_data == NULL){
        new_item = file_create(key, size_key, data, size_data, fd);
        ptr_n->n++;
    }else{
        new_item = file_update_data(old_data, data, size_data);
        old_data->next = NULL;
    }

    new_item->next = ptr_n->list;
    ptr_n->list = new_item;
    unlockNodeHash(ptr_n);
    if(!old_data){
        lockHash(ht);
        ht->number_of_item++;
        unlockHash(ht);
    }

    return old_data;
}

/**
  * Searches for a specific item in the hash table
  *
  * @param ht : hash table
  * @param key : the key of the element to search for in the hash table
  *
  * @returns : - pointer to the data corresponding to the key.
  *            - If the key was not found, return NULL.
  */
data_hash_t* hash_update_insert_append( const hash_t* ht, char* key, size_t size_key, void* data, size_t size_data ){
    if(!ht || !key || !data)
        return NULL;

     data_hash_t *ptr = NULL;
     unsigned int key_hash;

     key_hash = (* ht->hash_function)(key) % ht->size;

     node_h* ptr_n = ht->table[key_hash];
     lockNodeHash(ptr_n);
     ptr = ptr_n->list;
     while(ptr != NULL){
        if(ht->hash_key_compare(ptr->key, key) == 0){
            file_append_content(ptr, data, size_data);
            unlockNodeHash(ptr_n);
            return ptr;
         }
        ptr = ptr->next;
    }
    unlockNodeHash(ptr_n);

    return NULL;
}

/**
 * removes an element from the table
 *
 * @param key : the key of the element to be removed
 * @param hash_function : pointer to the hashing function to be used
 *
 * @returns: - the old given remorse
 *           - null if it were not there
 *
 * @exceptions : if one of the given parameters is NULL it returns NULL
 */
data_hash_t* hash_remove( hash_t* ht, char* key ){
     if(!ht || !key)
        return NULL;

    data_hash_t *curr, *prev;
    unsigned int key_hash;

    key_hash = (* ht->hash_function)(key) % ht->size;

    if(ht->table[key_hash] == NULL)
         return NULL;

    // I look for the element with key key
    node_h* ptr_n = ht->table[key_hash];
    lockNodeHash(ptr_n);
    prev=NULL;
    curr=ptr_n->list;
    while( curr!=NULL ){
        if(ht->hash_key_compare(curr->key, key) == 0){
            if(prev == NULL)
                ptr_n->list = curr->next;
            else
                prev->next = curr->next;

            ptr_n->n--;
            unlockNodeHash(ptr_n);
            lockHash(ht);
            ht->number_of_item--;
            unlockHash(ht);
            return curr;
        }

        prev=curr;
        curr=curr->next;
    }
    unlockNodeHash(ptr_n);
    return NULL;
}

/**
 * Free one hash table entry located by key
 *
 * @param ht : the hash table to be freed
 * @param key : the key of the item to be deleted
 *
 * @returns : - 0 on success
 *            - -1 on failure
 */
int hash_delete( hash_t* ht, char *key ){
     if(!ht || !key)
        return -1;

    data_hash_t *curr = NULL, *prev = NULL;
    unsigned int key_hash = -1;

    key_hash = (* ht->hash_function)(key) % ht->size;

    node_h* ptr_n = ht->table[key_hash];
    lockNodeHash(ptr_n);
    prev = NULL;
    curr=ptr_n->list;
    while( curr!=NULL ){
        if(ht->hash_key_compare(curr->key, key) == 0){
            if(prev == NULL){
                ptr_n->list = curr->next;
            }else{
                prev->next = curr->next;
            }
            ptr_n->n--;
            unlockNodeHash(ptr_n);
            lockHash(ht);
            ht->number_of_item--;
            unlockHash(ht);
            file_free(curr);
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }
    unlockNodeHash(ptr_n);

    return -1;
  }

/**
* Free hash table structures
*
* @param ht : the hash table to be freed
*
* @return : - 0 onn success
*           - -1 on failure
*/
int hash_destroy( hash_t* ht ){
    if(!ht)
        return -1;

    node_h* ptr_n = NULL;
    data_hash_t *ptr_list, *curr, *next;

    lockHash(ht);
    // deletion of the key and the content of each element in the table
    for(int i=0; i<ht->size; i++){
        ptr_n = ht->table[i];
        lockNodeHash(ptr_n);
        ptr_list = ptr_n->list;
        for(curr=ptr_list; curr!=NULL;){
            next = curr->next;
            file_free(curr);
            curr = next;
        }
        ptr_n->n = 0;
        unlockNodeHash(ptr_n);
        pthread_mutex_destroy(&ptr_n->nhlock);
        pthread_cond_destroy(&ptr_n->nhcond);
        free(ptr_n);
    }
    unlockHash(ht);
    pthread_mutex_destroy(&ht->hlock);
    pthread_cond_destroy(&ht->hcond);
    free(ht->table);
    free(ht);

     return 0;
 }
