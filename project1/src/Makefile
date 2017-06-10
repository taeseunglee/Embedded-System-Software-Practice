SRC_DIR = .

C_FLAGS=-pthread -static -c -Wall

build: build.lib build.process
	$(CC) $(C_FLAGS) main.c -o $(OBJS_DIR)/main.o
	$(CC) -pthread --static -Wall -o $(SRC_DIR)/../$(MAIN_PROG) $(OBJS_DIR)/*.o


.PHONY: build.process
build.process: build.program
	make -C $(SRC_DIR)/process build

.PHONY: build.program
build.program: build.lib
	make -C $(SRC_DIR)/program

.PHONY: build.lib
build.lib:
	make -C $(SRC_DIR)/lib


include $(SRC_DIR)/rules.mk
