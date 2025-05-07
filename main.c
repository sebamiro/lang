#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <stdint.h>

typedef uint8_t u8;
typedef int32_t b32;
typedef int32_t s32;
typedef uint32_t u32;
typedef uint64_t u64;

#include "token.h"

#define Log(format, ...) fprintf(stderr, format, __VA_ARGS__)

#include "lexer.c"
#include "parser.c"


int main(int argc, char** argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "USAGE: %s <source>.b\n", *argv);
		return 1;
	}
	++argv;

	struct stat s;
	if (stat(*argv, &s))
	{
		perror("Stat");
		return 1;
	}

	FILE* file = fopen(*argv, "r");
	if (!file)
	{
		perror("File");
		return 1;
	}
	u8* buf = malloc(s.st_size);
	assert(buf);
	if (!fread(buf, s.st_size, 1, file))
	{
		fprintf(stderr, "read error\n");
		return 1;
	}

	lexer lex = InitLexer(buf, s.st_size);

	parser parser = {0};
	parser.lex = &lex;
	return Parse(&parser);
}
