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
* @file command_handler.h
*
* definition of utility functions for client
*
* @author adrien koumgang tegantchouang
* @version 1.0
* @date 00/05/2021
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>


#include "command_handler.h"
#include "utils.h"

/*
#define number_of_cmds 11

typedef struct _cmd{
    int command;
    long intArg;
    long countArgs;
    char** list_of_arguments;
    struct _cmd* next;
} cmd;
*/

static cmd* listCmds = NULL;
static cmd* lastCmd = NULL;
static cmd* ptrCmd = NULL;

static int cmd_help = 0;
static int cmd_print = 0;
static int cmd_fconnect = 0;
static char arg_cmdf[STR_LEN];

static int firstD = 0;
static char dirnameD[STR_LEN];

static int firstd = 0;
static char dirnamed[STR_LEN];

static int hasError = 0;
static char* msgError[number_of_errors];


/**
* function that allows me to extract client commands from a string
*
* @param buffer_input : string containing all client commands
* @param buffer_output : array of strings containing commands
*                            and their arguments
*
* @returns : the number of strings written to the output buffer on success
*           -1 to error
*/
static int parse_arguments( char* buffer_input, char** list_output, int dim, char* separator ){

    if(!buffer_input) return -1;

    int i = 0;
    char *tmp;
    char *token = strtok_r(buffer_input, separator, &tmp);

    while(token && i<dim){
        int len = strlen(token);
        list_output[i] = (char *) malloc((len + 1) * sizeof(char) );
        memset(list_output[i], '\0', sizeof(char));

        strncpy(list_output[i], token, len+1);
        token = strtok_r(NULL, separator, &tmp);
        i++;
    }

    return i;
}

/**
*
*/
static void addCmd( int tcmd, int intA, char** list, int n ){

    // I add it to the command list
    cmd* new_a = (cmd *) malloc(sizeof(cmd));
    new_a->command = tcmd;
    new_a->intArg = intA;
    new_a->countArgs = n;
    new_a->list_of_arguments = list;
    new_a->next = NULL;
    if(lastCmd == NULL){
        listCmds = new_a;
        lastCmd = listCmds;
    }else{
        lastCmd->next = new_a;
        lastCmd = new_a;
    }

}

int hasHCmd( void ){
    return cmd_help;
}
int hasFCmd( void ){
    return cmd_fconnect;
}
int hasPCmd( void ){
    return cmd_print;
}

char* getF( void ){
    return arg_cmdf;
}

cmd* nextCmd( void ){
    if(!ptrCmd) return NULL;
    cmd* next_cmd = ptrCmd;
    ptrCmd = ptrCmd->next;
    return next_cmd;
}

void backToBegin( void ){
    ptrCmd = listCmds;
}

/**
* in case the client has written the command '-D' after that of '-w' or of '-W',
* it is possible to have access to the parameter of '-D' through this function
*
* @params : the pathname of the first directory indicated by the command '-D' if any
*           NULL if not
*/
char* getFirstDdir( void ){
    if(dirnameD[0] == '\0') return NULL;

    char* d = (char *) malloc(STR_LEN* sizeof(char));
    memset(d, '\0', STR_LEN);
    strncpy(d, dirnameD, STR_LEN);
    return d;
}

/**
* in case the client has written the command '-d' after that of '-r' or of '-R',
* it is possible to have access to the parameter of '-d' through this function
*
* @params : the pathname of the first directory indicated by the command '-d' if any
*           NULL if not
*/
char* getFirstddir( void ){
    if(dirnamed[0] == '\0') return NULL;

    char* d = (char *) malloc(STR_LEN* sizeof(char));
    memset(d, '\0', STR_LEN);
    strncpy(d, dirnamed, STR_LEN);
    return d;
}

