#include <string.h>

lexer InitLexer(u8* buf, u32 len)
{
	lexer res = {0};

	res.buf = buf;
	res.len = len;
	res.posCurToken = -1;
	return res;
}

inline char peekChar(lexer* lex)
{
	return lex->buf[lex->cur];
}

inline char peekNextChar(lexer* lex)
{
	return lex->buf[lex->cur + 1];
}

inline char eatChar(lexer* lex)
{
	return lex->buf[lex->cur++];
}

inline b32 isDigit(char c)
{
	return (c >= '0' && c <= '9');
}

inline b32 isAlphaDigit(char c)
{
	c &= ~0x20;
	return isDigit(c) || (c >= 'A' && c <= 'Z');
}

inline b32 isWhiteSpace(char c)
{
	return c > 0 && c <= 32;
}

inline void skipComment(lexer* lex)
{
	char c;
	u32 commentLevel = 1;

	while (commentLevel > 0)
	{
		c = peekChar(lex);
		if (c == '/')
		{
			if (peekNextChar(lex) == '*')
			{
				commentLevel += 1;
				eatChar(lex);
			}
		}
		else if (c == '*')
		{
			if (peekNextChar(lex) == '/')
			{
				commentLevel -= 1;
				eatChar(lex);
			}
		}
		eatChar(lex);
	}
	while (isWhiteSpace(peekChar(lex)))
	{
		eatChar(lex);
	}
}

