#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef int32_t b32;

#include "token.h"

#include "lexer.c"

#define ArrayTokens(...) (type_token[]){ __VA_ARGS__, Token_EOF, Token_None }

#define Tests_Lexer \
	Test("main() {}", ArrayTokens(Token_Identifier, '(', ')', '{', '}')); \
	Test("1 + 1", ArrayTokens(Token_Number, '+', Token_Number)); \
	Test("a = 1", ArrayTokens(Token_Identifier, '=', Token_Number)); \
	Test("a == 132", ArrayTokens(Token_Identifier, Token_Equal, Token_Number)); \
	Test("a === 132", ArrayTokens(Token_Identifier, Token_AssignEqual, Token_Number)); \
	Test("a != 132", ArrayTokens(Token_Identifier, Token_NotEqual, Token_Number)); \
	Test("a >= 132", ArrayTokens(Token_Identifier, Token_GreaterEqual, Token_Number)); \
	Test("a <= 132", ArrayTokens(Token_Identifier, Token_LessEqual, Token_Number)); \
	Test("a >> 132", ArrayTokens(Token_Identifier, Token_ShiftRight, Token_Number)); \
	Test("a << 132", ArrayTokens(Token_Identifier, Token_ShiftLeft, Token_Number)); \
	Test("a =| 132", ArrayTokens(Token_Identifier, Token_AssignOr, Token_Number)); \
	Test("a =& 132", ArrayTokens(Token_Identifier, Token_AssignAnd, Token_Number)); \
	Test("a =< 132", ArrayTokens(Token_Identifier, Token_AssignLess, Token_Number)); \
	Test("a =<= 132", ArrayTokens(Token_Identifier, Token_AssignLessEqual, Token_Number)); \
	Test("a => 132", ArrayTokens(Token_Identifier, Token_AssignGreater, Token_Number)); \
	Test("a =>= 132", ArrayTokens(Token_Identifier, Token_AssignGreaterEqual, Token_Number)); \
	Test("a =<< 132", ArrayTokens(Token_Identifier, Token_AssignShiftLeft, Token_Number)); \
	Test("a =>> 132", ArrayTokens(Token_Identifier, Token_AssignShiftRight, Token_Number)); \
	Test("a =+ 132", ArrayTokens(Token_Identifier, Token_AssignPlus, Token_Number)); \
	Test("a =- 132", ArrayTokens(Token_Identifier, Token_AssignMinus, Token_Number)); \
	Test("a =% 132", ArrayTokens(Token_Identifier, Token_AssignModulo, Token_Number)); \
	Test("a =* 132", ArrayTokens(Token_Identifier, Token_AssignMult, Token_Number)); \
	Test("a =/ 132", ArrayTokens(Token_Identifier, Token_AssignDiv, Token_Number)); \
	Test("a == 132", ArrayTokens(Token_Identifier, Token_Equal, Token_Number)); \
	Test("a != 132", ArrayTokens(Token_Identifier, Token_NotEqual, Token_Number)); \
	Test("a >= 132", ArrayTokens(Token_Identifier, Token_GreaterEqual, Token_Number)); \
	Test("a <= 132", ArrayTokens(Token_Identifier, Token_LessEqual, Token_Number)); \
	Test("a << 132", ArrayTokens(Token_Identifier, Token_ShiftLeft, Token_Number)); \
	Test("a >> 132", ArrayTokens(Token_Identifier, Token_ShiftRight, Token_Number)); \
	Test("auto i, s;", ArrayTokens(Token_Auto, Token_Identifier, ',', Token_Identifier, ';')); \
	Test("extrn putchar, char;", ArrayTokens(Token_Extrn, Token_Identifier, ',', Token_Identifier, ';')); \
	Test("if (char(s, i)) {} else {}", ArrayTokens(Token_If, '(', Token_Identifier, '(', Token_Identifier, ',', Token_Identifier, ')', ')', '{', '}', Token_Else, '{', '}')); \
	Test("while (char(s, i)) {}", ArrayTokens(Token_While, '(', Token_Identifier, '(', Token_Identifier, ',', Token_Identifier, ')', ')', '{', '}')); \
	Test("goto Label;", ArrayTokens(Token_Goto, Token_Identifier, ';')); \
	Test("switch (a) { case 1: {} }", ArrayTokens(Token_Switch, '(', Token_Identifier, ')', '{', Token_Case, Token_Number, ':', '{', '}', '}')); \
	Test("return 0;", ArrayTokens(Token_Return, Token_Number, ';')); \
	Test("\"string\";", ArrayTokens(Token_String, ';')); \
	Test("'c';", ArrayTokens(Token_Char, ';')); \
	Test("++a", ArrayTokens(Token_Increment, Token_Identifier)); \
	Test("a++", ArrayTokens(Token_Identifier, Token_Increment)); \
	Test("--a", ArrayTokens(Token_Decrement, Token_Identifier)); \
	Test("a--", ArrayTokens(Token_Identifier, Token_Decrement)); \

