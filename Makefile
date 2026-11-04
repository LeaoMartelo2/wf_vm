CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

VM_VERSION := $(shell expr `cksum include/wfvm.h | cut -f1 -d' '` % 100000 || 00000)
GIT_HASH := $(shell git describe --always --dirty 2>/dev/null || echo "Not a git environment")
BUILD_DATE := $(shell date "+%Y-%m-%d %H:%M:%S" || echo "Unable to get date(?)")

CFLAGS += -DVM_VERSION=$(VM_VERSION)
CFLAGS += -DGIT_HASH='"$(GIT_HASH)"'
CFLAGS += -DBUILD_DATE='"$(BUILD_DATE)"'

BUILD_DIR = build

BIN_ASSEMBLER = $(BUILD_DIR)/wfasm
BIN_RUNNER = $(BUILD_DIR)/wfrun

.PHONY: all clean directories

all: directories $(BIN_ASSEMBLER) $(BIN_RUNNER)

directories:
	@mkdir -p $(BUILD_DIR)

$(BIN_ASSEMBLER): src/assembler/main.c include/wfvm.h
	$(CC) $(CFLAGS) src/assembler/main.c -o $@

$(BIN_RUNNER): src/runner/main.c include/wfvm.h
	$(CC) $(CFLAGS) src/runner/main.c -o $@

clean:
	rm -rf $(BUILD_DIR)
