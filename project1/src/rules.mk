BUILD_DIR = $(SRC_DIR)/../build
OBJS_DIR = $(BUILD_DIR)/objs

MAIN_PROG = 20141570

CC= arm-none-linux-gnueabi-gcc
# CC= gcc
C_FLAGS=-pthread -static -c -Wall


.PHONY: clean
clean:
	@echo "Cleaned."
	@rm -rf $(BUILD_DIR) $(SRC_DIR)/../$(MAIN_PROG)

$(addsuffix .o, $(TARGET)): %.o: %.c %.h .mkdir.o
	$(CC) $(C_FLAGS) $(basename $@).c -o $(OBJS_DIR)/$(basename $@).o

.PHONY: .mkdir.o
.mkdir.o:
	@mkdir -p $(OBJS_DIR)
