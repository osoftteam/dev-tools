all: cfix

C++ = g++
INC := -I/home/ykhodak/dev-tools/dev/utils
CPPFLAGS = $(INC) -g -std=c++17 -O0
LDFLAGS = -lpthread
BUILD_DIR := ./bin


$(BUILD_DIR)/%.o : %.cpp
	@mkdir -p $(dir $@)
	$(CC) -c $(CPPFLAGS) $< -o $@	

cfix_obj = cfix_main.o cfix.o tcp_spinner.o
cfix_obj_path = $(patsubst %,$(BUILD_DIR)/%,$(cfix_obj))

cfix: $(cfix_obj_path)
	$(C++) $(cfix_obj_path) $(LDFLAGS) -o $(BUILD_DIR)/$@

.PHONY: clean

clean:
	rm -f $(BUILD_DIR)/*.o cfix
