
OBJ = $(patsubst %.c,%.o,$(wildcard *.c))
CC = aarch64-linux-gnu-gcc

adc_app	: $(OBJ)
	$(CC) -o $@ $^ 

.PHONY	: clean
clean 	:
	rm -rf *.o buzzer_test-io