SRC_DIR = .
TARGET = util environment


build: build.core build.process $(LIB)
	$(CC) $(C_FLAGS) -c main.c -o $(OBJS_DIR)/main.o
	$(CC) -Wall -o $(SRC_DIR)/../$(MAIN_PROG) $(OBJS_DIR)/*.o

build.core: $(addsuffix .o, $(TARGET))

.PHONY: build.process
build.process: build.program
	make -C $(SRC_DIR)/process build

.PHONY: build.program
build.program: build.thread
	make -C $(SRC_DIR)/program

.PHONY: build.thread
build.thread:
	make -C $(SRC_DIR)/thread


include $(SRC_DIR)/rules.mk
