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
* @file server.c
*
* definition of utility functions for server
*
* @author adrien koumgang tegantchouang
* @version 1.0
* @date 00/05/2021
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/uio.h>
#include <sys/un.h>

#include <assert.h>

#include "communication.h"
#include "utils.h"
#include "my_hash.h"
#include "my_file.h"
#include "queue.h"

// define for all programs
#define STR_LEN 80
#define STR_SIZE (STR_LEN * sizeof(char))
#define DIM_HASH_TABLE 103

// define for config server
#define n_param_config 5
#define t_w "THREAD_WORKERS"
#define s_m "SIZE_MEMORY"
#define n_f "NUMBER_OF_FILES"
#define s_n "SOCKET_NAME"
#define c_c "CONCURRENT_CLIENTS"

static int info = 1;
static unsigned long tempo_dgb = 1;

static int close_server = 0;

/**
* Definition of the structure that will have the server configuration data
*/
typedef struct _config_for_server{
    unsigned long   thread_workers;
    unsigned long   concurrent_clients;
    unsigned long   size_memory;
    unsigned long   number_of_files;
    char*           socket_name;
}cfs;

/**************************** definition of a structure that takes information
    about the files opened on the server on the server and
    the client that opened it ***********************************************/

typedef struct _fi{
    char*           file;
    struct _fi*     next;
}fi;

typedef struct _info_file{
    int     fd_client;
    fi*     files;
}info_file;

static info_file** info_files = NULL;

/*************** server variables *****************/

// variable containing server configurations
static cfs settings_server = {0, 0, 0, 0, NULL};

// file containers
static hash_t *files_server;

// request buffer
static Queue_t* buffer_request;


/*********** structure for counting elements in mutual exclusion **********/

typedef struct _count_elem{
    long                count;
    pthread_mutex_t     lock;
    pthread_cond_t      cond;
} count_elem_t;


/************************* number of threads created *************************/
static count_elem_t* threads_created;

/************************* number of client conneced *************************/
static count_elem_t* clients_connected;

/******************* pipe for communication Workers -> Master ****************/

static int canale[2];

/********* cleanup function ****/

void cleanup( void ){
    unlink(settings_server.socket_name);
}

/*********** function to initialised the structure for counting elements in mutual exclusion **********/

count_elem_t* init_struct_count_elem( void ){
    int err;
    count_elem_t* t;
    SYSCALL_RETURN_EQ("malloc", t, (count_elem_t *) malloc(sizeof(count_elem_t)), NULL, "");
    t->count = 0;
    SYSCALL_RETURN_VAL_NEQ("pthread_mutex_init", err, pthread_mutex_init(&t->lock, NULL), 0, NULL, "");
    SYSCALL_RETURN_VAL_NEQ("pthread_cond_init", err, pthread_cond_init(&t->cond, NULL), 0, NULL, "");
    return t;
}

/*** definition of the management functions of the hash table 'info_files' ***/

int init_hash_info_files( void ){
    info_files = (info_file **) malloc(settings_server.concurrent_clients * sizeof(info_file *));
    if(!info_files) return -1;
    return 0;
}

int add_info_file( int fd_client, char* file ){
    int pos = fd_client % settings_server.concurrent_clients;
    if(info_files[pos] == NULL){
        info_file* new_info = (info_file *) malloc(sizeof(info_file));
        if(!new_info) exit(EXIT_FAILURE);
        new_info->fd_client = fd_client;
        new_info->files = (fi *) malloc(sizeof(fi));
        if(!new_info->files) exit(EXIT_FAILURE);
        new_info->files->file = file;
        new_info->files->next = NULL;
        info_files[pos] = new_info;
    }else{
        info_file* old_info = info_files[pos];
        if(old_info->fd_client != fd_client) return -1;
        if(old_info->files){
            fi* curr = old_info->files;
            while((strcmp(curr->file, file) != 0) && (curr->next == NULL))
                curr = curr->next;
            if(curr->next == NULL){
                curr->next = (fi *) malloc(sizeof(fi));
                if(!curr->next) exit(EXIT_FAILURE);
                curr->next->file = file;
                curr->next->next = NULL;
            }
        }else{
            old_info->files = (fi *) malloc(sizeof(fi));
            old_info->files->file = file;
            old_info->files->next = NULL;
        }
    }
    return 0;
}

