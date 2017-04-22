BUILD_DIR = $(SRC_DIR)/../build
OBJS_DIR = $(BUILD_DIR)/objs
LIB_DIR = $(SRC_DIR)/lib

MAIN_PROG = 20141570
# LIB = $(LIB_DIR)/error_check $(LIB_DIR)/device $(LIB_DIR)/define $(LIB_DIR)/fpga_dot_font

# CC= arm-none-linux-gnueabi-gcc
CC= gcc
C_FLAGS= -pthread --static -Wall
LIB_PATH_FLAGS = -I $(LIB_DIR)


.PHONY: clean
clean:
	@echo "Cleaned."
	@rm -rf $(BUILD_DIR) $(SRC_DIR)/../$(MAIN_PROG)

$(addsuffix .o, $(TARGET)): %.o: %.c %.h .mkdir.o
	$(CC) $(LIB_PATH_FLAGS) $(C_FLAGS) -c $(basename $@).c -o $(OBJS_DIR)/$(basename $@).o

.PHONY: .mkdir.o
.mkdir.o:
	@mkdir -p $(OBJS_DIR)
