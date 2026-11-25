SRC=main.c curst.h defer.h do.h Makefile
FLAGS=-Wall -Wextra -Wno-dangling-else -ftrack-macro-expansion=0 -fsanitize=address,undefined
BIN=curst
all=$(BIN)

$(BIN): $(SRC)
	gcc $(FLAGS) main.c -o $@

.PHONY: debug
debug: $(SRC)
	gcc -E -P main.c | python3 -c "import sys; print(sys.stdin.read().encode('unicode_escape').replace(b'\\U',b'U').decode('unicode_escape'))" > out.c
	clang-format out.c > out2.c
	gcc $(FLAGS) out2.c

.PHONY: clean
clean:
	rm -f $(BIN) out.c out2.c