int find_info_file( int fd_client, char* file ){
    int pos = fd_client % settings_server.concurrent_clients;
    if(info_files[pos] == NULL)
        return -1;
    if(info_files[pos]->fd_client != fd_client)
        return -2;
    if(info_files[pos]->files == NULL)
        return 0;
    fi* curr = info_files[pos]->files;
    while((curr != NULL) && (strcmp(curr->file, file) != 0))
        curr = curr->next;
    if(curr == NULL) return 0;
    return 1;
}

int remove_info_file( int fd_client, char* file ){
    int pos = fd_client % settings_server.concurrent_clients;
    if(info_files[pos] == NULL)
        return -1;
    if(info_files[pos]->fd_client != fd_client)
        return -2;
    if(info_files[pos]->files == NULL)
        return -3;
    fi* prev = NULL;
    fi* curr = info_files[pos]->files;
    while((curr != NULL) && (strcmp(curr->file, file) != 0)){
        prev = curr;
        curr = curr->next;
    }
    if(curr == NULL) return -3;
    if(prev == NULL)
        info_files[pos]->files->next = curr->next;
    else
        prev->next = curr->next;
    free(curr);
    return 0;
}

void destroy_info_files( void ){
    if(info_files){
        for(int i=0; i<settings_server.concurrent_clients; i++)
            if(info_files[i]){
                fi* c = info_files[i]->files;
                fi* n = c;
                while(n != NULL){
                    n = c->next;
                    free(c);
                    c = n;
                }
                free(info_files[i]);
            }
        free(info_files);
    }
}

/*************** definition of server configuration functions ***************/


/**
* reinitializes the server configurations
*/
void reset_cfs(){
    cfs *config = &settings_server;

    if(!config)
        exit(EXIT_FAILURE);

    config->thread_workers = 0;
    config->concurrent_clients = 0;
    config->size_memory = 0;
    if(config->socket_name)
        free(config->socket_name);
    config->socket_name = NULL;

}


/**
* check if the current server configurations
* are well done for it to work
*
* @returns : -1 if server has bad configuration
*              0 if is good configuration server
*/
int is_correct_cfs(){
    cfs *config = &settings_server;

    if(!config)
        exit(EXIT_FAILURE);

    if(config->thread_workers <= 1)
        return -1;

    if(config->concurrent_clients <= 1)
        return -1;

    if(config->size_memory <= 0)
        return -1;

    if(config->number_of_files <= 0)
        return -1;

    if(config->socket_name == NULL)
        return -1;

    return 0;
}


/**
* function that allows you to read the server configurations
*/
void read_config_server(){

if(info) fprintf(stdout, "=>%ld<= : reading server configuration in progress...\n", tempo_dgb++);

    cfs *config = &settings_server;

    fprintf(stdout, "information about the current server configuration :\n");
    fprintf(stdout, "number of threads workers = %ld\n",
                        config->thread_workers);
    fprintf(stdout, "number of concurrent clients = %ld\n",
                        config->concurrent_clients);
    fprintf(stdout, "size of memory for server = %ld\n",
                        config->size_memory);
    fprintf(stdout, "max number of files to write on server = %ld\n",
                        config->number_of_files);
    fprintf(stdout, "name to use for the socket : %s\n",
                        config->socket_name);
    fflush(stdout);

if(info) fprintf(stdout, "=>%ld<= : finished server configuration reading.\n", tempo_dgb++);

}


