SRC_DIR = .
TARGET = util environment

build: build.core build.process $(LIB)
	$(CC) $(C_FLAGS) -c main.c -o $(OBJS_DIR)/main.o
	$(CC) $(C_FLAGS) -o $(SRC_DIR)/../$(MAIN_PROG) $(OBJS_DIR)/*.o

build.core: $(addsuffix .o, $(TARGET))

build.process:
	make -C $(SRC_DIR)/process build


include $(SRC_DIR)/rules.mk
