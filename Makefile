CC = gcc
CFLAGS = -g -Wall -Werror -lrt
c_files = $(wildcard *.c)
object_files = $(c_files:.c=.o)
targets = cliente servidor
library = libclaves.so
.PHONY: all clean
all: $(targets)

servidor: servidor.o
	@echo "Compiling $@ ..."
	$(CC)  $< -o $@ 

# enlazar libreria dinamica con objeto clientes
cliente: cliente.o $(library) 
	@echo "Compiling $@ with $^"
	$(CC) -o $@ $< -L. -lclaves

# compilar el .c a un objeto .o
claves.o: claves.c
	@echo "Creating dynamic library object from: $< ..."
	$(CC) -c -Wall -Werror -fPIC $< -o $@

#compilar .o en una libreria dinamica .so
$(library): claves.o
	@echo "Creating dynamic library file from: $< ..."
	$(CC) -shared -o $@ $^

%.o: %.c
	@echo "Creating object file from: $< ..."
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(object_files) $(targets) $(library)
