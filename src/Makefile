#-----------------------------------------------------------------------
# File: Makefile
#
# $ make 				// same as $make lib
# $ make lib        // compiles every C_source_file into diferent execs
#
# Author: Sergio Romero Montiel
#
# Created on November 16, 2018
#-----------------------------------------------------------------------

#-----------------------FILES-------------------------------------------
# Lista de fuentes
INCLIB  := ../include
SOURCES := ${wildcard *.c}
HEADERS := ${wildcard *.h, ${INCLIB}/*.h}
# Lista de objetos que forman la lib
OBJECTS := ${SOURCES:.c=.o}
# Libreria del simulador de misiles
LIB := libmotfifo.a
LIBDIR := ../lib
#-----------------------TOOLS-------------------------------------------
# Compiladores y Enlazadores (no modificar, usamos los por defecto)
#CC =
# Opciones para el compilador
CFLAGS = -Wall -I${INCLIB}
# ----------------------RULES-------------------------------------------
# Targets y sufijos
.PHONY: all lib clean vclean
#regla para hacer la libreria
all: $(LIBDIR)/$(LIB)
lib: $(LIB)
$(LIBDIR)/$(LIB): $(LIB)
	cp $< $(LIBDIR)
$(LIB): $(OBJECTS)
	ar -rc $@ $^
$(OBJECTS): $(HEADERS) Makefile
clean:
	-rm -fv $(OBJECTS) $(LIB)
vclean:
	-rm -fv $(OBJECTS) $(LIB) $(LIBDIR)/$(LIB)
#-----------------------------------------------------------------------
