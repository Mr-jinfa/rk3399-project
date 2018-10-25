
OBJ = $(patsubst %.c,%.o,$(wildcard *.c))
CC = aarch64-linux-gnu-gcc

compass_app	: $(OBJ)
	$(CC) -o $@ $^ 

.PHONY	: clean
clean 	:
	rm -rf *.o compass_app