token GetNextToken(lexer* lex)
{
	token res = {0};

	while (isWhiteSpace(peekChar(lex)))
	{
		eatChar(lex);
	}

	if (peekChar(lex) == '/' && peekNextChar(lex) == '*')
	{
		lex->cur += 2;
		skipComment(lex);
	}


	char c = peekChar(lex);
	res.type = Token_None;
	switch (c)
	{
		case 0:
		case '[':
		case ']':
		case '{':
		case '}':
		case '(':
		case ')':
		case '*':
		case '/':
		case '%':
		case '&':
		case '|':
		case ',':
		case ';':
		case ':':
		case '?':
		{
			res.type = c;
			res.start = lex->cur;
			eatChar(lex);
		} break;

		case '=': /* Assign|Equal */
		{
			res.type = c;
			res.start = lex->cur;
			eatChar(lex);
			c = peekChar(lex);
			switch (c)
			{
				case '=':
				{
					eatChar(lex);
					c = peekChar(lex);
					if (c == '=')
					{
						res.type = Token_AssignEqual;
						eatChar(lex);
					}
					else
					{
						res.type = Token_Equal;
					}
				}break;
				case '|':
				{
					res.type = Token_AssignOr;
					eatChar(lex);
				} break;
				case '&':
				{
					res.type = Token_AssignAnd;
					eatChar(lex);
				} break;
				case '!':
				{
					if (peekNextChar(lex) == '=')
					{
						res.type = Token_AssignNotEqual;
						eatChar(lex);
						eatChar(lex);
					}
				} break;
				case '<':
				{
					eatChar(lex);
					c = peekChar(lex);
					if (c == '=')
					{
						res.type = Token_AssignLessEqual;
						eatChar(lex);
					}
					else if (c == '<')
					{
						res.type = Token_AssignShiftLeft;
						eatChar(lex);
					}
					else
					{
						res.type = Token_AssignLess;
					}
				} break;
				case '>':
				{
					eatChar(lex);
					c = peekChar(lex);
					if (c == '=')
					{
						res.type = Token_AssignGreaterEqual;
						eatChar(lex);
					}
					else if (c == '>')
					{
						res.type = Token_AssignShiftRight;
						eatChar(lex);
					}
					else
					{
						res.type = Token_AssignGreater;
					}
				} break;
				case '+':
				{
					res.type = Token_AssignPlus;
					eatChar(lex);
				} break;
				case '-':
				{
					res.type = Token_AssignMinus;
					eatChar(lex);
				} break;
				case '%':
				{
					res.type = Token_AssignModulo;
					eatChar(lex);
				} break;
				case '*':
				{
					res.type = Token_AssignMult;
					eatChar(lex);
				} break;
				case '/':
				{
					res.type = Token_AssignDiv;
					eatChar(lex);
				} break;
				default:
				{}break;
			}
		} break;

		case '!': /* Negation|NotEqual */
		{
			res.type = c;
			res.start = lex->cur;
			eatChar(lex);
			if (peekChar(lex) == '=')
			{
				res.type = Token_NotEqual;
				eatChar(lex);
			}
		} break;
		case '+': /* Plus|Inc */
		{
			res.type = c;
			res.start = lex->cur;
			eatChar(lex);
			if (peekChar(lex) == '+')
			{
				res.type = Token_Increment;
				eatChar(lex);
			}
		} break;
		case '-': /* Minus|Dec */
		{
			res.type = c;
			res.start = lex->cur;
			eatChar(lex);
			if (peekChar(lex) == '-')
			{
				res.type = Token_Decrement;
				eatChar(lex);
			}
		} break;
		case '>':
		{
			res.type = c;
			res.start = lex->cur;
			eatChar(lex);
			c = peekChar(lex);
			if (c == '=')
			{
				res.type = Token_GreaterEqual;
				eatChar(lex);
			}
			else if (c == '>')
			{
				res.type = Token_ShiftRight;
				eatChar(lex);
			}
		} break;
		case '<':
		{
			res.type = c;
			res.start = lex->cur;
			eatChar(lex);
			c = peekChar(lex);
			if (c == '=')
			{
				res.type = Token_LessEqual;
				eatChar(lex);
			}
			else if (c == '<')
			{
				res.type = Token_ShiftLeft;
				eatChar(lex);
			}
		} break;

		case '\'':
		{
			eatChar(lex);
			res.start = lex->cur;
			c = eatChar(lex);
			if (c == '*')
			{
				c = eatChar(lex);
			}
			if (eatChar(lex) == '\'')
			{
				res.type = Token_Char;
			}
		} break;

		case '"':
		{
			eatChar(lex);
			res.start = lex->cur;
			res.type = Token_String;

			while (eatChar(lex) != '"') {}
		} break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			res.start = lex->cur;
			res.type = Token_Number;
			for (; isDigit(c); c = peekChar(lex))
			{
				eatChar(lex);
			}
		} break;

		default:
		{
			res.start = lex->cur;
			res.type = Token_Identifier;
			u32 len = 0;
			while (isAlphaDigit(peekChar(lex)))
			{
				eatChar(lex);
				++len;
			}
			/* Keywords */
			switch (len)
			{
				case 2:
				{
					if (strncmp((char*)(lex->buf + res.start), "if", 2) == 0)
					{
						res.type = Token_If;
					}
				} break;
				case 4:
				{
					if (strncmp((char*)(lex->buf + res.start), "auto", 4) == 0)
					{
						res.type = Token_Auto;
					}
					if (strncmp((char*)(lex->buf + res.start), "case", 4) == 0)
					{
						res.type = Token_Case;
					}
					if (strncmp((char*)(lex->buf + res.start), "else", 4) == 0)
					{
						res.type = Token_Else;
					}
					if (strncmp((char*)(lex->buf + res.start), "goto", 4) == 0)
					{
						res.type = Token_Goto;
					}
				} break;
				case 5:
				{
					if (strncmp((char*)(lex->buf + res.start), "extrn", 5) == 0)
					{
						res.type = Token_Extrn;
					}
					if (strncmp((char*)(lex->buf + res.start), "while", 5) == 0)
					{
						res.type = Token_While;
					}
				} break;
				case 6:
				{
					if (strncmp((char*)(lex->buf + res.start), "switch", 6) == 0)
					{
						res.type = Token_Switch;
					}
					if (strncmp((char*)(lex->buf + res.start), "return", 6) == 0)
					{
						res.type = Token_Return;
					}
				} break;
			}
		} break;
	}

	lex->curToken = res;
	lex->posCurToken = lex->cur;
	return res;
}

token PeekToken(lexer* lex)
{
	token res = lex->curToken;

	if (lex->cur != lex->posCurToken)
	{
		res = GetNextToken(lex);
		lex->curToken = res;
	}

	return res;
}

/**
 * Copies the identifier to buffer and returns its len.
 */
u32 GetIdentifier(lexer* lex, token token, char* buffer)
{
	u32 len = 0;
	u32 startIdent = token.start;

	while (isAlphaDigit(lex->buf[startIdent + len]))
	{
		buffer[len] = lex->buf[startIdent + len];
		len++;
	}
	buffer[len] = 0; // Zero terminated, because
	return len;
}

