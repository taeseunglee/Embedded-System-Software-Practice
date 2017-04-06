BUILD_DIR = $(SRC_DIR)/build
OBJS_DIR = $(BUILD_DIR)/objs
LIB_DIR = $(SRC_DIR)/../lib
MAIN_PROG = 20141570
CC= arm-none-linux-gnueabi-gcc
C_FLAGS= -pthread --static
LIB = error_check device environment define fpga_dot_font


.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)

$(addsuffix .o, $(TARGET)): %.o: %.c %.h .mkdir.o
	$(CC) $(C_FLAGS) -c $(basename $@).c -o $(OBJS_DIR)/$(basename $@).o

.PHONY: .mkdir.o
.mkdir.o:
	@mkdir -p $(OBJS_DIR)
