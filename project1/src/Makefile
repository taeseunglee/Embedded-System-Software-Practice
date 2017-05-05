SRC_DIR = .
MAIN = main

build: build.lib build.process
	$(CC) $(LIB_PATH_FLAGS) $(C_FLAGS) -c main.c -o $(OBJS_DIR)/main.o
	$(CC) -Wall -o $(SRC_DIR)/../$(MAIN_PROG) $(OBJS_DIR)/*.o $(PTHREAD_FLAG)


.PHONY: build.process
build.process: build.program build.thread
	make -C $(SRC_DIR)/process build

.PHONY: build.program
build.program: build.thread
	make -C $(SRC_DIR)/program

.PHONY: build.thread
build.thread:
	make -C $(SRC_DIR)/thread

.PHONY: build.lib
build.lib:
	make -C $(SRC_DIR)/lib

#$(LIB_DIR)/error_check $(LIB_DIR)/device \
#					$(LIB_DIR)/define $(LIB_DIR)/fpga_dot_font


include $(SRC_DIR)/rules.mk
