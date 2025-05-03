#! /bin/bash


binaryB="/tmp/binary.b"
binaryExe="/tmp/binary"
OutAsm="/tmp/binary.s"
OutObj="/tmp/binary.o"

function createCBinary {
	echo "main() { return $1; }" > $binaryB
	./B $binaryB > $OutAsm
	gcc -c -m32 -x assembler $OutAsm -o $OutObj
	ld -m elf_i386 $OutObj ./brt0.o -o $binaryExe
	$binaryExe
}

function Test {
	createCBinary "$1"
	if [ $? -eq $(("$1")) ]
	then
		echo "  " ["$1"] OK
	else
		echo "  " ["$1"] KO $? $(("$1"))
	fi
}

echo - TestBinaryOp

Test "1 + 1"
Test '2 * 5'
Test '42 / 10'
Test '42 % 10'
Test '42 | 10'
Test '42 & 10'
Test '1 << 4'
Test '42 >> 3'

Test '2 * 5 + 42'
Test '2 * 5 + 42 / 10'
Test '42 + 2 * 5'
Test '42 + 2 | 5'
Test '42 | 2 | 5'
Test '42 + 2 & 5'
Test '42 & 2 & 5'
Test '42 + 2 / 5'
Test '2 / 5 + 42'
Test '42 + 2 % 5'
Test '2 % 5 + 42'