/**
* function that allows you to set a precise configuration
* of the server starting from a given string
*
* @param str : string containing the configuration
*
* @returns : 0 to success
*            -1 to failure
**/
int parsing_str_for_config_server( char* str ){

    cfs *config = &settings_server;

    if(!str || !config)
        return -1;

    char *tmp;
    char *token = strtok_r(str, ":", &tmp);

    while(token){

        if(strncmp(token, t_w, sizeof(t_w)) == 0){
            token = strtok_r(NULL, ":", &tmp);
            token[strcspn(token, "\n")] = '\0';

            if( (config->thread_workers = (unsigned long) getNumber(token, 10)) < 0)
                return -1;

        }else if(strncmp(token, c_c, sizeof(c_c)) == 0){
            token = strtok_r(NULL, ":", &tmp);
            token[strcspn(token, "\n")] = '\0';

            if( (config->concurrent_clients = (unsigned long) getNumber(token, 10)) < 0)
                return -1;

        }else if(strncmp(token, s_m, sizeof(s_m)) == 0){
            token = strtok_r(NULL, ":", &tmp);
            token[strcspn(token, "\n")] = '\0';

            if( (config->size_memory = (unsigned long) getNumber(token, 10)) < 0)
                return -1;

        }else if(strncmp(token, n_f, sizeof(n_f)) == 0){
            token = strtok_r(NULL, ":", &tmp);
            token[strcspn(token, "\n")] = '\0';

            if( (config->number_of_files = (unsigned long) getNumber(token, 10)) < 0)
                return -1;

        }else if(strncmp(token, s_n, sizeof(s_n)) == 0){
            token = strtok_r(NULL, ":", &tmp);
            token[strcspn(token, "\n")] = '\0';

            int n = strlen(token);
            // if the string is empty
            if(n <= 1)
                return -1;
            if(!config->socket_name)
                config->socket_name = (char *) malloc((n+1) * sizeof(char));
            // if the allocation has failed
            if(!config->socket_name)
                return -1;
            strncpy(config->socket_name, token, n+1);
        }

        token = strtok_r(NULL, ":", &tmp);
    }

    return 0;
}


/**
* Function that allows you to take the basic configurations of
* a server from a file
*
* @param file : the file where the initialization data will be taken
*
* @returns : - 0 on success
*            - -1 on error
*/
int config_file_parser_for_server( FILE* file ){

    int err;

    if(!file)
        return -1;

    char *buffer;
    SYSCALL_EXIT_EQ("malloc", buffer, (char *) malloc(STR_SIZE), NULL, "malloc failure");

    reset_cfs();

    int i=0;
    for(i=0; i<n_param_config; i++){
        memset(buffer, '\0', STR_SIZE);

        if(fgets(buffer, STR_LEN, file) == NULL){
            if(feof(file) != 0)
                break;
            else
                return EXIT_FAILURE;
        }

        SYSCALL_RETURN_EQ("parsing_str_for_config_server", err,
                                parsing_str_for_config_server(buffer), -1,
                                "config file error");

    }

    read_config_server();

    SYSCALL_RETURN_EQ("is_correct_cfs", err, is_correct_cfs(), -1, "Failure to configure server");

    return 0;

}


/**
* function that allows you to configure a server from a file
*
* @param path_file_config : path leading to the configuration file
*
* @return : 0 to success
*/
int config_server( char* path_file_config ){

if(info) fprintf(stdout, "=>%ld<= : reading server configurations from configuration file in progress...\n", tempo_dgb++);

    int err;
    FILE *file_config;

    SYSCALL_EXIT_EQ("fopen", file_config, fopen(path_file_config, "r"), NULL, "fopen");
    SYSCALL_EXIT_EQ("config_file_parser_for_server", err, config_file_parser_for_server(file_config),
                            -1, "configuration server");
    SYSCALL_EXIT_EQ("fclose", err, fclose(file_config), -1, "fclose");

    read_config_server();

fprintf(stdout, "=>%ld<= : reading server configurations from finished configuration file.\n", tempo_dgb++);

    return 0;
}

