# Executable's file name
EXEC := cmpcat

# Paths to directories
BUILD_DIR := ./build
SRC_DIR := ./src
INC_DIR := ./include

# Source files (.c)
SRCS := $(wildcard $(SRC_DIR)/*.c)
# Object files (.o)
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

# Compiler
CC = gcc
# Compiler options
CFLAGS = -Wall -Wextra -pedantic -g $(addprefix -I,$(INC_DIR))

$(EXEC): $(OBJS)
	$(CC) $^ -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $(OBJS))
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(EXEC)

# accounting
count:
	wc $(SRCS) $(wildcard $(INC_DIR)/*.h)

.PHONY: clean count