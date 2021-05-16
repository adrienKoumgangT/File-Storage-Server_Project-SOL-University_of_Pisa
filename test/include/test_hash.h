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
 * @file test_hash.c
 *
 * Test of type hash_t : hash table
 *
 * @author adrien koumgang tegantchouang
 * @version 1.0
 * @date 00/05/2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my_file.h"
#include "my_hash.h"

#define init_cnt 0

#define size_table 100
#define n_elements_insert 15
#define n_elements_update_insert 3
#define n_elements_remove 7

#define size_str 80

unsigned long tempo = 0;

void read_data(data_hash_t* data){
    if(!data){
        fprintf(stdout, "dato NULL\n");
        return;
    }

    fprintf(stdout, "chiave: %s --- content: %s\n",
                        data->key, data->data);
}

void read_list(data_hash_t* list){
    if(!list){
        fprintf(stdout, "lista vuota\n");
        return;
    }

    data_hash_t* next = list;
    do{
        fprintf(stdout, "chiave: %s --- content: %s ::: ",
                            next->key, next->data);
        next = next->next;
    }while(next != NULL);

    fprintf(stdout, "end list\n");
}

void read_table(hash_t* ht){
    if(!ht){
        fprintf(stdout, "tabella vuota\n");
        return;
    }

    for(int i=0; i<ht->size; i++){
        fprintf(stdout, "list n.%d = ", i);
        read_list(ht->table[i]);
    }
}

int avoid_test_hash(void){
    int i=0, index=0;
    unsigned int seme1 = 0, seme2 = 1, seme3 = 2;



    fprintf(stdout, "%lu : dichiarazione della tabella hash\n", tempo++);
    hash_t *ht = NULL;
    fprintf(stdout, "%lu : dichiarazione della tabella hash -> success\n",
                        tempo++);

    fprintf(stdout, "%lu : creazione della tabella hash\n", tempo++);
    ht = hash_create(size_table, &hash_function_for_file_t,
                        &hash_key_compare_for_file_t);
    fprintf(stdout, "dimensione della tabella hash creata: %ld\n",
                    sizeof(ht));
    fprintf(stdout, "%lu : creazione della tabella hash -> success\n",
                        tempo++);

    fprintf(stdout, "%lu : inserzione di %d elementi nella tabella\n",
                        tempo++, n_elements_insert);
    for(i=init_cnt; i<n_elements_insert+init_cnt; i++){
        char *str_key = (char *) malloc(size_str * sizeof(char));
        char *str_data = (char *) malloc(size_str * sizeof(char));
        memset(str_key, '\0', size_str);
        memset(str_data, '\0', size_str);
        index = rand_r(&seme2) % n_elements_insert;
        sprintf(str_key, "key n. %d", i+index);
        sprintf(str_data, "content n. %d", i);
        hash_insert(ht, str_key, str_data);
    }
    fprintf(stdout, "%lu : inserzione di %d elementi nella tabella -> success\n",
                        tempo++, n_elements_insert);

    fprintf(stdout, "%lu : lettura della tabella\n", tempo++);
    read_table(ht);
    fprintf(stdout, "%lu : lettura della tabella -> success\n", tempo++);

    fprintf(stdout, "%lu : update di %d elementi della tabella\n",
                        tempo++, n_elements_update_insert);
    for(i=0; i<n_elements_update_insert; i++){
        char *str_key = (char *) malloc(size_str * sizeof(char));
        char *str_data = (char *) malloc(size_str * sizeof(char));
        index = rand_r(&seme1) % n_elements_insert;
        memset(str_key, '\0', size_str);
        memset(str_data, '\0', size_str);
        sprintf(str_key, "key n. %d", index);
        sprintf(str_data, "content n. %d", index+n_elements_insert);
        data_hash_t *ptr =hash_update_insert(ht, str_key, str_data);
        fprintf(stdout, "dato estratto: ");
        read_data(ptr);
        if(ptr){
            if(!ptr->key) free(ptr->key);
            if(!ptr->data) free(ptr->data);
            free(ptr);
        }
    }
    fprintf(stdout, "%lu : update di %d elementi della tabella -> success\n",
                    tempo++, n_elements_update_insert);

    fprintf(stdout, "%lu : lettura della tabella\n", tempo++);
    read_table(ht);
    fprintf(stdout, "%lu : lettura della tabella -> success\n", tempo++);

    fprintf(stdout, "%lu : rimossione di certi elementi\n", tempo++);
    for(i=0; i<n_elements_update_insert; i++){
        char *str_key = (char *) malloc(size_str * sizeof(char));
        index = rand_r(&seme3) % n_elements_insert;
        memset(str_key, '\0', size_str);
        data_hash_t *ptr = NULL;
        ptr = hash_remove(ht, str_key);
        /* if(!hash_delete(ht, str_key))
            fprintf(stdout, "dato di chiave %s tolto con successo\n", str_key);
        else
            fprintf(stdout, "dato di chiave %s absente\n", str_key); */
        fprintf(stdout, "dato rimosso: ");
        read_data(ptr);
        if(ptr){
            if(ptr->key) free(ptr->key);
            if(ptr->data) free(ptr->data);
            free(ptr);
        }
    }
    fprintf(stdout, "%lu : rimossione di certi elementi -> success\n", tempo++);

    fprintf(stdout, "%lu : lettura della tabella\n", tempo++);
    read_table(ht);
    fprintf(stdout, "%lu : lettura della tabella -> success\n", tempo++);

    fprintf(stdout, "%lu : cancellazione della tabella\n", tempo++);
    hash_destroy(ht);
    fprintf(stdout, "%lu : cancellazione della tabella -> success\n", tempo++);

    return EXIT_SUCCESS;
}
