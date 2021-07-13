#!/bin/bash

cmd="../main/bin/client -f ./mysock3"
ft1="./files/bd_files/bd1/file1.txt"
ft2="./files/bd_files/bd/file1.pdf"
ft3="./files/bd_files/bd/file4.pdf"
ft4="./files/bd_files/bd/file5.pdf"
ft5="./files/bd_files/bd/file6.pdf"
ft6="./files/bd_files/bd/file7.pdf"
ft7="./files/bd_files/bd/file8.pdf"
d1="./files/bd_files/bd1/"
d2="./files/bd_files/bd2/"
d3="./files/bd_files/bd3/"
d4="./files/bd_files/bd/"
d5="./files/bd_files/output"

$cmd -W $ft1 | $cmd -W $ft2 | $cmd -W $ft3 | $cmd -W $ft4 | $cmd -W $ft4

$cmd -d $d5 -r n=0 | $cmd -W $ft5 | $cmd -W $ft6 -D $d2 | $cmd -W $ft4 -D $d2

$cmd -w $d1
