typedef enum
{
	Token_None = -1,
	Token_EOF,

	Token_OpenSquareBracket = '[',
	Token_CloseSquareBracket = ']',
	Token_OpenBracket = '{',
	Token_CloseBracket = '}',
	Token_OpenParenthesis = '(',
	Token_CloseParenthesis = ')',
	Token_Plus = '+',
	Token_Minus = '-',
	Token_Mult = '*',
	Token_Div = '/',
	Token_Mod = '%',
	Token_And = '&',
	Token_Or = '|',
	Token_Comma = ',',
	Token_SemiColon = ';',
	Token_Colon = ':',
	Token_Iterrogation = '?',
	Token_Less = '<',
	Token_Greater = '>',

	Token_Increment = 128,
	Token_Decrement,

	Token_Identifier,
	Token_Number,
	Token_Char,
	Token_String,

	/* Binary */
	Token_Equal,
	Token_NotEqual,
	Token_GreaterEqual,
	Token_LessEqual,
	Token_ShiftLeft,
	Token_ShiftRight,

	/* Assign */
	Token_AssignOr,
	Token_AssignAnd,
	Token_AssignEqual,
	Token_AssignNotEqual,
	Token_AssignLess,
	Token_AssignLessEqual,
	Token_AssignGreater,
	Token_AssignGreaterEqual,
	Token_AssignShiftLeft,
	Token_AssignShiftRight,
	Token_AssignPlus,
	Token_AssignMinus,
	Token_AssignModulo,
	Token_AssignMult,
	Token_AssignDiv,

	/* Keywords */
	Token_Auto,
	Token_Extrn,
	Token_Case,
	Token_If,
	Token_Else,
	Token_While,
	Token_Switch,
	Token_Goto,
	Token_Return,

} type_token;

typedef struct {
	type_token	type;
	u32	start;
} token;

typedef struct {
	u8*		buf;
	u32		len;
	u32		cur;
	token	curToken;
	u32		posCurToken;
} lexer;
