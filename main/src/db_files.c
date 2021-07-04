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

 #include "db_files.h"
 #include "utils.h"


// for file_t

static inline void lockFile( file_t* ft ){
    LOCK(&ft->flock);
}

static inline void unlockFile( file_t* ft ){
    UNLOCK(&ft->flock);
}

static inline void unlockFileAndWait( file_t* ft ){
    WAIT(&ft->fcond, &ft->flock);
}

static inline void unlockFileAndSignal( file_t* ft ){
    SIGNAL(&ft->fcond);
    UNLOCK(&ft->flock);
}

// for node_hash_t
static inline void lockNodeHash( node_hash_t* nh ){
    LOCK(&nh->nhlock);
}

static inline void unlockNodeHash( node_hash_t* nh ){
    UNLOCK(&nh->nhlock);
}

static inline void unlockNodeHashAndWait( node_hash_t* nh ){
    WAIT(&nh->nhcond, &nh->nhlock);
}

static inline void unlockNodeHashAndSignal( node_hash_t* nh ){
    SIGNAL(&nh->nhcond);
    UNLOCK(&nh->nhlock);
}

// for hash_t
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


/**************** operations performed on the file type *******************/

// simple hash function
 /**
 *  hash function that computes the hash value given to key
 *
 * @params key : key to find its hash value
 *
 * @returns : -1 if not valid key
 *              hash value it is valid key
 */
static unsigned int hash_function_for_file_t(char* key){
    if(!key)
        return -1;

    size_t len = strlen(key)+1;
    if(len <= 1)
        return -1;
    char *str_key = (char *) malloc(len * sizeof(char));
    memset(str_key, '\0', len);
    strncpy(str_key, key, len);

    unsigned int key_value = 0;
    for(int i=0; str_key[i] != '\0'; i++)
        key_value += (unsigned int) str_key[i];
    free(str_key);
    return key_value;
}

/**
* print the file with its information and content
*
*   @params f : file to print the contents of
*/
void file_print(file_t* f){
    if(f){
        fprintf(stdout, "%s  %ld  %s", f->key, f->size_data, (char *)f->data);
    }
}

/**
*
*/
file_t* file_create( char* key, size_t size_key, void* data, size_t size_data, int fd ){
    if(!key) return NULL;

    file_t* new_file = (file_t *) malloc(sizeof(file_t));
    if(!new_file) return NULL;
    new_file->key       = key;
    new_file->size_key  = size_key;
    new_file->data      = data;
    new_file->size_data = size_data;
    new_file->log       = -1;
    new_file->next      = NULL;
    FD_ZERO(&new_file->set);
    FD_SET(fd, &new_file->set);
    pthread_mutex_init(&new_file->flock, NULL);
    pthread_cond_init(&new_file->fcond, NULL);
    return new_file;
}

/**
*
*/
void file_free(file_t* f){
    if(f){
        if(f->key) free(f->key);
        if(f->data) free(f->data);
        pthread_mutex_destroy(&(f->flock));
        pthread_cond_destroy(&(f->fcond));
        free(f);
    }
}

/**
*
*/
static file_t* file_update_data(file_t* ft, void* data, size_t size_data ){
    if(!ft || !data) return NULL;

    file_t* new_file = (file_t *) malloc(sizeof(file_t));
    if(!new_file) return NULL;


    if(ft->data == NULL){
        new_file->data = data;
    }else{
        new_file->data = malloc(ft->size_data + size_data + 1);
        if( sprintf(new_file->data, "%s%s", (char *)ft->data, (char *)data) < 0) return NULL;
        free(data);
    }

    new_file->key       = ft->key;
    new_file->size_key  = ft->size_key;
    new_file->size_data = size_data;
    new_file->set       = ft->set;
    new_file->log       = ft->log;
    new_file->next      = ft->next;
    pthread_mutex_init(&new_file->flock, NULL);
    pthread_cond_init(&new_file->fcond, NULL);

    return new_file;
}

