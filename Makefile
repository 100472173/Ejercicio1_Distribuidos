CC = gcc
CFLAGS = -g -Wall -Werror
c_files = $(wildcard *.c)
c_file_names = $(pathsubst %.c, %, $(c_files))
object_files = $(c_files:.c=.o)
.PHONY: all clean
all: cliente 

servidor: servidor.o
	@echo "Compiling $@ ..."
	$(CC) structures.o $< -o $@ 

# enlazar libreria dinamica con objeto clientes
cliente: lib_claves.so cliente.o
	@echo "Compiling $@ ..."
	$(CC) -o $@ $< -L. -l clave

# compilar el .c a un objeto .o
claves.o: claves.c
	@echo "Creating dynamic library object from: $< ..."
	$(CC) -c $(CFLAGS) -fPIC $< -o $@

# compilar .o en una libreria dinamica .so
lib_claves.so: claves.o
	@echo "Creating dynamic library file from: $< ..."
	$(CC) -shared $< -o  $@



%.o: %.c
	@echo "Creating object file from: $< ..."
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(object_files) lib_claves.so