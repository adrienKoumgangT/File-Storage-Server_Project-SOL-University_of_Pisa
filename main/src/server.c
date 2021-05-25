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
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>

#include <assert.h>

#include "server.h"
#include "utils.h"


// define for all programs
#define STR_LEN 80
#define STR_SIZE (STR_LEN * sizeof(char))

// variabile globale per il dbg
int tempo_dgb = 1;

// server variables
struct _config_for_server configurations_server;
struct _hash_t *files_server;


/* ###############################################
*  #definition of server configuration functions #
*  ###############################################
*/

/**
* reinitializes the server configurations
*/
void reset_cfs(){
    cfs *config = &configurations_server;

    if(!config)
        exit(EXIT_FAILURE);

    config->thread_workers = 0;
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
*              1 if is good configuration server
*/
int is_correct_cfs(){
    cfs *config = &configurations_server;

    if(!config)
        exit(EXIT_FAILURE);

    if(config->thread_workers <= 1)
        return -1;

    if(config->size_memory <= 0)
        return -1;

    if(config->number_of_files <= 0)
        return -1;

    if(config->socket_name == NULL)
        return -1;

    return 1;
}


/**
* function that allows you to read the server configurations
*/
void read_config_server(){

fprintf(stdout, "%d : entro dentro read_config_server\n", tempo_dgb++);

    cfs *config = &configurations_server;

    fprintf(stdout, "information about the current server configuration :\n");
    fprintf(stdout, "number of threads workers = %ld\n",
                        config->thread_workers);
    fprintf(stdout, "size of memory for server = %ld\n",
                        config->size_memory);
    fprintf(stdout, "max number of files to write on server = %ld\n",
                        config->number_of_files);
    fprintf(stdout, "name to use for the socket : %s\n",
                        config->socket_name);
    fflush(stdout);

fprintf(stdout, "%d : esco da read_config_server\n", tempo_dgb++);

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

fprintf(stdout, "%d : entro dentro parsing_str_for_config_server\n",
                                tempo_dgb++);

    cfs *config = &configurations_server;

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
        }else if(strncmp(token, s_m, sizeof(s_m)) == 0){
            token = strtok_r(NULL, ":", &tmp);
            token[strcspn(token, "\n")] = '\0';

            if( (config->size_memory = (unsigned long) getNumber(token, 10)) < 0)
                return -1;
        }else if(strncmp(token, n_f, sizeof(n_f)) == 0){
            token = strtok_r(NULL, ":", &tmp);
            token[strcspn(token, "\n")] = '\0';

            if( (config->number_of_files = (unsigned long) getNumber(token, 10))
                                    < 0)
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

fprintf(stdout, "%d : esco da parsing_str_for_config_server\n", tempo_dgb++);

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

fprintf(stdout, "%d : entro dentro config_file_parser_for_server\n",
                                tempo_dgb++);

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

fprintf(stdout, "%d : esco da config_file_parser_for_server\n", tempo_dgb++);

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

fprintf(stdout, "%d : entro dentro config_server\n", tempo_dgb++);

    int err;
    FILE *file_config;

    SYSCALL_EXIT_EQ("fopen", file_config, fopen(path_file_config, "r"), NULL, "fopen");
    SYSCALL_EXIT_EQ("config_file_parser_for_server", err, config_file_parser_for_server(file_config),
                            -1, "configuration server");
    SYSCALL_EXIT_EQ("fclose", err, fclose(file_config), -1, "fclose");

    read_config_server();

fprintf(stdout, "%d : esco da config_server\n", tempo_dgb++);

    return 0;
}

/**
* main function of the server that starts the server
*/
void run_server( char* path_file_config ){

fprintf(stdout, "%d : entro dentro run_server\n", tempo_dgb++);
    int err;

    SYSCALL_EXIT_EQ("config_server", err, config_server(path_file_config), -1,
                            "server configuration");


fprintf(stdout, "%d : esco da run_server\n", tempo_dgb++);
}
