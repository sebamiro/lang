#include <stdio.h>
#include <string.h>

typedef unsigned long u64;
typedef unsigned int u32;

u64 hash(char* str, u32 len)
{
	u64 hash = 5381;
	for (u32 i = 0; i < len; ++i)
	{
		hash = ((hash << 5) + hash) + str[i];
	}
	return hash;
}

void test(char *s)
{
	u64 h = hash(s, strlen(s));

	printf("[%s] %ld, %ld\n", s, h, h % 4096);
}

int main(void)
{
	test("hola");
	test("Hola");
	test("main");
	test("h");
	test("H");
	test("n");
	test("testFunction");
	test("calculate_ateasad");
}
