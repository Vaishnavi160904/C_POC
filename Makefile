# Makefile for HR Resume Screening System
# Works with MinGW-w64 (gcc/mingw32-make) on Windows, and gcc/make on Linux/Mac.

CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude -pthread
SRC_DIR = src
OBJ_DIR = obj
BIN = hr_resume_screener.exe

ifeq ($(OS),Windows_NT)
LDLIBS += -lbcrypt
endif

SOURCES = $(wildcard $(SRC_DIR)/*.c) main.c
OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(SOURCES)))

VPATH = $(SRC_DIR)

all: $(BIN)

$(BIN): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

run: all
	./$(BIN)

clean:
	rm -rf $(OBJ_DIR) $(BIN) run_tests run_tests.exe

# ---- CUnit test suite (unit + integration + functional + performance) ----
# Requires libcunit1-dev (Linux/WSL: apt install libcunit1-dev) or the
# "cunit" package via MSYS2 pacman on Windows. See tests/README.md.
TEST_SOURCES = $(wildcard tests/unit/*.c) $(wildcard tests/integration/*.c) \
               $(wildcard tests/functional/*.c) $(wildcard tests/performance/*.c) \
               tests/test_runner.c
TEST_BIN = run_tests

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(SOURCES) $(TEST_SOURCES)
	$(CC) $(CFLAGS) -Itests/include -o $@ $(filter-out main.c,$(SOURCES)) $(TEST_SOURCES) -lcunit $(LDLIBS)

.PHONY: all run clean test
