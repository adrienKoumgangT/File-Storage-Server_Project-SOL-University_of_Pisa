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

 #include "my_hash.h"
 #include "my_file.h"


/*
 typedef struct _hash_t {
 	int size;
 	int number_of_item;
 	data_hash_t **table;
 	unsigned int (*hash_function)(char *);
 	int (*hash_key_compare)(char *, char *);
 } hash_t;
*/

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
 hash_t* hash_create( const int size, unsigned int (*hash_function)(char *), int (*hash_key_compare)(char *, char *) ){
     if(size <= 0)
        return NULL;

     hash_t *ht;
     int i;

     ht = (hash_t *) malloc(sizeof(hash_t));
     if(!ht)
         return NULL;

     ht->number_of_item = 0;
     ht->table = (data_hash_t **) malloc(size * sizeof(data_hash_t *));
     if(!ht->table)
         return NULL;

    ht->size = size;
    for(i=0; i<ht->size; i++)
        ht->table[i] = NULL;
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

    ptr = ht->table[key_hash];
    while(ptr != NULL){
        if(ht->hash_key_compare(ptr->key, key))
            return ptr;
        ptr = ptr->next;
    }

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
 data_hash_t* hash_insert( hash_t* ht, char* key, char* data){
     if(!ht || !key || !data)
        return NULL;

    unsigned int key_hash;

    key_hash = (* ht->hash_function)(key) % ht->size;
fprintf(stdout, "key = %s and hash_key = %d\n", key, key_hash);

    // check if the data is already present in the table
    for(data_hash_t *ptr=ht->table[key_hash]; ptr != NULL; ptr=ptr->next)
        if(ht->hash_key_compare(ptr->key, key))
            return NULL;

    // if data was not found, add at start
    data_hash_t* new_item = (data_hash_t *) malloc(sizeof(data_hash_t));
    if(!new_item)
        return NULL;

    new_item->key = key;
    new_item->data = data;
    new_item->len = strlen(data);
    new_item->size = sizeof(data);
    new_item->next = ht->table[key_hash];

    ht->table[key_hash] = new_item;
fprintf(stdout, "insert data : key=%s and data=%s\n", ht->table[key_hash]->key, ht->table[key_hash]->data);
    ht->number_of_item++;

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
 data_hash_t* hash_update_insert( hash_t* ht, char* key, char* data ){
     if(!ht || !data)
        return NULL;

     data_hash_t *curr, *prev;
     data_hash_t *old_data = NULL;
     unsigned int key_hash;

     key_hash = (* ht->hash_function)(key) % ht->size;

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

    data_hash_t *new_data = (data_hash_t *) malloc(sizeof(data_hash_t));
    new_data->key = key;
    new_data->data = data;
    new_item->len = strlen(data);
    new_item->size = sizeof(data);
    new_data->next = ht->table[key_hash];

    return old_data;
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
    while( curr!=NULL ){
        if(ht->hash_key_compare(curr->key, key)){
            if(prev == NULL)
                ht->table[key_hash] = curr->next;
            else
                prev->next = curr->next;

            ht->number_of_item--;
            return curr;
            curr = NULL;
        }

        prev=curr;
        curr=curr->next;
    }

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
    while( curr!=NULL ){
        if(ht->hash_key_compare(curr->key, key)){
            if(prev == NULL){
                ht->table[key_hash] = curr->next;
            }else{
                prev->next = curr->next;
            }

            if(curr->key) free(curr->key);
            if(curr->data) free(curr->data);
            free(curr);
            ht->number_of_item--;
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }

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
             if(curr->key)
                free(curr->key);
             if(curr->data)
                free(curr->data);
             free(curr);
             curr = next;
         }
     }

     free(ht->table);
     free(ht);

     return 0;
 }
