CC = cc

CFLAGS = -Wall -Werror -Wextra -Wpedantic
ifeq ($(MAKECMDGOALS), release)
	CFLAGS += -DNDEBUG -O3
else
	CFLAGS += -ggdb -O0
endif

SRC_DIR = .
ARENA_ALLOC_DIR = $(SRC_DIR)/arena_alloc
BIN_DIR = bin

SRC_FILES = $(wildcard $(SRC_DIR)/*.c $(ARENA_ALLOC_DIR)/*.c)
OBJ_FILES = $(patsubst %.c, $(BIN_DIR)/%.o, $(SRC_FILES))

TARGET = $(BIN_DIR)/test_arena

.PHONY: all release clean

all: $(TARGET)

release: all

clean:
	rm -f $(TARGET) $(OBJ_FILES)

$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
