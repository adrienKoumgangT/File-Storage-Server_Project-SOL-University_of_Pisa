/**
 * @file my_hash.h
 *
 * Definition of type hash_t
 *
 * A generic hash_t has :
 * 		- a name (filename) : a string representing the key of the file
 *		- a content (content) : the contents of the file
 *		- a size of content (size) : the size of the content
 *		- a pointer to another file (next) : to use to create a list of files
 *
 * @author adrien koumgang tegantchouang
 * @version 1.0
 * @date 00/05/2021
 */


 #if !defined(HASH_T)
 #define HASH_T

#include "my_file.h"

 // definition of element to be inserted in the hash table
 /*
 typedef struct _file_t {
   char *key;
   char *data;
   struct _file_t *next;
 } file_t;
 */
 typedf struct _file_t data_hash_t;

 /* the structure of hash table */
 typedef struct _hash_t {
 	int size;
 	int number_of_item;
 	data_hash_t **table;
 	unsigned int (*hash_function)(char *);
 	int (*hash_key_compare)(char *, char *);
 } hash_t;

 /* Create a new hash table */
 hash_t* hash_create( const int, unsigned int (*hash_function)(char *), int (*hash_key_compare)(char *, char *) );


 void* hash_find( hash_t*, char* );


 data_hash_t* hash_insert( hash_t*, data_hash_t* );


 data_hash_t* hash_update_insert( hash_t*, data_hash_t* );


 data_hash_t* hash_remove( hash_t*, char* );


 int hash_destroy( hash_t* );


 #endif
