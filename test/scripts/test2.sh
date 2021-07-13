#!/bin/bash

cmd="../main/bin/client -f ./mysock2 -p -t 200"
ft1="./files/bd_files/bd1/file1.txt,./files/bd_files/bd/file1.pdf"
ft2="./files/bd_files/bd/file4.pdf,./files/bd_files/bd/file5.pdf"
ft3="./files/bd_files/bd/file6.pdf,./files/bd_files/bd/file7.pdf,./files/bd_files/bd/file8.pdf"
d1="./files/bd_files/bd3/"
d2="./files/bd_files/output"

$cmd -W $ft1 -D $d2

$cmd -W $ft2 -D $d2

$cmd -W $ft3 -D $d2

$cmd -w $d1
