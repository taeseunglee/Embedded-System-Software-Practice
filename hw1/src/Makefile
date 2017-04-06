SRC_DIR = .
TARGET = util
LIB = error_check device environment define fpga_dot_font

build: build.core build.process $(addsuffix .h, $(LIB))
	$(CC) $(C_FLAGS) -c main.c -o $(OBJS_DIR)/main.o
	$(CC) $(C_FLAGS) -o $(BUILD_DIR)/$(MAIN_PROG) $(OBJS_DIR)/*.o

build.core: $(addsuffix .o, $(TARGET))

build.process:
	make -C $(SRC_DIR)/process build


include $(SRC_DIR)/rules.mk