/**
*
*/
int file_take_lock( file_t* ft, int fd_lock ){
    if(fd_lock <= 0) return -1;

    lockFile(ft);
    if(ft->log >= 0) unlockFileAndWait(ft);
    ft->log = fd_lock;
    FD_SET(fd_lock, &ft->set);
    unlockFile(ft);
    return 0;
}

/**
*
*/
int file_leave_lock( file_t* ft, int fd_lock ){
    if(fd_lock <= 0) return -1;

    lockFile(ft);
    if(ft->log != fd_lock){
        unlockFileAndSignal(ft);
        return -2;
    }
    ft->log = -1;
    unlockFileAndSignal(ft);
    return 0;
}

/**
*
*/
int file_has_lock( file_t* ft, int fd ){
    int r = 0;
    lockFile(ft);
    if(ft->log == fd) r = 1;
    unlockFile(ft);
    return r;
}

/**
*
*/
int file_read_content( file_t * ft, void* content, size_t* size_content ){
    lockFile(ft);
    if(content) free(content);
    content =  malloc(ft->size_data+1);
    //memset(content, '\0', ft->size_data+1);
    //strncpy(content, (char *)ft->data, ft->size_data);
    memset(content, '0', ft->size_data);
    if( sprintf(content, "%s", (char *)ft->data) < 0) return -1;
    *size_content = ft->size_data;
    unlockFileAndSignal(ft);
    return 0;
}

/**
*
*/
int file_write_content( file_t* ft, void* content, size_t size_content ){
    if(!content || size_content <= 0) return -1;

    lockFile(ft);
    if(ft->data) free(ft->data);
    ft->data = malloc(size_content+1);
    //memset(ft->data, '\0', size_content+1);
    //strncpy(ft->data, content, size_content);
    memset(ft->data, '0', size_content+1);
    ft->size_data = size_content;
    if( sprintf(ft->data, "%s", (char *)content) < 0) return -1;
    unlockFileAndSignal(ft);
    return 0;
}

/**
*
*/
int file_append_content( file_t* ft, void* content, size_t size_content ){
    if(!content || size_content <= 0) return -1;

    lockFile(ft);
    void* p = ft->data;
    ft->data = malloc(ft->size_data + size_content+1);
    //strncat(ft->data, content, size_content);
    if( sprintf(ft->data, "%s%s", (char *)p, (char *)content) < 0) return -1;
    ft->size_data += size_content;
    free(p);
    unlockFileAndSignal(ft);
    return 0;
}

/**
*
*/
void file_add_fd( file_t* ft, int fd ){
    lockFile(ft);
    FD_SET(fd, &(ft->set));
    unlockFile(ft);
}

/**
*
*/
void file_remove_fd( file_t* ft, int fd ){
    lockFile(ft);
    FD_CLR(fd, &(ft->set));
    unlockFile(ft);
}

/**
*
*/
int file_has_fd( file_t* ft, int fd ){
    int r = 0;
    lockFile(ft);
    if(FD_ISSET(fd, &(ft->set))) r = 1;
    unlockFile(ft);
    return r;
}


