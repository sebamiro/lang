// #include "ast.c"

#define MaxIdetifierLenght 256
#define MaxExternDeclarations 256

typedef struct {
	lexer*	lex;

	char	scratchBuffer[MaxIdetifierLenght];
	char	extrn[MaxExternDeclarations][MaxIdetifierLenght];
	u32		lenExtrn;
} parser;

typedef struct
{
	enum {
		Rval_Lval,
		Rval_Constant,
		Rval_Identifier,
		Rval_OnStack,
	} type;
	token token;
	b32 hasError;
} rval;

enum binary_precedence
{
	Prec_BinCmp,
	Prec_BinShift,
	Prec_BinUnary,
	Prec_BinAdd,
	Prec_BinDiv,
	Prec_BinMul,
};

b32 parseArgList(parser* parser, type_token endToken);

b32 ExpectNextToken(lexer* lex, type_token type)
{
	token t = GetNextToken(lex);
	return t.type == type;
}

b32 ExpectToken(lexer* lex, type_token type)
{
	token t = PeekToken(lex);

	return t.type == type;
}

rval term(parser* parser)
{
	rval result = {0};

	token token = PeekToken(parser->lex);
	switch (token.type)
	{
		case Token_Char:
		case Token_Number:
		case Token_String:
		{
			result.token = token;
			result.type = Rval_Constant;
			GetNextToken(parser->lex);
		} break;
		case Token_Identifier:
		{
			result.token = token;
			result.type = Rval_Identifier;
			GetNextToken(parser->lex);
		} break;
		default:
		{
			ReportCompilerError(parser->lex, token, "Expected identifier or constant:\n");
			result.hasError = 1;
		} break;
	}
	return result;
}

s32 getPrecedence(type_token t)
{
	switch (t)
	{
		case Token_ShiftLeft:
		case Token_ShiftRight:
			return Prec_BinShift;
		case '&':
		case '|':
			return Prec_BinUnary;
		case Token_Equal:
		case Token_NotEqual:
		case '>':
		case '<':
		case Token_GreaterEqual:
		case Token_LessEqual:
			return Prec_BinCmp;
		case '+': return Prec_BinAdd;
		case '*': return Prec_BinMul;
		case '/':
		case '%':
		{
			return Prec_BinDiv;
		} break;
		case ';':
		case ')': return 0;
		default:
		{
			return -1;
		} break;
	}
	return 0;
}

b32 binary_op(parser* parser, rval* lhs, s32 curPrecedence, type_token end)
{
	token operation = PeekToken(parser->lex);
	s32 opPrecedence = getPrecedence(operation.type);

	if (opPrecedence == -1)
	{
		ReportCompilerError(parser->lex, lhs->token, "Expected binary operator or ';' after value:\n");
		return 1;
	}

	if (operation.type == end || opPrecedence < curPrecedence)
	{
		Log("  rhs: %d\n", GetNumber(parser->lex, lhs->token));
		return 0;
	}

	Log("Operation: %c\n", operation.type);
	if (lhs->type == Rval_Constant) Log("  lhs: %d\n", GetNumber(parser->lex, lhs->token));
	token tokenRhs = GetNextToken(parser->lex);
	rval rhs;
	if (tokenRhs.type == '(')
	{
		GetNextToken(parser->lex);
		rhs = term(parser);
		if (binary_op(parser, &rhs, 0, ')')) return 1;
		GetNextToken(parser->lex);
		return 0;
	}
	rhs = term(parser);
	if (rhs.hasError) return 1;
	if (binary_op(parser, &rhs, opPrecedence, end)) return 1;
	return 0;
}

/**
 * Expression
 */
