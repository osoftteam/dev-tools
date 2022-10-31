.PHONY: all
info:
	@echo $(CXX)
	@which g++
	@g++ --version
	@echo ----------

list: $(wildcard *.cpp)
	@ls -la $?

INC := -Ibenchmark/include
INC += -I/home/ykhodak/dev-tools/dev/utils
#LINK := 
CPPFLAGS := $(INC) $(LINK) -std=c++17 -Wall -Wextra -O3
LDFLAGS := -lbenchmark -lpthread
BUILD := bin

%:
	@mkdir -p $(BUILD)
	cppcheck $@.cpp
	$(CXX) $(CPPFLAGS) $@.cpp $(LDFLAGS) -o $(BUILD)/$@
	ls -la $(BUILD)/$@

clean:
	@\rm -rf $(BUILD)
