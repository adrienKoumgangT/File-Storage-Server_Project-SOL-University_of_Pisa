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
 * @file my_file.c
 *
 * Definition of the functions used for the hash table
 *
 * @author adrien koumgang tegantchouang
 * @version 1.0
 * @date 00/05/2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my_file.h"

 // simple hash function
 /**
 *  hash function that computes the hash value given to key
 *
 * @params key : key to find its hash value
 *
 * @returns : -1 if not valid key
 *              hash value it is valid key
 */
 unsigned int hash_function_for_file_t(char* key){
     char *str_key = (char *) key;
     if(!str_key)
        return -1;

    unsigned int key_value = 0;
    for(int i=0; str_key[i] != '\0'; i++)
        key_value += (unsigned int) str_key[i];

    return key_value;
 }

 // compare function
 /**
 *  function that allows you to compare two keys
 *
 * @params first_key : first key to compare
 * @params second_key : second value to compare
 *
 * @returns : 0 if egual
 *             -1, 1 otherwise
 */
 int hash_key_compare_for_file_t(char* first_key, char* second_key){
     return strcmp(first_key , second_key);
 }

/**
* print the file with its information and content
*
*   @params f : file to print the contents of
*/
 void print_file(file_t *f){
     if(f){
         fprintf(stdout, "%s  %ld  %s", f->key, f->size, f->data);
     }
 }