// TODO: Add support for negative numbers
u32 GetNumber(lexer* lex, token token)
{
	u32 result = 0;

	for (u32 i = token.start; isDigit(lex->buf[i]); i++)
	{
		result = result*10 + (lex->buf[i] - '0');
	}
	return result;
}

void printToken(type_token t)
{
	switch(t)
	{
		case Token_None:
			printf("Token_None"); break;
		case Token_EOF:
			printf("Token_EOF"); break;
		case Token_Increment:
			printf("Token_Increment"); break;
		case Token_Decrement:
			printf("Token_Decrement"); break;
		case Token_Identifier:
			printf("Token_Identifier"); break;
		case Token_Number:
			printf("Token_Number"); break;
		case Token_Char:
			printf("Token_Char"); break;
		case Token_String:
			printf("Token_String"); break;
		case Token_Equal:
			printf("Token_Equal"); break;
		case Token_NotEqual:
			printf("Token_NotEqual"); break;
		case Token_GreaterEqual:
			printf("Token_GreaterEqual"); break;
		case Token_LessEqual:
			printf("Token_LessEqual"); break;
		case Token_ShiftLeft:
			printf("Token_ShiftLeft"); break;
		case Token_ShiftRight:
			printf("Token_ShiftRight"); break;
		case Token_AssignOr:
			printf("Token_AssignOr"); break;
		case Token_AssignAnd:
			printf("Token_AssignAnd"); break;
		case Token_AssignEqual:
			printf("Token_AssignEqual"); break;
		case Token_AssignNotEqual:
			printf("Token_AssignNotEqual"); break;
		case Token_AssignLess:
			printf("Token_AssignLess"); break;
		case Token_AssignLessEqual:
			printf("Token_AssignLessEqual"); break;
		case Token_AssignGreater:
			printf("Token_AssignGreater"); break;
		case Token_AssignGreaterEqual:
			printf("Token_AssignGreaterEqual"); break;
		case Token_AssignShiftLeft:
			printf("Token_AssignShiftLeft"); break;
		case Token_AssignShiftRight:
			printf("Token_AssignShiftRight"); break;
		case Token_AssignPlus:
			printf("Token_AssignPlus"); break;
		case Token_AssignMinus:
			printf("Token_AssignMinus"); break;
		case Token_AssignModulo:
			printf("Token_AssignModulo"); break;
		case Token_AssignMult:
			printf("Token_AssignMult"); break;
		case Token_AssignDiv:
			printf("Token_AssignDiv"); break;
		case Token_Auto:
			printf("Token_Auto"); break;
		case Token_Extrn:
			printf("Token_Extrn"); break;
		case Token_Case:
			printf("Token_Case"); break;
		case Token_If:
			printf("Token_If"); break;
		case Token_Else:
			printf("Token_Else"); break;
		case Token_While:
			printf("Token_While"); break;
		case Token_Switch:
			printf("Token_Switch"); break;
		case Token_Goto:
			printf("Token_Goto"); break;
		case Token_Return:
			printf("Token_Return"); break;
		default:
			printf("%c", t); break;
	}
}

void ReportCompilerError(lexer* lex, token token, const char* format)
{
	u32 line = 1;
	u32 cursor = 1;

	u32 beginLine = 0;
	for (u32 i = 0; i < token.start; ++i)
	{
		if (lex->buf[i] == '\n')
		{
			line++;
			beginLine = i + 1;
		}
	}
	u32 endLine = token.start;
	cursor = token.start - beginLine + 1;
	while (lex->buf[endLine] != '\n' && endLine < lex->len)
	{
		endLine++;
	}

	u32 ident = 3;
	for (u32 i = beginLine; lex->buf[i] == '\t' || lex->buf[i] == ' '; i++)
	{
		if ('\t') ident += 2;
		if (' ') ident += 1;
	}
	lex->buf[endLine] = 0;
	fprintf(stderr, "%s\nfile.b:%d:%d\n%d | %s\n", format, line, cursor, line, (char*)(lex->buf + beginLine));
	fprintf(stderr, "  |%*s\n\n", cursor + ident, "^~~");
}


