# Makefile for HR Resume Screening System
# Works with MinGW-w64 (gcc/mingw32-make) on Windows, and gcc/make on Linux/Mac.

CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude -pthread
<<<<<<< HEAD
COVERAGE_CFLAGS = -Wall -Wextra -g -O0 -fprofile-arcs -ftest-coverage -Iinclude -pthread
=======
>>>>>>> 931690db4b496c0f25d92c94054677714538d9fa
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
<<<<<<< HEAD
	rm -f *.gcda *.gcno *.gcov coverage_source.txt
=======
>>>>>>> 931690db4b496c0f25d92c94054677714538d9fa

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

<<<<<<< HEAD
cppcheck:
	cppcheck --enable=all --inconclusive --std=c11 --force -Iinclude --suppress=missingIncludeSystem --error-exitcode=1 src main.c tests

.PHONY: all run clean test coverage coverage-clean cppcheck

# ---- GCC/Gcov code coverage ----
# Builds and runs the complete CUnit suite with coverage instrumentation,
# then reports coverage for production code under src/ only.
# ---- GCC/Gcov code coverage ----
# ---- GCC/Gcov code coverage ----

COVERAGE_DIR = coverage_obj
COVERAGE_BIN = coverage_tests

coverage:
	rm -rf $(COVERAGE_DIR) $(COVERAGE_BIN) *.gcov coverage_source.txt
	mkdir -p $(COVERAGE_DIR)

	@echo ""
	@echo "========================================"
	@echo "   BUILDING WITH COVERAGE"
	@echo "========================================"

	@for f in $(wildcard src/*.c); do \
		base=$$(basename $$f .c); \
		echo "Compiling $$f"; \
		$(CC) $(COVERAGE_CFLAGS) -c $$f -o $(COVERAGE_DIR)/$$base.o || exit 1; \
	done

	@echo ""
	@echo "========================================"
	@echo "   BUILDING TEST EXECUTABLE"
	@echo "========================================"

	$(CC) $(COVERAGE_CFLAGS) -Itests/include \
		-o $(COVERAGE_BIN) \
		$(COVERAGE_DIR)/*.o \
		$(TEST_SOURCES) \
		-lcunit $(LDLIBS)

	@echo ""
	@echo "========================================"
	@echo "       RUNNING TESTS"
	@echo "========================================"

	./$(COVERAGE_BIN)

	@echo ""
	@echo "========================================"
	@echo "       CODE COVERAGE REPORT"
	@echo "========================================"

	@for f in $(wildcard src/*.c); do \
		gcov -o $(COVERAGE_DIR) $$f; \
	done | tee coverage_source.txt

	@echo ""
	@echo "========================================"
	@echo "       SOURCE COVERAGE SUMMARY"
	@echo "========================================"

		@awk '/^File '\''src\/.*\.c'\''/ { \
		file=$$0; \
	} \
	/^Lines executed:/ { \
		if ($$0 ~ /No executable lines/) next; \
		split($$2, a, ":"); \
		split(a[2], b, "%"); \
		percent=b[1]; \
		lines=$$4; \
		if (lines > 0) { \
			executed += (percent / 100) * lines; \
			total += lines; \
		} \
	} \
	END { \
		if (total > 0) \
			printf "Overall Source Coverage: %.2f%% (%d/%d lines)\n", \
			(executed / total) * 100, executed + 0.5, total; \
		else \
			print "No executable source lines found"; \
	}' coverage_source.txt

coverage-clean:
	rm -rf $(COVERAGE_DIR) $(COVERAGE_BIN)
	rm -f *.gcda *.gcno *.gcov coverage_source.txt
=======
.PHONY: all run clean test
>>>>>>> 931690db4b496c0f25d92c94054677714538d9fa
