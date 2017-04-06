CC= arm-none-linux-gnueabi-gcc
C_FLAGS= -pthread --static


all: main

main: main.o util.o
	$(CC) $(C_FLAGS) -o main main.o util.o


main.o: main.c error_check.h device.h environment.h define.h fpga_dot_font.h util.o
	$(CC) $(C_FLAGS) -c main.c

util.o: util.c util.h
	$(CC) $(C_FLAGS) -c util.c 

.PHONY: clean
clean:
	@rm -rf main main.o util.o
	@echo "Cleaned."
