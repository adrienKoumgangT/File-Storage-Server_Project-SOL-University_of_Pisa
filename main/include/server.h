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


#if !defined(CONFIG_F)
#define CONFIG_F

/**
* Definition of the structure that will have the server configuration data
*/
typedef struct _config_for_server{
    unsigned long thread_workers;
    unsigned long size_memory;
    unsigned long number_of_files;
    char *socket_name;
}cfs;

// define for config server
#define n_param_config 4
#define t_w "THREAD_WORKERS"
#define s_m "SIZE_MEMORY"
#define n_f "NUMBER_OF_FILES"
#define s_n "SOCKET_NAME"


// functions used to configure the server
void reset_cfs();
int is_correct_cfs();
int config_server( char* path_file_config );
int config_file_parser( FILE* file, cfs* config );
int parsing_str( char* str, cfs* config );
void read_config_server();

#endif

#if !defined(_SERVER_)
#define _SERVER_


// functions used for server operation
void run_server( char* path_file_config );


#endif
