.PHONY: all
info:
	@echo ${DEV_DIR}
	@echo $(CXX)
	@which g++
	@g++ --version
	@echo ----------

list: $(wildcard *.cpp)
	@ls -la $?

DEV_DIR:=${HOME}/dev-tools/dev/
INC := -Ibenchmark/include
INC += -I/home/ykhodak/dev-tools/dev/utils
LINK := -L${DEV_DIR}/utils/build
CPPFLAGS := $(INC) $(LINK) -std=c++17 -Wall -Wextra -O3
LDFLAGS := -lbenchmark -lpthread -ldev-utils -lgtest
BUILD := bin

%:
	@mkdir -p $(BUILD)
	cppcheck $@.cpp
	$(CXX) $(CPPFLAGS) $@.cpp $(LDFLAGS) -o $(BUILD)/$@
	ls -la $(BUILD)/$@

clean:
	@\rm -rf $(BUILD)
