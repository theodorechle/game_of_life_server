CPP_C=g++
CPP_FLAGS=-std=c++17 -Wall -g -MMD -MP
BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src
COMMONS_LIB=game_of_life_commons/bin/game_of_life_commons_lib

# Source files
SRC=$(SRC_DIR)/main.cpp

SUBDIRS=shared game network
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
$(MAIN): $(OBJ) $(COMMONS_LIB).a
	@mkdir -p $(BIN_DIR)
	$(CPP_C) $(CPP_FLAGS) -o $@ $^

# Rule for compiling all object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CPP_C) $(CPP_FLAGS) -c $< -o $@

$(COMMONS_LIB).a:
	$(MAKE) -C game_of_life_commons -j lib DEBUG=$(DEBUG)

# Clean all generated files
clean:
	@find obj -mindepth 1 ! -name .gitkeep -delete
	@find bin -mindepth 1 ! -name .gitkeep -delete
	$(MAKE) -C game_of_life_commons clean