/****************************** thread workers *******************************/

void* workers( void* args ){
    int operation, err;
    int toClose = 0;
    while(!close_server){
        toClose = 0;
        long fd_client_r = (long) pop(buffer_request);

        // I read the type of request made by the client
        if(read(fd_client_r, &operation, sizeof(int)) == -1){
            toClose = 1;
            goto fine_while;
        }

        char* pathname = NULL;

        switch(operation){
            case _OF_O:{ // if it's an 'open file' request
                // I read the request for 'open file'
                if(read_request_OF(fd_client_r, pathname) == -1){
                    toClose = 1;
                    goto fine_while;
                }

                // TODO: trattare la richiesta
                int resp = 0;

                // I write the result of the request for 'open file'
                if(write_response_OF(fd_client_r, resp) == -1){
                    toClose = 1;
                    goto fine_while;
                }
                break;
            }
            default:{
                toClose = 1;
                goto fine_while;
            }
        }



        // TODO: gestire le diverse richieste

        fine_while:
            SYSCALL_EXIT_EQ("write", err, write(canale[0], &fd_client_r, sizeof(long)), -1, "");
            SYSCALL_EXIT_EQ("write", err, write(canale[0], &toClose, sizeof(int)), -1, "");
    }

    return NULL;
}

/******************************* thread master *******************************/

/**
* master: function of the server that starts the server
*/
void master( void ){
    int i, err;

if(info) fprintf(stdout, "=>%ld<= : creating a file descriptor container.\n", tempo_dgb++);

    fd_set set, tmpset;
    FD_ZERO(&set);
    FD_ZERO(&tmpset);

if(info) fprintf(stdout, "=>%ld<= : creation and configuration of the server communication channel with clients in progress...\n", tempo_dgb++);

    struct sockaddr_un server_addr;
    memset(&server_addr, '0', sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, settings_server.socket_name, strlen(settings_server.socket_name)+1);

    int fd_socket = -1;
    SYSCALL_EXIT_EQ("socket", fd_socket, socket(AF_UNIX, SOCK_STREAM, 0), -1, "");
    printf("ciao mondo!!!\n");
    SYSCALL_EXIT_EQ("bind", err, bind(fd_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)), -1, "");
    printf("salut le monde!!!\n");
    SYSCALL_EXIT_EQ("listen", err, listen(fd_socket, settings_server.concurrent_clients), -1, "");

    FD_SET(fd_socket, &set);

if(info) fprintf(stdout, "=>%ld<= : finished creation and configuration of the server communication channel with clients.\n",tempo_dgb++);

if(info) fprintf(stdout, "=>%ld<= : configuration of the methods of creation and functioning of threads in progress...\n", tempo_dgb++);

    SYSCALL_EXIT_EQ("pipe", err, pipe(canale), -1, "");
    FD_SET(canale[0], &set);

    pthread_attr_t thread_attr;
    pthread_t thread_id;

    SYSCALL_EXIT_NEQ("pthread_attr_init", err, pthread_attr_init(&thread_attr), 0, "");
    SYSCALL_EXIT_NEQ("pthread_attr_setdetachstate", err, pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED), 0, "");

    /*
    thread_id = (pthread_t *) malloc(settings_server.thread_workers * sizeof(pthread_t));
    for(i=0; i<settings_server.thread_workers; i++)
        thread_id[i] = -1;
    */

    SYSCALL_EXIT_EQ("init_struct_count_elem", threads_created, init_struct_count_elem(), NULL, "");

if(info) fprintf(stdout, "=>%ld<= : configuration of the methods of creation and functioning of the finished threads.\n", tempo_dgb++);