/**
*
*/
int initCmds( int argc, char** argv ){
    if((argc < 2) || (argv == NULL)) return -1;

    memset(dirnameD, '\0', STR_LEN);
    memset(dirnamed, '\0', STR_LEN);

    int i=0;

    for(i=0; i<14; i++) msgError[i] = NULL;

    char str_args[STR_LEN];
    int flag_D = 0, flag_w_W = 0;
    int flag_d = 0, flag_r_R = 0;

    i=1;
    while(i<argc){
        int type_of_argument = -1;
        if(strncmp(argv[i], "-h", 2) == 0) type_of_argument = cmd_h;
        else if(strncmp(argv[i], "-w", 2) == 0) type_of_argument = cmd_w;
        else if(strncmp(argv[i], "-W", 2) == 0) type_of_argument = cmd_W;
        else if(strncmp(argv[i], "-D", 2) == 0) type_of_argument = cmd_D;
        else if(strncmp(argv[i], "-r", 2) == 0) type_of_argument = cmd_r;
        else if(strncmp(argv[i], "-R", 2) == 0) type_of_argument = cmd_R;
        else if(strncmp(argv[i], "-d", 2) == 0) type_of_argument = cmd_d;
        else if(strncmp(argv[i], "-t", 2) == 0) type_of_argument = cmd_tt;
        else if(strncmp(argv[i], "-l", 2) == 0) type_of_argument = cmd_l;
        else if(strncmp(argv[i], "-u", 2) == 0) type_of_argument = cmd_u;
        else if(strncmp(argv[i], "-c", 2) == 0) type_of_argument = cmd_c;
        else if(strncmp(argv[i], "-p", 2) == 0) type_of_argument = cmd_p;
        else if(strncmp(argv[i], "-f", 2) == 0) type_of_argument = cmd_f;

        switch(type_of_argument){
            case cmd_h:{
                cmd_help = 1;
                break;
            }
            /**********  comande f   **************/
            case cmd_f:{
                if(!cmd_fconnect){
                    cmd_fconnect = 1;
                    memset(arg_cmdf, '\0', STR_LEN);
                    if(argv[i][2] != '\0'){ // if the server address is attached to the command '-f'
                        strncpy(arg_cmdf, (argv[i] + 2), STR_LEN);
                    }else{
                        if((i+1 < argc) && (strncmp(argv[i+1], "-", 1) != 0)){ // if the server address is contained in the following argument, the command '-f'
                            i++;
                            strncpy(arg_cmdf, argv[i], STR_LEN);
                        }else{
                            msgError[cmd_f] = (char *) malloc(STR_LEN * sizeof(char));
                            hasError = 1;
                            memset(msgError[cmd_f], '\0', STR_LEN);
                            strncpy(msgError[cmd_f], "FATAL ERROR: problem with the server address (socket name) to connect to.\n", STR_LEN);
                        }
                    }
                }else{
                    if(!msgError[cmd_f]) msgError[cmd_f] = (char *) malloc(STR_LEN * sizeof(char));
                    hasError = 1;
                    memset(msgError[cmd_f], '\0', STR_LEN);
                    strncpy(msgError[cmd_f], "FATAL ERROR: the '-f' command must be used only once.\n", STR_LEN);
                }
                break;
            }
            /**********  comande p   **************/
            case cmd_p:{
                if(!cmd_print) cmd_print = 1;
                else{
                    if(!msgError[cmd_p]){
                        hasError = 1;
                        if(!msgError[cmd_p]) msgError[cmd_p] = (char *) malloc(STR_LEN * sizeof(char));
                        memset(msgError[cmd_p], '\0', STR_LEN);
                        strncpy(msgError[cmd_p], "FATAL ERROR: the '-p' command must be used only once.\n", STR_LEN);
                    }
                }
                break;
            }
            /**********  comande t   **************/
            case cmd_tt:{
                long time_to_tt = 0;
                memset(str_args, '\0', STR_LEN);
                if(argv[i][2] != '\0'){
                    strncpy(str_args, (argv[i] + 2), STR_LEN);
                    time_to_tt = getNumber(str_args, 10);
                    addCmd(cmd_tt, time_to_tt, NULL, 0);
                }else{
                    if((i+1 < argc) && (strncmp(argv[i+1], "-", 1) != 0)){
                        i++;
                        strncpy(str_args, argv[i], STR_LEN);
                        time_to_tt = getNumber(str_args, 10);
                        addCmd(cmd_tt, time_to_tt, NULL, 0);
                    }else{
                        addCmd(cmd_tt, 0, NULL, 0);
                    }
                }
                break;
            }
            /**********  comande D   **************/
            case cmd_D:{
                flag_D = 1;
                if(argv[i][2] != '\0'){ // if the dirame is attached to the command '-D'
                    strncpy(str_args, (argv[i] + 2), STR_LEN);
                    goto point_cmd_D;
                }else{
                    i++;
                    if((i < argc) && strncmp(argv[i], "-", 1) != 0){ // if the dirname is contained in the following argument, the command '-D'
                        strncpy(str_args, argv[i], STR_LEN);
                        point_cmd_D:
                            if(firstD){
                                strncpy(dirnameD, str_args, STR_LEN);
                                firstD = 0;
                            }
                            // I add it to the command list
                            char** new_a = (char **) malloc(1 * sizeof(char*));
                            int len = strlen(str_args) + 1;
                            new_a[0]= (char *) malloc(len * sizeof(char));
                            strncpy(new_a[0], str_args, len);
                            addCmd(cmd_D, -1, new_a, 1);
                    }else{
                        hasError = 1;
                        if(!msgError[cmd_D]) msgError[cmd_D] = (char *) malloc(STR_LEN * sizeof(char));
                        memset(msgError[cmd_D], '\0', STR_LEN);
                        strncpy(msgError[cmd_D], "FATAL ERROR: the '-D' command takes an argument. try more again\n", STR_LEN);
                    }
                }
                break;
            }
            /**********  comande w   **************/
            case cmd_w:{
                memset(str_args, '\0', STR_LEN);
                if(argv[i][2] != '\0'){
                    strncpy(str_args, (argv[i] + 2), STR_LEN);
                    goto while_cmd_w;
                }else{
                    if((i+1 < argc) && (strncmp(argv[i+1], "-", 1) != 0)){
                        i++;
                        int j=0;
                        strncpy(str_args, argv[i], STR_LEN);
                        while_cmd_w:
                            while((str_args[j] != '\0') && (str_args[j] != ',')) j++;
                            if(str_args[j] == '\0'){
                                char** new_a = (char **) malloc(1 * sizeof(char*));
                                int len = strlen(str_args) + 1;
                                new_a[0]= (char *) malloc(len * sizeof(char));
                                strncpy(new_a[0], str_args, len);
                                addCmd(cmd_w, 0, new_a, 1);
                            }else{
                                char** new_a = (char **) malloc(2 * sizeof(char *));
                                parse_arguments(str_args, new_a, 2, ",");
                                long n_w = 0;
                                n_w = getNumber(new_a[1], 10);
                                free(new_a[1]);
                                new_a[1] = NULL;
                                addCmd(cmd_w, n_w, new_a, 2);
                            }
                    }else{
                        hasError = 1;
                        if(!msgError[cmd_w]) msgError[cmd_w] = (char *) malloc(STR_LEN * sizeof(char));
                        memset(msgError[cmd_w], '\0', STR_LEN);
                        strncpy(msgError[cmd_w], "FATAL ERROR: the '-w' command takes an argument. try more again\n", STR_LEN);
                    }
                }
                break;
            }
            /**********  comande W   **************/
            case cmd_W:{
                flag_w_W = 1;
                memset(str_args, '\0', STR_LEN);
                int count_a = 1;
                if(argv[i][2] != '\0'){
                    strncpy(str_args, (argv[i] + 2), STR_LEN);
                    goto while_cmd_W;
                }else{
                    if((i+1 < argc) && (strncmp(argv[i+1], "-", 1) != 0)){
                        i++;
                        int j=0;
                        strncpy(str_args, argv[i], STR_LEN);
                        while_cmd_W:
                            while(str_args[j] != '\0'){
                                if(str_args[j] == ',') count_a++;
                                j++;
                            }
                            char** new_a = (char **) malloc(count_a * sizeof(char *));
                            parse_arguments(str_args, new_a, count_a, ",");
                            addCmd(cmd_W, -1, new_a, count_a);
                    }else{
                        hasError = 1;
                        if(!msgError[cmd_W]) msgError[cmd_W] = (char *) malloc(STR_LEN * sizeof(char));
                        memset(msgError[cmd_W], '\0', STR_LEN);
                        strncpy(msgError[cmd_W], "FATAL ERROR: the '-W' command takes an argument. try more again\n", STR_LEN);
                    }
                }
                break;
            }
            /**********  comande r   **************/
            case cmd_r:{
                memset(str_args, '\0', STR_LEN);
                int count_a = 1;
                if(argv[i][2] != '\0'){
                    strncpy(str_args, (argv[i] + 2), STR_LEN);
                    goto while_cmd_r;
                }else{
                    if((i+1 < argc) && (strncmp(argv[i+1], "-", 1) != 0)){
                        i++;
                        int j=0;
                        strncpy(str_args, argv[i], STR_LEN);
                        while_cmd_r:
                            while(str_args[j] != '\0'){
                                if(str_args[j] == ',') count_a++;
                                j++;
                            }
                            char** new_a = (char **) malloc(count_a * sizeof(char *));
                            parse_arguments(str_args, new_a, count_a, ",");
                            addCmd(cmd_r, -1, new_a, count_a);
                    }else{
                        hasError = 1;
                        if(!msgError[cmd_r]) msgError[cmd_r] = (char *) malloc(STR_LEN * sizeof(char));
                        memset(msgError[cmd_r], '\0', STR_LEN);
                        strncpy(msgError[cmd_r], "FATAL ERROR: the '-r' command takes an argument. try more again\n", STR_LEN);
                    }
                }
                break;
            }
            /**********  comande R   **************/
            case cmd_R:{
                long n_to_R = 0;
                memset(str_args, '\0', STR_LEN);
                if(argv[i][2] != '\0'){
                    strncpy(str_args, (argv[i] + 2), STR_LEN);
                    n_to_R = getNumber(str_args, 10);
                    addCmd(cmd_R, n_to_R, NULL, 0);
                }else{
                    if((i+1 < argc) && (strncmp(argv[i+1], "-", 1) != 0)){
                        i++;
                        strncpy(str_args, argv[i], STR_LEN);
                        n_to_R = getNumber(str_args, 10);
                        addCmd(cmd_R, n_to_R, NULL, 0);
                    }else{
                        addCmd(cmd_R, 0, NULL, 0);
                    }
                }
                break;
            }
            /**********  comande d   **************/
            case cmd_d:{
                flag_d = 1;
                if(argv[i][2] != '\0'){ // if the dirame is attached to the command '-D'
                    strncpy(str_args, (argv[i] + 2), STR_LEN);
                    goto point_cmd_d;
                }else{
                    i++;
                    if((i < argc) && strncmp(argv[i], "-", 1) != 0){ // if the dirname is contained in the following argument, the command '-D'
                        strncpy(str_args, argv[i], STR_LEN);
                        point_cmd_d:
                            if(firstd){
                                strncpy(dirnamed, str_args, STR_LEN);
                                firstd = 0;
                            }
                            // I add it to the command list
                            char** new_a = (char **) malloc(1 * sizeof(char*));
                            int len = strlen(str_args) + 1;
                            new_a[0]= (char *) malloc(len * sizeof(char));
                            strncpy(new_a[0], str_args, len);
                            addCmd(cmd_d, -1, new_a, 1);
                    }else{
                        hasError = 1;
                        if(!msgError[cmd_d]) msgError[cmd_d] = (char *) malloc(STR_LEN * sizeof(char));
                        memset(msgError[cmd_d], '\0', STR_LEN);
                        strncpy(msgError[cmd_d], "FATAL ERROR: the '-D' command takes an argument. try more again\n", STR_LEN);
                    }
                }
                break;
            }
            /**********  comande l   **************/
            case cmd_l:{
                memset(str_args, '\0', STR_LEN);
                int count_a = 1;
                if(argv[i][2] != '\0'){
                    strncpy(str_args, (argv[i] + 2), STR_LEN);
                    goto while_cmd_l;
                }else{
                    if((i+1 < argc) && (strncmp(argv[i+1], "-", 1) != 0)){
                        i++;
                        int j=0;
                        strncpy(str_args, argv[i], STR_LEN);
                        while_cmd_l:
                            while(str_args[j] != '\0'){
                                if(str_args[j] == ',') count_a++;
                                j++;
                            }
                            char** new_a = (char **) malloc(count_a * sizeof(char *));
                            parse_arguments(str_args, new_a, count_a, ",");
                            addCmd(cmd_l, -1, new_a, count_a);
                    }else{
                        hasError = 1;
                        if(!msgError[cmd_l]) msgError[cmd_l] = (char *) malloc(STR_LEN * sizeof(char));
                        memset(msgError[cmd_l], '\0', STR_LEN);
                        strncpy(msgError[cmd_l], "FATAL ERROR: the '-l' command takes an argument. try more again\n", STR_LEN);
                    }
                }
                break;
            }
            /**********  comande u   **************/
            case cmd_u:{
                memset(str_args, '\0', STR_LEN);
                int count_a = 1;
                if(argv[i][2] != '\0'){
                    strncpy(str_args, (argv[i] + 2), STR_LEN);
                    goto while_cmd_u;
                }else{
                    if((i+1 < argc) && (strncmp(argv[i+1], "-", 1) != 0)){
                        i++;
                        int j=0;
                        strncpy(str_args, argv[i], STR_LEN);
                        while_cmd_u:
                            while(str_args[j] != '\0'){
                                if(str_args[j] == ',') count_a++;
                                j++;
                            }
                            char** new_a = (char **) malloc(count_a * sizeof(char *));
                            parse_arguments(str_args, new_a, count_a, ",");
                            addCmd(cmd_u, -1, new_a, count_a);
                    }else{
                        hasError = 1;
                        if(!msgError[cmd_u]) msgError[cmd_u] = (char *) malloc(STR_LEN * sizeof(char));
                        memset(msgError[cmd_u], '\0', STR_LEN);
                        strncpy(msgError[cmd_u], "FATAL ERROR: the '-u' command takes an argument. try more again\n", STR_LEN);
                    }
                }
                break;
            }
            /**********  comande c   **************/
            case cmd_c:{
                memset(str_args, '\0', STR_LEN);
                int count_a = 1;
                if(argv[i][2] != '\0'){
                    strncpy(str_args, (argv[i] + 2), STR_LEN);
                    goto while_cmd_c;
                }else{
                    if((i+1 < argc) && (strncmp(argv[i+1], "-", 1) != 0)){
                        i++;
                        int j=0;
                        strncpy(str_args, argv[i], STR_LEN);
                        while_cmd_c:
                            while(str_args[j] != '\0'){
                                if(str_args[j] == ',') count_a++;
                                j++;
                            }
                            char** new_a = (char **) malloc(count_a * sizeof(char *));
                            parse_arguments(str_args, new_a, count_a, ",");
                            addCmd(cmd_l, -1, new_a, count_a);
                    }else{
                        hasError = 1;
                        if(!msgError[cmd_c]) msgError[cmd_d] = (char *) malloc(STR_LEN * sizeof(char));
                        memset(msgError[cmd_c], '\0', STR_LEN);
                        strncpy(msgError[cmd_c], "FATAL ERROR: the '-c' command takes an argument. try more again\n", STR_LEN);
                    }
                }
                break;
            }
            default:{
                hasError = 1;
                if(!msgError[number_of_errors-1]) msgError[number_of_errors-1] = (char *) malloc(STR_LEN * sizeof(char));
                memset(msgError[number_of_errors-1], '\0', STR_LEN);
                strncpy(msgError[number_of_errors-1], "FATAL ERROR: command '", STR_LEN);
                strncat(msgError[number_of_errors-1], argv[i], STR_LEN);
                strncat(msgError[number_of_errors-1], "' not recognized\n", STR_LEN);
            }
        }
        i++;
    }

    if(flag_D && !flag_w_W){
        hasError = 1;
        if(!msgError[cmd_D]) msgError[cmd_D] = (char *) malloc(STR_LEN * sizeof(char));
        memset(msgError[cmd_D], '\0', STR_LEN);
        strncpy(msgError[cmd_D], "FATAL ERROR: the '-D' argument requires to be used with the '-w' or '-W' argument\n", STR_LEN);
    }

    if(flag_d && !flag_r_R){
        hasError = 1;
        if(!msgError[cmd_d]) msgError[cmd_d] = (char *) malloc(STR_LEN * sizeof(char));
        memset(msgError[cmd_d], '\0', STR_LEN);
        strncpy(msgError[cmd_d], "FATAL ERROR: the '-d' argument requires to be used with the '-r' or '-R' argument\n", STR_LEN);
    }

    ptrCmd = listCmds;

    if(hasError) return EXIT_FAILURE;
    else return EXIT_SUCCESS;

}

void finish( void ){
    if(listCmds != NULL){
        cmd* aux = NULL;
        while(listCmds != NULL){
            aux = listCmds;
            listCmds = listCmds->next;
            free(aux);
        }
    }

    int i=0;
    for(i=0; i<number_of_errors; i++){
        if(msgError[i]) free(msgError[i]);
    }
}

char** getErrors( void ){
    return msgError;
}
