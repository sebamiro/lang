Name=B
OutAsm=/tmp/out.s
OutObj=/tmp/out.o
OutExe=/tmp/out

CFLAGS= -g -Wall -Werror -Wextra -O3

all:
	gcc $(CFLAGS) -o $(Name) main.c

run: all
	./$(Name) test.b

# run: all
# 	./$(Name) test.b > $(OutAsm)
# 	cat $(OutAsm)
# 	gcc -c -m32 -x assembler $(OutAsm) -o $(OutObj)
# 	ld -m elf_i386 $(OutObj) ./brt0.o -o $(OutExe)
# 	$(OutExe)

test:
	gcc $(CFLAGS) -o $(Name)_test test.c && ./$(Name)_test
	@bash ./test_binary.sh

h:
	gcc $(CFLAGS) -o hash hash_table.c  && ./hash