if(info) fprintf(stdout, "=>%ld<= : beginning of acceptance of requests.\n", tempo_dgb++);

    SYSCALL_EXIT_EQ("init_struct_count_elem", clients_connected, init_struct_count_elem(), NULL, "");

    SYSCALL_EXIT_EQ("hash_create", files_server, hash_create( DIM_HASH_TABLE, &hash_function_for_file_t, &hash_key_compare_for_file_t ) , NULL, "")

    SYSCALL_EXIT_EQ("initQueue", buffer_request, initQueue(), NULL, "");

    SYSCALL_EXIT_EQ("init_hash_info_files", err, init_hash_info_files(), -1, "");

    int fdmax = canale[0];

    do{
        tmpset = set;
        SYSCALL_EXIT_EQ("select", err, select(fdmax+1, &tmpset, NULL, NULL, NULL), -1, "");

        // let's try to understand from which file descriptor we received a request
        for(i=0; i<= fdmax; i++){
            if(FD_ISSET(i, &tmpset)){
                long connfd = -1;
                // if it is a new connection request
                if(i == fd_socket){
                    // check if I have reached the limit number of clients connected at the same time
                    pthread_mutex_lock(&clients_connected->lock);
                    if(clients_connected->count >= settings_server.concurrent_clients){
                        pthread_mutex_unlock(&clients_connected->lock);
                        continue;
                    }else{
                        int c1 = ++clients_connected->count;
                        pthread_mutex_unlock(&clients_connected->lock);
                        SYSCALL_EXIT_EQ("accept", connfd, accept(fd_socket, (struct sockaddr *)NULL, NULL), -1, "");
                        FD_SET(connfd, &set);
                        if(connfd > fdmax) fdmax = connfd;
                        pthread_mutex_lock(&threads_created->lock);
                        if(threads_created->count < (c1 + 3)){
                            if(pthread_create(&thread_id, &thread_attr, workers, (void *) NULL) != 0){
                                fprintf(stderr, "pthread_create FAILED\n");
                            }
                        }
                        pthread_mutex_unlock(&threads_created->lock);
                        continue;
                    }
                }

                // if it is the worker thread who has finished handling a client request
                if(i == canale[0]){
                    int toClose = 0;
                    SYSCALL_EXIT_EQ("read", err, read(canale[0], &connfd, sizeof(long)), -1, "");
                    SYSCALL_EXIT_EQ("read", err, read(canale[0], &toClose, sizeof(int)), -1, "");
                    if(toClose){
                        FD_SET(connfd, &set);
                        if(connfd > fdmax) fdmax = connfd;
                    }
                    continue;
                }

                // if it is a generic request from a client
                connfd = i;
                // int* fd_client_request;
                FD_CLR(connfd, &set);
                push(buffer_request, (void *) connfd);
            }
        }

    }while(!close_server);

    pthread_mutex_lock(&threads_created->lock);
    while(threads_created->count > 0){
        pthread_cond_wait(&threads_created->cond, &threads_created->lock);
    }
    pthread_mutex_lock(&threads_created->lock);

    close(canale[0]);
    close(canale[1]);

    SYSCALL_EXIT_NEQ("pthread_attr_destroy", err, pthread_attr_destroy(&thread_attr), 0, "");
    free(threads_created);
    free(clients_connected);
    destroy_info_files();
    SYSCALL_EXIT_EQ("hash_destroy", err, hash_destroy(files_server), -1, "");
    //SYSCALL_EXIT_EQ("deleteQueue", err, deleteQueue(buffer_request), void, "");
    deleteQueue(buffer_request);
}

/******************************* main function *******************************/

int main(int argc, char** argv){
//    cleanup();
//    atexit(cleanup);

    if(argc < 2){
        fprintf(stderr, "test file execution scheme: %s pathname_file_config\n", argv[0]);
        return EXIT_FAILURE;
    }

    int err;

    SYSCALL_EXIT_EQ("config_server", err, config_server(argv[1]), -1,
                            "server configuration failure");

if(info) fprintf(stdout, "=>%ld<= : server startup!\n", tempo_dgb++);

    master();

if(info) fprintf(stdout, "=>%ld<= : server closed!\n", tempo_dgb++);
    return 0;
}
