#################################################################################
# MIT License																	#
#																				#
# Copyright (c) 2021 Adrien Koumgang Tegantchouang								#
#																				#
# Permission is hereby granted, free of charge, to any person obtaining a copy	#
# of this software and associated documentation files (the "Software"), to deal	#
# in the Software without restriction, including without limitation the rights	#
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell		#
# copies of the Software, and to permit persons to whom the Software is			#
# furnished to do so, subject to the following conditions:						#
#																				#
# The above copyright notice and this permission notice shall be included in all#
# copies or substantial portions of the Software.								#
#																				#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR	#
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,		#
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE	#
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER		#
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,	#
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE	#
# SOFTWARE.																		#
#################################################################################

#############################################
# @file Makefile							#
#											#
# definition of make file for test			#
#											#
# @author adrien koumgang tegantchouang		#
# @version 1.0								#
# @date 00/05/2021							#
#############################################

CC 			= gcc
CFLAGS 		= -Wall -Werror -pedantic
INCLUDES 	= -I . -I ./include -I ../main/include
LDFLAGS 	= -L
OPTFLAGS	= -g
LIBS 		= -lpthread

SRC = src/
OBJ = objs/
BIN = bin/
INC = include/


TARGETS = test


.PHONY: all clean cleanall
.SUFFIXES: .c .h


$(BIN)client: $(OBJ)main_server.o $(OBJ)client.o $(OBJ)interface.o $(OBJ)my_file.o $(OBJS)my_hash.o
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $^

$(OBJ)test.o: $(SRC)test.c $(INC)test_hash.h
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

$(OBJ)my_hash.o: $(SRCMAIN)my_hash.c $(INCMAIN)my_hash.h $(INCMAIN)my_file.h
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

$(OBJ)my_file.o: $(SRCMAIN)my_file.c $(INCMAIN)my_file.h $(INCMAIN)my_hash.h
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

clean:
	rm -f $(BIN)test

cleanall: clean
	\rm -f *.o *~
