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
     id(!ht->table)
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
 void* hash_find( hash_t* ht, char* key ){
     if(!ht || !key)
        return NULL;

    data_hash_t *ptr;
    unsigned int key_hash;

    key_hash = (* ht->hash_function)(key) % ht->size;

    ptr = ht->table[key_hash];
    while(ptr != NULL){
        if(ht->hash_key_compare(ptr->key, key))
            return ptr->data;
        ptr = ptr->next;
    }

    return NULL;
 }

/**
* Insert new data into the hash table
*
* @param ht : the hash table
* @param data : pointer to the new item's data
*
* @returns : - NULL if the data is already present in the table
*            -  data otherwise
*
* @exceptions : if one of the given parameters is NULL it returns NULL
*/
 data_hash_t* hash_insert( hash_t* ht, data_hash_t* data){
     if(!ht || !data)
        return NULL;

    unsigned int key_hash;

    key_hash = (* ht->hash_function)(dat->key) % ht->size;

    // check if the data is already present in the table
    for(data_hash_t *ptr=ht->table[key_hash]; ptr != NULL; ptr=ptr->next)
        if(ht->hash_key_compare(ptr->key, key))
            return NULL;

    // if data was not found, add at start
    data->next = ht->table[key_hash];
    ht->table[key_hash] = data;
    ht->number_of_item++;

    return data;
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
 data_hash_t* hash_update_insert( hash_t* ht, data_hash_t* data ){
     if(!ht || !data)
        return NULL;

    data_hash_t *old_data = NULL;
     data_hash_t *curr, *prev;
     unsigned int key_hash;

     key_hash = (* ht->hash_function)(data->key) % ht->size;

     // I look for the value to replace
     for(prev=NULL, curr=ht->table[key_hash]; curr!=NULL; prev=curr, curr=curr->next)
        if(ht->hash_key_compare(curr->key, data->key)){
            data->next = curr->next;

            if(prev == NULL) // if the item searched for at the top of the list
                ht->table[key_hash] = data;
            else
                prev->next = data;

            old_data = curr;
            curr = NULL;
        }

    // if there was no element, I put the data at the top of the list
    if(old_data == NULL){
        data->next = ht->table[key_hash];
        ht->table[key_hash] = data;
    }

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

    data_hash_t *old_data = NULL;
    data_hash_t *curr, *prev;
    unsigned int key_hash;

    key_hash = (* ht->hash_function)(key) % ht->size;

    // I look for the element with key key
    for(prev=NULL, curr=ht->table[key_hash]; curr!=NULL; prev=curr, curr=curr->next)
        if(ht->hash_key_compare(curr->key, key)){
            if(prev == NULL)
                ht->table[key_hash];
            else
                prev->next = curr->next;

            old_data = curr;
            curr = NULL;
        }

    return old_data;
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
             free(curr->key);
             if(curr->data) free(curr->data);
             curr = next;
         }
     }

     free(ht->table);
     free(ht);

     return 0;
 }