/*************** operations performed on the hash table *******************/


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
hash_t* hash_create( const int size ){
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
    ht->table = (node_hash_t **) malloc(size * sizeof( node_hash_t* ));
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
        ht->table[i] = (node_hash_t *) malloc(sizeof(node_hash_t));
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
file_t* hash_find( const hash_t* ht, char* key ){
    if(!ht || !key)
        return NULL;

    node_hash_t *ptr_n;
    file_t* ptr;
    unsigned int key_hash;

    key_hash = hash_function_for_file_t(key) % ht->size;

    ptr_n = ht->table[key_hash];
    lockNodeHash(ptr_n);
    if(ptr_n->n <= 0){
        unlockNodeHash(ptr_n);
        return NULL;
    }
    ptr = ptr_n->list;
    while(ptr != NULL){
fprintf(stdout, "db_files.c -> hash_find: key = %s and ptr->key = %s\n", key, ptr->key);
        if(strcmp(ptr->key, key) == 0){
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
file_t* hash_insert( hash_t* ht, char* key, size_t size_key, void* data, size_t size_data, int fd ){
      if(!ht || !key)
         return NULL;

     unsigned int key_hash;

     key_hash = hash_function_for_file_t(key) % ht->size;
fprintf(stdout, "db_files.c -> hash_insert: key = %s and hash_key = %d\n", key, key_hash);

    node_hash_t* ptr_n = ht->table[key_hash];
    lockNodeHash(ptr_n);
     // check if the data is already present in the table
     file_t *ptr = NULL;
     if(ptr_n->n > 0){
         ptr = ptr_n->list;
         while( ptr != NULL ){
fprintf(stdout, "db_files.c -> hash_insert: key = %s and ptr->key = %s\n", key, ptr->key);
             if(strcmp(ptr->key, key) == 0){
                 unlockNodeHash(ptr_n);
                 return NULL;
             }
             ptr=ptr->next;
         }
    }

     // if data was not found, add at start
    file_t* new_item = file_create(key, size_key, data, size_data, fd);
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
file_t* hash_update_insert( hash_t* ht, char* key, size_t size_key, void* data, size_t size_data, int fd ){
      if(!ht || !data)
         return NULL;

      file_t *curr = NULL, *prev = NULL;
      file_t *old_data = NULL;
      unsigned int key_hash;

      key_hash = hash_function_for_file_t(key) % ht->size;

      node_hash_t* ptr_n = ht->table[key_hash];
      lockNodeHash(ptr_n);
      // I look for the value to replace
      for(prev=NULL, curr=ptr_n->list; curr!=NULL; prev=curr, curr=curr->next){
fprintf(stdout, "db_files.c -> hash_update_insert: key = %s and ptr->key = %s\n", key, curr->key);
         if(strcmp(curr->key, key) == 0){

             if(prev == NULL) // if the item searched for at the top of the list
                 ptr_n->list = curr->next;
             else
                 prev->next = curr->next;

             old_data = curr;
             curr = NULL;
         }
     }

    file_t* new_item = NULL;
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
file_t* hash_update_insert_append( const hash_t* ht, char* key, size_t size_key, void* data, size_t size_data ){
    if(!ht || !key || !data)
        return NULL;

     file_t *ptr = NULL;
     unsigned int key_hash;

     key_hash = hash_function_for_file_t(key) % ht->size;

     node_hash_t* ptr_n = ht->table[key_hash];
     lockNodeHash(ptr_n);
     ptr = ptr_n->list;
     while(ptr != NULL){
fprintf(stdout, "db_files.c -> hash_update_insert_append: key = %s and ptr->key = %s\n", key, ptr->key);
        if(strcmp(ptr->key, key) == 0){
            unlockNodeHash(ptr_n);
            file_append_content(ptr, data, size_data);
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
file_t* hash_remove( hash_t* ht, char* key ){
     if(!ht || !key)
        return NULL;

    file_t *curr, *prev;
    unsigned int key_hash;

    key_hash = hash_function_for_file_t(key) % ht->size;

    if(ht->table[key_hash] == NULL)
         return NULL;

    // I look for the element with key key
    node_hash_t* ptr_n = ht->table[key_hash];
    lockNodeHash(ptr_n);
    prev=NULL;
    curr=ptr_n->list;
    while( curr!=NULL ){
fprintf(stdout, "db_files.c -> hash_remove: key = %s and curr->key = %s\n", key, curr->key);
        if(strcmp(curr->key, key) == 0){
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

    file_t* aux = hash_remove(ht, key);
    if(!aux) return -1;
    file_free(aux);

    return 1;
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

    node_hash_t* ptr_n = NULL;
    file_t *ptr_list, *curr, *next;

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
