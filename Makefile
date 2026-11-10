CC ?= gcc
CFLAGS ?= -O2 -g -std=c11 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L
CPPFLAGS += -Iinclude
SRC = src/main.c src/buffer.c src/cmd.c src/fileio.c src/utf8.c src/cvnss_parse.c src/cvnss_decode.c src/cvnss_encode.c
OBJ = $(SRC:.c=.o)
TARGET = cved
all: $(TARGET)
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@
src/%.o: src/%.c include/cved.h include/cvnss_engine.h include/utf8.h src/rules_generated.h src/utf8_case_generated.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
test: all
	python3 tests/test_suite.py ./$(TARGET)
strict: CFLAGS += -Werror
strict: clean all test
asan: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer -O1
asan: clean all
clean:
	rm -f $(OBJ) $(TARGET)
.PHONY: all clean test strict asan
