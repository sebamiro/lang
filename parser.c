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

typedef struct
{
	u32 len;
} ast_global;

typedef struct
{
	enum {
		Ast_Statement,
		Ast_BinaryOperation,
		Ast_Constant,
		Ast_Identifier,
	} Type;
} ast_value;

typedef handle_ast u32;
typedef struct
{
	handle_ast	len;
	handle_ast	cur;
	ast_value*	data;
} ast;


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
			fprintf(stderr, "[term] Not implemented for now :P (%c)\n", token.type);
			result.hasError = 1;
		} break;
	}
	return result;
}

enum binary_precedence
{
	Prec_BinCmp,
	Prec_BinShift,
	Prec_BinUnary,
	Prec_BinAdd,
	Prec_BinDiv,
	Prec_BinMul,
};

void printOperation(type_token t)
{
	switch (t)
	{
		case Token_ShiftLeft:
		{
			printf("mov edx, ecx\n");
			printf("mov ecx, eax\n");
			printf("sal edx, cl\n");
			printf("mov eax, edx\n");
		} break;
		case Token_ShiftRight:
		{
			printf("mov edx, ecx\n");
			printf("mov ecx, eax\n");
			printf("mov eax, edx\n");
			printf("sar eax, ecx\n");
		} break;
		case '+':
		{
			printf("add eax, ecx\n");
		} break;
		case '|':
		{
			printf("or eax, ecx\n");
		} break;
		case '&':
		{
			printf("and eax, ecx\n");
		} break;
		case '*':
		{
			printf("imul eax, ecx\n");
		} break;
		case '/':
		{
			printf("push eax\n");
			printf("mov eax, ecx\n");
			printf("pop ecx\n");
			printf("idiv eax, ecx\n");
		} break;
		case '%':
		{
			printf("push eax\n");
			printf("mov eax, ecx\n");
			printf("pop ecx\n");
			printf("idiv eax, ecx\n");
			printf("mov eax, edx\n");
		} break;
		default:
		{
			fprintf(stderr, "Should not be called with: %c\n", t);
			assert(0);
		} break;
	}
}

u32 getPrecedence(type_token t)
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
		case ';': return 0;
		default:
		{
			fprintf(stderr, "[getPrecedence] Inivalid toke: %c\n", t);
			assert(0);
		} break;
	}
	return 0;
}