b32 expression(parser* parser)
{
	rval lhs = term(parser);

	if (lhs.hasError) return 1;

	token token = PeekToken(parser->lex);
	switch (token.type)
	{
		case Token_ShiftLeft:
		case Token_ShiftRight:
		case '+':
		case '*':
		case '|':
		case '&':
		case '/':
		case '%':
		{
			while (1)
			{
				if (binary_op(parser, &lhs, 0, ';')) return 1;
				if (PeekToken(parser->lex).type == ';') break;
				lhs.type = Rval_OnStack;
			}
		} break;
		case ';':
		{
			if (lhs.type == Rval_Constant)
			{
				if (lhs.token.type == Token_Number)
				{
					Log("  Constant number: %d\n", GetNumber(parser->lex, lhs.token));
				}
				else
				{
					Log("  Constant: %s\n", "unknown");
				}
			}
			else if (lhs.type == Rval_Identifier)
			{
				GetIdentifier(parser->lex, lhs.token, parser->scratchBuffer);
				Log("  Identifier: %s\n", parser->scratchBuffer);
			}
		} break;
		default:
		{
			fprintf(stderr, "Not implemented for now :P\n");
			assert(0);
		} break;
	}
	return 0;
}

b32 statement(parser* parser)
{
	token t = PeekToken(parser->lex);
	switch (t.type)
	{
		case Token_Extrn:
		{
			Log("Extrn%s\n", "");
			if (parseArgList(parser, ';')) return 1;
		} break;
		case Token_Auto:
		{
			Log("Auto%s\n", "");
			if (parseArgList(parser, ';')) return 1;
		} break;
		case '{': // Compound statement
		{
			while (GetNextToken(parser->lex).type != '}')
			{
				if (statement(parser)) return 1;
			}
			if (!ExpectToken(parser->lex, '}'))
			{
				fprintf(stderr, "Expected closing '}'\n"); // @Report
				return 1;
			}
		} break;
		case Token_Return:
		{
			Log("%s", "Return\n");
			if (GetNextToken(parser->lex).type != ';')
			{
				if (expression(parser)) return 1;
				if (!ExpectToken(parser->lex, ';'))
				{
					ReportCompilerError(parser->lex, t, "Expected ';' after return statement\n");
					return 1;
				}
			}
		} break;
		default:
		{
			fprintf(stderr, "%d %c\n", t.type, PeekToken(parser->lex).type);
			fprintf(stderr, "Not implemented for now :P\n");
			assert(0);
		} break;
	}
	return 0;
}

/**
 * Initial value, can be a literal or an identifier.
 * Used for initializing global variables.
 */
b32 ival(parser* parser, token token)
{
	b32 hasError = 0;
	switch (token.type)
	{
		case Token_Identifier: // @Robustness: Check if exists
		{
			GetIdentifier(parser->lex, token, parser->scratchBuffer);
			Log("  IVal Identifier: %s\n", parser->scratchBuffer);
		} break;
		case Token_Number:
		{
			u32 n = GetNumber(parser->lex, token);
			Log("  IVal Number: %d\n", n);
		} break;
		case Token_String:
		{
			Log("  IVal String: %s\n", "unknown");
		} break;
		case Token_Char:
		{
			Log("  IVal Char: %s\n", "unknown");
		} break;
		default:
		{
			ReportCompilerError(parser->lex, token, "Expected identifier or constant for initialization value:\n");
			hasError = 1;
		} break;
	}
	return hasError;
}

/**
 * Defines a global variable as a label.
 * A global variablel is accesible from anywhere.
 *
 * Ex:
 *		1. name 4;
 *		2. name[] 4, 5, name;
 *		3. name[3] 4, 5, name;
 *		4. name name2;
 *		5. name name2, name3;
 *		6. name;
 *
 * 2 and 5 are actually the same.
 * If no value is provided is initialized to 0.
 * If another identifier is provided, it will reference that variable
 * not copy it.
 */
