
OBJ = $(patsubst %.c,%.o,$(wildcard *.c))
CC = aarch64-linux-gnu-gcc

buzzer_test-io	: $(OBJ)
	$(CC) -o $@ $^ 

.PHONY	: clean
clean 	:
	rm -rf *.o buzzer_test-io