b32 expression(parser* parser);
b32 binary_op(parser* parser, rval* lhs, u32 curPrecedence)
{
	token operation = PeekToken(parser->lex);
	u32 opPrecedence = getPrecedence(operation.type);

	if (operation.type == ';' || opPrecedence < curPrecedence)
	{
		if (lhs->type == Rval_Constant)
		{
			printf("mov eax, %d\n", GetNumber(parser->lex, lhs->token));
		}
		else if (lhs->type == Rval_Identifier)
		{
			GetIdentifier(parser->lex, lhs->token, parser->scratchBuffer);
			printf("mov eax, %s\n", parser->scratchBuffer);
		}
		return 0;
	}

	GetNextToken(parser->lex);
	rval rhs = term(parser);
	if (rhs.hasError) return 1;
	if (binary_op(parser, &rhs, opPrecedence)) return 1;
	if (lhs->type == Rval_Constant)
	{
		printf("mov ecx, %d\n", GetNumber(parser->lex, lhs->token));
	}
	else if (lhs->type == Rval_Identifier)
	{
		GetIdentifier(parser->lex, lhs->token, parser->scratchBuffer);
		printf("mov ecx, %s\n", parser->scratchBuffer);
	}
	else if (lhs->type == Rval_OnStack)
	{
		printf("pop ecx\n");
	}
	printOperation(operation.type);

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
				if (binary_op(parser, &lhs, 0)) return 1;
				if (PeekToken(parser->lex).type == ';') break;
				printf("push eax\n");
				lhs.type = Rval_OnStack;
			}
		} break;
		case ';':
		{
			// TODO: Internal definition(?)
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
	switch (PeekToken(parser->lex).type)
	{
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
			if (GetNextToken(parser->lex).type != ';')
			{
				if (expression(parser)) return 1;
				if (!ExpectToken(parser->lex, ';'))
				{
					fprintf(stderr, "Expected ';' after 'return' statement\n"); // @Report
					return 1;
				}
			}
			printf("leave\n");
			printf("ret\n");
		} break;
		default:
		{
			fprintf(stderr, "%d %c\n", PeekToken(parser->lex).type, PeekToken(parser->lex).type);
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
			printf("  .long %s\n", parser->scratchBuffer);
		} break;
		case Token_Number:
		{
			u32 n = GetNumber(parser->lex, token);
			printf("  .long %d\n", n);
		} break;
		case Token_String:
		{
			/* u32 n = GetNumber(parser->lex, token); */
			/* printf("  .string \"%s\"\n", n); */
			fprintf(stderr, "To implement: ival String\n");
			hasError = 1;
		} break;
		case Token_Char:
		{
			/* u32 n = GetNumber(parser->lex, token); */
			/* printf("  .string \"%s\"\n", n); */
			fprintf(stderr, "To implement: ival Char\n");
			hasError = 1;
		} break;
		default:
		{
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
	token iterToken = PeekToken(parser->lex);

	if (iterToken.type == '[')
	{
		iterToken = GetNextToken(parser->lex);
		switch (iterToken.type)
		{
			case Token_Number:
			{
				vectorSize = GetNumber(parser->lex, PeekToken(parser->lex));
				GetNextToken(parser->lex);
			} break;
			case ']':
			{
				// noop
			} break;
			default:
			{
				fprintf(stderr, "Expected number or ']' for vector initialization\n");
				return 1;
			} break;
		}
		if (!ExpectToken(parser->lex, ']'))
		{
			fprintf(stderr, "Expected ']' for vector initialization\n");
			return 1;
		}
		iterToken = GetNextToken(parser->lex);
	}
	printf("%s: # Global variable declaration\n", parser->scratchBuffer);

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
				fprintf(stderr, "Expected ','\n");
				return 1;
			}
			iterToken = GetNextToken(parser->lex);
		}
	}

	if (!ExpectToken(parser->lex, ';'))
	{
		fprintf(stderr, "Expected ';' at end of vector initialization\n");
		return 1;
	}

	if (vectorSize > 0)
	{
		if (vectorSize < vectorCount)
		{
			fprintf(stderr, "Expected maximum %d vector elements, got %d\n", vectorSize, vectorCount);
			return 1;
		}
		printf("  .zero %d\n", (vectorSize - vectorCount) * 4);
	}
	return 0;
}

/**
 * A function has an identifier, and a list of parameters, that are pass on the
 * stack in reverse order.
 *
 * Must preserve registers, ebx, esi, edi, ebp, esp;
 * eax, ecx, edx are scratch registers.
 *
 * Ex: main(argc, argv, envp) { <statement> }
 */
b32 function(parser* parser)
{
	// Function start
	printf(".globl %s # Start Function\n", parser->scratchBuffer);
	printf("%s:\n", parser->scratchBuffer);
	printf(".long \"%s\" + 4\n", parser->scratchBuffer);
	printf("enter 0, 0\n"); // NOTE: Check why this is for the stack variables.

	// TODO: Add arg list to the stack. Check extrn arguments??
	// If so shuold save them on a global, or here.
	// Since no types, only the arg count is enough I guess.
	if (!ExpectNextToken(parser->lex, ')'))
	{
		ReportCompilerError(parser->lex, PeekToken(parser->lex), "Expected empty paramteter list"); // TODO: Report compiler error
		return 1;
	}
	GetNextToken(parser->lex);

	if (statement(parser))
	{
		fprintf(stderr, "Statement error\n");
		return 1;
	}

	// TODO: Clear auto array; set len to zero(?)
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
	printf(".intel_syntax noprefix\n");
	printf(".text\n");
	printf(".text\n");
	b32 hasError = definition(parser);;
	return hasError;
}