#define Tests_Lexer_Numbers \
	Test("a = 123;", 123); \
	Test("a = 321;", 321); \
	Test("a = 1;", 1); \
	Test("12;", 12); \

u32 totalTests = 0;
u32 okTests = 0;

void TestLexer(char*str, type_token* tokens)
{
	lexer lex = InitLexer((u8*)str, strlen(str));

	totalTests++;
	printf("  [%s] ", str);
	for (u32 i = 0; tokens[i] != Token_None; i++)
	{
		token token = GetNextToken(&lex);
		if (token.type != tokens[i])
		{
			printf("##KO\n    ");
			printToken(token.type);
			printf(" != ");
			printToken(tokens[i]);
			printf("\n");
			return;
		}
	}
	printf("OK\n");
	okTests++;
	return;
}

void TestGetIdent(char*str, type_token* tokens)
{
	char buffer[256];
	lexer lex = InitLexer((u8*)str, strlen(str));

	totalTests++;
	printf("  [%s] ", str);
	for (u32 i = 0; tokens[i] != Token_None; i++)
	{
		token token = GetNextToken(&lex);
		if (tokens[i] != Token_Identifier)
		{
			continue;
		}
		if (token.type != tokens[i])
		{
			printf("##KO\n    ");
			printToken(token.type);
			printf(" != ");
			printToken(tokens[i]);
			printf("\n");
			return;
		}
		u32 len = GetIdentifier(&lex, token, buffer);
		if (len == 0)
		{
			printf("##KO\n    ");
			printf("Identifier length expected to be > 0\n");
			return;
		}

		for (u32 i = 0; i < len; i++)
		{
			if (str[token.start + i] != buffer[i])
			{
				printf("##KO\n    ");
				printf("String returned not equal to the identifier: %s != %s\n", str, buffer);
				return;
			}
		}
	}
	printf("OK\n");
	okTests++;
	return;
}

void TestGetNumber(char*str, u32 expected)
{
	lexer lex = InitLexer((u8*)str, strlen(str));

	totalTests++;
	printf("  [%s] ", str);
	while (1)
	{
		token token = GetNextToken(&lex);
		if (token.type != Token_Number)
		{
			continue;
		}
		u32 got = GetNumber(&lex, token);
		if (got != expected)
		{
			printf("##KO\n    ");
			printf(" %d != %d\n", got, expected);
			return;
		}
		break;
	}
	printf("OK\n");
	okTests++;
	return;
}

#define TestBinaryOperation(op) \
	"main() { return "#op" }"

int main(void)
{

	printf("\n- TestLexer \n");
#define Test TestLexer
	Tests_Lexer
#undef Test

	printf("\n- TestGetIdent \n");
#define Test TestGetIdent
	Tests_Lexer
#undef Test

	printf("\n- TestGetNumber \n");
#define Test TestGetNumber
	Tests_Lexer_Numbers
#undef Test

	printf("\n-------\n  %d/%d\n\n", okTests, totalTests);
}
