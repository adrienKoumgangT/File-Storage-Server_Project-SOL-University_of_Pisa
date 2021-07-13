#!/bin/bash

cmd="valgrind --leak-check=full ../main/bin/client -f ./mysock1 -p -t 200"
f1="./files/bd_files/bd1/file1.txt"
f2="./files/bd_files/bd1/file2.txt"
d1="./files/bd_files/bd1/"
d2="./files/bd_files/output/"

#stampa il messaggio di 'help'
$cmd -h

#chiede la scrittura nel server un file
$cmd -W $f1

#chiede la scrittura del file scritto in precedenza nel server
$cmd -d . -r $f1

#chiede la lettura di un file non esistente nel server
$cmd -d . -r $f2

#chiede la scrittura nel server di 3 files contenuti in una cartella
$cmd -w ./files/bd_files/bd1/,n=3

#chiede la lettura di tutti i file presente nel server
$cmd -d $d2 -R n=0

#chiede di lasciare la mutua esclusione di un file di cui non si ha la mutua esclusione
$cmd -u $f1

#chiede di rimuovere dal server un file di cui non si ha la mutua esclusione
$cmd -c $f1