b32 global(parser* parser)
{
	u32 vectorSize = 0;
	token sizeToken;
	token iterToken = PeekToken(parser->lex);

	if (iterToken.type == '[')
	{
		iterToken = GetNextToken(parser->lex);
		if (iterToken.type == Token_Number)
		{
			vectorSize = GetNumber(parser->lex, PeekToken(parser->lex));
			sizeToken = iterToken;
			GetNextToken(parser->lex);
		}
		if (!ExpectToken(parser->lex, ']'))
		{
			ReportCompilerError(parser->lex, iterToken, "Expected number or ']' for vector initialization:\n");
			return 1;
		}
		iterToken = GetNextToken(parser->lex);
	}
	Log("Global: %s\n", parser->scratchBuffer);


	u32 vectorCount = 0;
	while (iterToken.type != ';' && iterToken.type != Token_EOF)
	{
		if (ival(parser, iterToken)) return 1;
		iterToken = GetNextToken(parser->lex);
		if (vectorSize != 0)
		{
			vectorCount += 1;
		}
		if (iterToken.type != ';')
		{
			if (iterToken.type != ',')
			{
				ReportCompilerError(parser->lex, iterToken, "Expected ',' after value:\n");
				return 1;
			}
			iterToken = GetNextToken(parser->lex);
		}
	}

	if (!ExpectToken(parser->lex, ';'))
	{
		ReportCompilerError(parser->lex, iterToken, "Expected ';' at end of vector initialization:\n");
		return 1;
	}

	if (vectorSize > 0)
	{
		if (vectorSize < vectorCount)
		{
			ReportCompilerError(parser->lex, sizeToken, "Expected maximum %d vector elements, got %d:\n");
			/* fprintf(stderr, "", vectorSize, vectorCount); */
			return 1;
		}
	}
	return 0;
}


/**
 * A function has an identifier, and a list of parameters, that are passed on the
 * stack in reverse order.
 *
 * Must preserve registers, ebx, esi, edi, ebp, esp;
 * eax, ecx, edx are scratch registers.
 *
 * Ex: main(argc, argv, envp) { <statement> }
 */
b32 function(parser* parser)
{
	Log("Function Start: %s\n", parser->scratchBuffer);

	// ArgList  TODO: add to function socped declaraton and parameters.
	if (parseArgList(parser, ')')) return 1;

	if (!ExpectNextToken(parser->lex, '{'))
	{
		ReportCompilerError(parser->lex, PeekToken(parser->lex), "Expected '{':\n");
		return 1;
	}

	while (GetNextToken(parser->lex).type != '}')
	{
		if (statement(parser)) return 1;
	}

	return 0;
}

b32 parseArgList(parser* parser, type_token endToken)
{
	token startParenthesis = PeekToken(parser->lex);

	token iterToken = GetNextToken(parser->lex);
	while (iterToken.type != endToken)
	{
		if (iterToken.type == Token_Identifier)
		{
			GetIdentifier(parser->lex, iterToken, parser->scratchBuffer);
			Log("  arg: %s\n", parser->scratchBuffer); // TODO: Hash it an add it to a list.

			iterToken = GetNextToken(parser->lex);
			if (iterToken.type == ',')
			{
				iterToken = GetNextToken(parser->lex);
			}
			else if (iterToken.type == Token_Identifier)
			{
				ReportCompilerError(parser->lex, PeekToken(parser->lex), "Expected separation comma ',' between arguments:\n");
				return 1;
			}
			else if (iterToken.type != endToken)
			{
				ReportCompilerError(parser->lex, startParenthesis, "Expected closing parenthesis ')' for:\n");
				return 1;
			}
		}
		else
		{
			ReportCompilerError(parser->lex, iterToken, "Expected identifier:\n");
			return 1;
		}
	}
	return 0;
}

b32 definition(parser* parser)
{
	if (!ExpectToken(parser->lex, Token_Identifier))
	{
		ReportCompilerError(parser->lex, PeekToken(parser->lex), "[definition] Expected Identifier\n"); // TODO: Report compiler error
		return 1;
	}
	token ident = PeekToken(parser->lex);

	while (ident.type == Token_Identifier)
	{
		GetIdentifier(parser->lex, ident, parser->scratchBuffer);
		switch (GetNextToken(parser->lex).type)
		{
			case '(':
			{
				if (function(parser)) return 1;
			} break;

			case ';':
			{
				printf("%s:\n.zero 4\n", parser->scratchBuffer);
			} break;

			case Token_Number:
			case Token_Char:
			case Token_String:
			case Token_Identifier:
			case '[':
			{
				if (global(parser)) return 1;
			} break;
			default:
			{
				fprintf(stderr, "Unexpected token\n");
				return 1;
			} break;
		}
		GetIdentifier(parser->lex, ident, parser->extrn[parser->lenExtrn++]);
		ident = GetNextToken(parser->lex);
	}

	return 0;
}

b32 Parse(parser* parser)
{
	b32 hasError = definition(parser);;
	return hasError;
}
