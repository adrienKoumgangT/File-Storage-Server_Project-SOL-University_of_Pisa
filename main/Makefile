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


TARGETS = $(BIN)server \
			$(BIN)client


.PHONY: all clean cleanall
.SUFFIXES: .c .h

all: $(TARGETS)

$(BIN)server: $(OBJ)server.o $(OBJ)queue.o $(OBJ)my_hash.o $(OBJ)my_file.o
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $^ $(LIBS)

$(BIN)client: $(OBJ)client.o  $(OBJ)interface.o $(OBJ)command_handler.o
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $^

$(OBJ)server.o: $(SRC)server.c $(INC)utils.h $(INC)my_file.h $(INC)my_hash.h $(INC)queue.h $(INC)communication.h
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $< $(LIBS)

$(OBJ)client.o: $(SRC)client.c $(INC)interface.h $(INC)utils.h $(INC)command_handler.h
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

$(OBJ)interface.o: $(SRC)interface.c $(INC)interface.h $(INC)communication.h $(INC)utils.h
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

$(OBJ)command_handler.o: $(SRC)command_handler.c $(INC)command_handler.h
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

$(OBJ)replace_policies.o: $(SRC)replace_policies.c $(INC)replace_policies.h $(INC)utils.h
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

$(OBJ)queue.o: $(SRC)queue.c $(INC)queue.h $(INC)utils.h
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

$(OBJ)my_hash.o: $(SRC)my_hash.c $(INC)my_hash.h $(INC)my_file.h
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $< $(LIBS)

$(OBJ)my_file.o: $(SRC)my_file.c $(INC)my_file.h $(INC)my_hash.h
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

clean:
	rm -f $(BIN)client $(server)

cleanall: clean
	\rm -f *.o *~
