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


#ifndef _CMD_HANDLER_
#define _CMD_HANDLER_

#define cmd_w (0)
#define cmd_W (1)
#define cmd_D (3)
#define cmd_r (4)
#define cmd_R (5)
#define cmd_d (6)
#define cmd_tt (7)
#define cmd_l (8)
#define cmd_u (9)
#define cmd_c (10)

#define cmd_p (11)
#define cmd_f (12)
#define cmd_h (13)

#define number_of_options 14
#define number_of_cmds 11
#define number_of_errors 15

typedef struct _cmd{
    int command;
    long intArg;
    long countArgs;
    char** list_of_arguments;
    struct _cmd* next;
} cmd;



int hasHCmd( void );
int hasFCmd( void );
int hasPCmd( void );

char* getFirstDdir( void );
char* getFirstddir( void );
char* getF( void );
cmd* nextCmd( void );
char** getErrors( void );

int initCmds( int, char** );
void finish( void );

#endif
