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

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <errno.h>
 #include <limits.h>
 #include <sys/types.h>
 #include <pthread.h>

 #include "my_hash.h"
 #include "my_file.h"



static pthread_mutex_t *hlock;
static pthread_cond_t *hcond;


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

     ht->size = size;
     ht->number_of_item = 0;
     ht->table = (data_hash_t **) malloc(size * sizeof(data_hash_t *));
     if(!ht->table)
         return NULL;
    hlock = (pthread_mutex_t *) malloc(size * sizeof(pthread_mutex_t));
    hcond = (pthread_cond_t *) malloc(size * sizeof(pthread_cond_t));
    for(i=0; i<ht->size; i++){
        ht->table[i] = NULL;
        if(pthread_mutex_init(&hlock[i], NULL) != 0){
            perror("pthread_mutex_init");
            return NULL;
        }
        if(pthread_cond_init(&hcond[i], NULL) != 0){
            perror("pthread_cond_init");
            pthread_mutex_destroy(&hlock[i]);
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

     data_hash_t *ptr;
     unsigned int key_hash;

     key_hash = (* ht->hash_function)(key) % ht->size;

     pthread_mutex_lock(&hlock[key_hash]);
     ptr = ht->table[key_hash];
     while(ptr != NULL){
        if(ht->hash_key_compare(ptr->key, key)){
            pthread_mutex_unlock(&hlock[key_hash]);
            return ptr;
        }
         ptr = ptr->next;
     }
     pthread_mutex_unlock(&hlock[key_hash]);

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

/**
*
*/
data_hash_t* hash_insert( hash_t* ht, char* key, char* data, int fd ){
      if(!ht || !key || !data)
         return NULL;

     unsigned int key_hash;

     key_hash = (* ht->hash_function)(key) % ht->size;
 fprintf(stdout, "key = %s and hash_key = %d\n", key, key_hash);

    pthread_mutex_lock(&hlock[key_hash]);
     // check if the data is already present in the table
     for(data_hash_t *ptr=ht->table[key_hash]; ptr != NULL; ptr=ptr->next)
         if(ht->hash_key_compare(ptr->key, key)){
             pthread_mutex_unlock(&hlock[key_hash]);
             return NULL;
         }

     // if data was not found, add at start
     data_hash_t* new_item = (data_hash_t *) malloc(sizeof(data_hash_t));
     if(!new_item){
         pthread_mutex_unlock(&hlock[key_hash]);
         return NULL;
     }

    new_item->next = ht->table[key_hash];
    new_item->key = key;
    new_item->size_key = sizeof(key);
    new_item->data = data;
    new_item->size_data = sizeof(data);
    new_item->log = -1;
    FD_ZERO(&new_item->set);
    FD_SET(fd, &new_item->set);
    ht->table[key_hash] = new_item;
    ht->number_of_item++;
    pthread_mutex_unlock(&hlock[key_hash]);
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
data_hash_t* hash_update_insert( hash_t* ht, char* key, size_t size_key, char* data, size_t size_data ){
      if(!ht || !data)
         return NULL;

      data_hash_t *curr, *prev;
      data_hash_t *old_data = NULL;
      unsigned int key_hash;

      key_hash = (* ht->hash_function)(key) % ht->size;

      pthread_mutex_lock(&hlock[key_hash]);
      // I look for the value to replace
      for(prev=NULL, curr=ht->table[key_hash]; curr!=NULL; prev=curr, curr=curr->next)
         if(ht->hash_key_compare(curr->key, key)){

             if(prev == NULL) // if the item searched for at the top of the list
                 ht->table[key_hash] = curr->next;
             else
                 prev->next = curr->next;

             old_data = curr;
             curr = NULL;
         }

     data_hash_t *new_item = (data_hash_t *) malloc(sizeof(data_hash_t));
     new_item->key = key;
     new_item->size_key = size_key;
     new_item->data = data;
     new_item->size_data = size_data;
     new_item->log = -1;
     if(old_data) new_item->set = old_data->set;
     new_item->next = ht->table[key_hash];
     ht->table[key_hash] = new_item;
     pthread_mutex_unlock(&hlock[key_hash]);

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
data_hash_t* hash_update_cat( const hash_t* ht, char* key, size_t size_key, char* data, size_t size_data ){
    if(!ht || !key || !data)
        return NULL;

     data_hash_t *ptr;
     unsigned int key_hash;

     key_hash = (* ht->hash_function)(key) % ht->size;

     pthread_mutex_lock(&hlock[key_hash]);
     ptr = ht->table[key_hash];
     while(ptr != NULL){
        if(ht->hash_key_compare(ptr->key, key)){
            file_append_content(ptr, data, size_data);
            pthread_mutex_unlock(&hlock[key_hash]);
            return ptr;
         }
        ptr = ptr->next;
    }
    pthread_mutex_unlock(&hlock[key_hash]);

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
     prev=NULL;
     curr=ht->table[key_hash];
     pthread_mutex_lock(&hlock[key_hash]);
     while( curr!=NULL ){
         if(ht->hash_key_compare(curr->key, key)){
             if(prev == NULL)
                 ht->table[key_hash] = curr->next;
             else
                 prev->next = curr->next;

            pthread_mutex_unlock(&hlock[key_hash]);
            ht->number_of_item--;
            return curr;
             //curr = NULL;
         }

         prev=curr;
         curr=curr->next;
     }
     pthread_mutex_unlock(&hlock[key_hash]);
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

     data_hash_t *curr, *prev;
     unsigned int key_hash;

     key_hash = (* ht->hash_function)(key) % ht->size;

     prev = NULL;
     curr=ht->table[key_hash];
     pthread_mutex_unlock(&hlock[key_hash]);
     while( curr!=NULL ){
         if(ht->hash_key_compare(curr->key, key)){
             if(prev == NULL){
                 ht->table[key_hash] = curr->next;
             }else{
                 prev->next = curr->next;
             }
             pthread_mutex_unlock(&hlock[key_hash]);
             file_free(curr);
             ht->number_of_item--;
             return 0;
         }
         prev = curr;
         curr = curr->next;
     }
     pthread_mutex_unlock(&hlock[key_hash]);

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

     data_hash_t *ptr_list, *curr, *next;

     // deletion of the key and the content of each element in the table
     for(int i=0; i<ht->size; i++){
         ptr_list = ht->table[i];
         for(curr=ptr_list; curr!=NULL;){
             next = curr->next;
             file_free(curr);
             curr = next;
         }
     }

     free(ht->table);
     free(ht);

     return 0;
 }
