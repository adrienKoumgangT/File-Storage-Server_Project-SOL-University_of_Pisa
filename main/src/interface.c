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
* @file interface.h
*
* definition of utility functions for client
*
* @author adrien koumgang tegantchouang
* @version 1.0
* @date 00/05/2021
*/

#define _POSIX_C_SOURCE  200112L  // needed for S_ISSOCK

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>
#include <dirent.h>
#include <time.h>
#include <assert.h>
#include <errno.h>

#include <pwd.h>
#include <grp.h>
#include <stddef.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

//#include "utils.h"
#include "interface.h"

int sock_fd;

/*
EBADF : Descriptore di domanda non valido
ECONNREFUSED : connessione rifiutata
EFAULT : indirizzo sbagliato
EINVAL : argomento non valido
ETIMEDOUT : il tentativo di connessione è scaduto prima che fosse
                stabilita una connessione
*/

/*
La funzione connect() fallirà se:

EADDRNOTAVAIL : L'indirizzo specificato non è disponibile dalla macchina locale.
EAFNOSUPPORT : L'indirizzo specificato non è un indirizzo valido per la famiglia
                di indirizzi del socket specificato.
EALREADY : E' già in corso una richiesta di connessione per il socket specificato
EBADF : L'argomento socket non è un descrittore di file.
ECONNREFUSED : L'indirizzo di destinazione non era in ascolto per le connessioni
                o ha rifiutato la richiesta di connessione.
EINPROGRESS : O_NONBLOCK è impostato per il descrittore di file per il socket
                e la connessione non può essere stabilitata immediatamente;
                la connessione deve essere stabilita in modo asinscrono.
EINTR : Il tentativo di stabilire una connessione è stato interroto dalla
        consegna di un segnale che è stato intercettato;
        la connessione deve essere stabilita in modo asinscrono.
EISCONN : Il socket specificato è in modalità di connessione ed è già connesso.
ENETUNREACH : Non è presente alcun percorso verso la rete.
ENOTSOCK : L'argomento socket non fa riferimento a un socket.
EPROTOTIPO : L'indirizzo specificato ha un tipo diverso rispetto al socket
                associato all'indirizzo per specificato.
ETIMEDOUT : Il tentativo di connessione è scaduto prima che fosse stabilitata
                una connessione.

Se la famiglia di indirizzi del socket è AF_UNIX, connect() fallirà se:
EIO : Si è verificato un errore di I/O durante la lettura nel file system
ELOOP : Esiste un loop nei collegamenti simbolici riscontrati durante
            la risoluzione del percorso in address.
ENAMETOOLONG : Un componente di un percorso ha superato {NAME_MAX} caratteri
                o un intero percorso ha superato {PATH_MAX} caratteri.
ENOENT : Un componente del percorso non nomina un file esistente o
            il percorso è una stringa vuota.
ENOTDIR : Un componente del prefisso del percorso del nome del percorso in
            address non è una directory.

La funzione connect() potrebbe non riuscire se:
EACCES :
EADDRINUSE :
ECONNRESET :
EHOSTUNREACH :
EINVAL :
ELOOP :
ENAMETOOLONG :
ENETDOWN :
ENOBUFS :
EOPNOTSUPP :
*/

/**
* An AF_UNIX connection is opened to the socket file sockname
*
* @param sockname : name of the socket to connect to
* @param msec : time to repeat to retry the connection with the server if it fails
* @param abstime : time until you try to connect to the server
*
* @returns :  0 to success
*            -1 to failure
*/
int openConnection( const char* sockname, int msec, const struct timespec abstime ){

    if(!sockname){
        errno = EINVAL;
        return -1;
    }

    struct sockaddr_un serv_addr;

    if((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        errno = EFAULT;
        return -1;
    }

    while(1){


        memset(&serv_addr, '0', sizeof(serv_addr));
        serv_addr.sun_family = AF_UNIX;
        strncpy(serv_addr.sun_path, sockname, sizeof(sockname)+1);

        int notused = connect(sock_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));

        if(notused == -1){
            if(errno == EADDRNOTAVAIL ||
                errno == EAFNOSUPPORT ||
                errno == EBADF){
                    // errore che richiede l'uscita
            }

            if(errno == EALREADY ||
                errno == ECONNREFUSED ||
                errno == EINPROGRESS){
                    // errore che richiede di aspettare
            }
        }

    }

    errno = ECONNREFUSED;
}

int closeConnection( const char* sockname );

int openFile( const char* pathname, int flags );

int readFile( const char* pathname, void** buf, size_t* size );

int readNFile( int N, const char* dirname );

int appendToFile( const char* pathname, void* buf, size_t size, const char* dirname );

int lockFile( const char* pathname );

int unlockFile( const char* pathname );

int closeFile( const char* pathname );

int removeFile( const char* pathname );
