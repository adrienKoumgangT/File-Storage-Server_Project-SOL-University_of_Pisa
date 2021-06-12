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
* @file communication.h
*
* definition of utility functions for read and write by socket
*
* @author adrien koumgang tegantchouang
* @version 1.0
* @date 00/05/2021
*/


#ifndef COMUNICATION_H
#define COMUNICATION_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <time.h>

#include "my_file.h"

// flags that specify the operation requested by the client
#define OF      "OF";
#define RF      "RF";
#define RNF     "RNF";
#define WF      "WF";
#define ATF     "ATF";
#define LF      "LF";
#define UF      "UF";
#define CF      "CF";
#define RF      "RF";

// operation
#define _OF_O       (1)
#define _RF_O       (2)
#define _RNF_O      (3)
#define _WF_O       (4)
#define _ATF_O      (5)
#define _LF_O       (6)
#define _UF_O       (7)
#define _CF_O       (8)
#define _RFI_O      (9)

#define O_CREATE            (1)
#define O_LOCK              (2)
#define O_CREATE_LOCK       (3)

// success or failed operation
#define SUCCESS_O (0)
#define FAILED_O (-1)



/******************************* client  *************************/


/*************************** write request **********************/


/**
* makes an open file request to the server
*/
int write_request_OF( const int, const char*, int );

/**
*
*/
int write_request_RF( const int, const char* );

/**
*
*/
int write_request_RNF( const int, const int );

/**
* function that allows me to tell the server that
* the client wants to write a file in the database
*/
int write_request_WF_ATF( const int, const int, const char*, const char*, size_t );

int write_request_LF_UF_CF_RFI( const int, const int, const char* );


/****************************** read response ************************/

/***/
int read_response_OF( const int, int*, char* );

/***/
int read_response_RF( const int, char**, size_t* );

/***/
int read_response_RNF( const int, int*, char**, size_t*, char**, size_t* );

int read_response_WF_ATF( const int, int*, char*, char*, char*, size_t* );

int read_response_LF_UF_CF_RFI( const int, int*, char* );


/*********************************** server ***************************/


/*************************** read request **********************/

/**
* receives an open file request from the client
*/
int read_request_OF( const int, char* );


/**
*
*/
int read_request_RF( const int, char* );

/**
*
*/
int read_request_RNF( const int, int* );

/**
* function that allows a client to read the data requested
* to write a file in the database
*/
int read_request_WF( const int, file_t* );


int read_request_ATF( const int, char*, char*, size_t* );

int read_request_LF_UF_CF_RFI( const int, char* );


/********************************* write response ***************************/

/***/
int write_response_OF( const int, int, char* );

/***/
int write_response_RF(const int, char*, size_t );

/***/
int write_response_RNF( const int, const int, char**, size_t*, char**, size_t* );

/**
*
*/
int write_response_WF_ATF( const int, const int, const char*, const char*, const char*, const size_t );

/***/
int write_response_LF_UF_CF_RFI( const int, const int, const char* );

#endif
