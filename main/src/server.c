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

#include "server.h"
#include "utils.h"


// define for all programs
#define STR_LEN 80
#define STR_SIZE (STR_LEN * sizeof(char))



// server variables
struct _config_for_server configurations_server;
struct _hash_t *files_server;

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
*              0 if is good configuration server
*/
int is_correct_cfs(){
    cfs *config = &configurations_server;

    if(!config)
        exit(EXIT_FAILURE);

    if(config->thread_workers <= 1)
        return -1;

    if(config->size_memory <= 0)
        return -1;

    if(config->socket_name == NULL)
        return -1;

    return 0;
}

/**
* function that allows you to read the server configurations
*/
void read_config_server(){
    cfs *config = &configurations_server;

    fprintf(stdout, "information about the current server configuration :\n");
    fprintf(stdout, "number of threads workers = %ld\n",
                        config->thread_workers);
    fprintf(stdout, "size of memory for server = %ld\n",
                        config->size_memory);
    fprintf(stdout, "name to use for the socket : %s",
                        config->socket_name);
}


/**
* function that allows you to set a precise configuration
* of the server starting from a given string
*
* @param str : string containing the configuration
*
* @returns : 0 to success
*               -1 to failure
**/
int parsing_str_for_config_server( char* str ){
    cfs *config = &configurations_server;

    if(!str || !config)
        return -1;

    char *tmp;
    char *token = strtok_r(str, ":", &tmp);

    while(token){
        if(strncmp(token, t_w, sizeof(t_w)) == 0){
            token = strtok_r(NULL, ":", &tmp);
            token[strcspn(token, "\n")] = '\0';
fprintf(stdout, "token with t_w = %s\n", token);
fflush(stdout);
            if(!getNumber(token, &(config->thread_workers)))
                return -1;
        }else if(strncmp(token, s_m, sizeof(s_m)) == 0){
            token = strtok_r(NULL, ":", &tmp);
            token[strcspn(token, "\n")] = '\0';
fprintf(stdout, "token with t_w = %s\n", token);
fflush(stdout);
            if(!getNumber(token, &(config->size_memory)))
                return -1;
        }else if(strncmp(token, s_m, sizeof(t_w)) == 0){
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
    NULL_EXIT(buffer, (char *) malloc(STR_SIZE),
                                "malloc failure");

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

        MINUS_ONE_EXIT(err, parsing_str_for_config_server(buffer),
                                "config file");
    }

    read_config_server();

    if(is_correct_cfs() != 0){
        fprintf(stderr, "Failure to configure server\n");
        exit(EXIT_FAILURE);
    }

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

    int err;
    FILE *file_config;

    NULL_EXIT(file_config, fopen(path_file_config, "r"), "fopen");
    MINUS_ONE_EXIT(err, config_file_parser_for_server(file_config),
                            "configuration server");
    VALUE_EXIT(EOF, err, fclose(file_config), "fclose");

    read_config_server();

    return 0;
}

/**
* main function of the server that starts the server
*/
void run_server( char* path_file_config ){

    int err;

    MINUS_ONE_EXIT(err, config_server(path_file_config), "server configuration");

}
