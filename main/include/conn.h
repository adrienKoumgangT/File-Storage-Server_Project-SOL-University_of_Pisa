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
* @file utils.h
*
* definition of utility functions
*
* @author adrien koumgang tegantchouang
* @version 1.0
* @date 00/05/2021
*/


#ifndef CONN_H
#define CONN_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "my_file.h"

/**
* 
*/
readn_r( int fd, file_t f ){
    int err;

    switch(operation){
        case _OF_M:{
            break;
        }
        case _RF_M:{
            break;
        }
        case _RNF_M:{
            break;
        }
        case _ATF_M:{
            break;
        }
        case _LF_M:{
            break;
        }
        case _UF_M:{
            break;
        }
        case _CF_M:{
            break;
        }
        case _RF_M:{
            break;
        }
        default:
            return -1;
    }
}

read_a(){

}

write_r( int fd, file_t f,  int operation){
    int err;

    if(!(operation == _OF_M) || !(operation == _RF_M) ||
                                !(operation == _RFN_M) ||
                                !(operation == _ATF_M) ||
                                !(operation == _LF_M) ||
                                !(operation == _UF_M) ||
                                !(operation == _CF_M) ||
                                !(operation == _RF_M)
        ) return -1;

    if((err = write(fd, operation, sizeof(operation))) == -1){
        if(errno != EINTR) return -1;
    }

    switch(operation){
        case _OF_M:{
            if((err = write(fd, f->key, sizeof(f->key))) == -1){
                if(errno == EINTR) continue;
                return -1;
            }

            if((err = write(fd, f->size, sizeof(f->size))) == -1){
                if(errno == EINTR) continue;
                return -1;
            }

            if((err = write(fd, f->len, sizeof(f->len))) == -1){
                if(errno == EINTR) continue;
                return -1;
            }

            for(int i=0; i<f->len; i++){
                size_t left = sizeof(f->data[i]);
                char* bufptr = f->data[i];
                while(left>0){
                    if((err = write(fd, bufptr, left)) == -1){
                        if(errno == EINTR) continue;
                        return -1;
                    }
                    if(err == 0) break;
                    left -= r;
                }
            }
            break;
        }
        case _RF_M:{
            break;
        }
        case _RNF_M:{
            break;
        }
        case _ATF_M:{
            break;
        }
        case _LF_M:{
            break;
        }
        case _UF_M:{
            break;
        }
        case _CF_M:{
            break;
        }
        case _RF_M:{
            break;
        }
        default:
            return -1;
    }



    return 1;
}

write_a(){

}

#endif
