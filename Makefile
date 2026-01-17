CPP_C=g++
CPP_FLAGS=-std=c++17 -Wall -g -MMD -MP
BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src

# Source files
SRC=$(SRC_DIR)/main.cpp

SUBDIRS=shared game_thread network_thread
SRC_SUBDIRS=$(foreach dir, $(SUBDIRS), $(wildcard $(SRC_DIR)/$(dir)/*.cpp))

# Object files
OBJ=$(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC)) $(foreach dir, $(SUBDIRS), $(wildcard $(SRC_DIR)/$(dir)/*.cpp))

# Executable targets
MAIN=$(BIN_DIR)/game_of_life_server

.PHONY: all clean tests

ifeq ($(DEBUG),1)
CPP_FLAGS += -DDEBUG
endif

all: $(MAIN)

# Build everything except tests
$(MAIN): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CPP_C) $(CPP_FLAGS) -o $@ $^

# Rule for compiling all object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CPP_C) $(CPP_FLAGS) -c $< -o $@

# Clean all generated files
clean:
	@find obj -mindepth 1 ! -name .gitkeep -delete
	@find bin -mindepth 1 ! -name .gitkeep -delete
	$(MAKE) -C cpp_gui clean
