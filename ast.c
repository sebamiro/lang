typedef struct {
	char*	Data;
	u32		Size;
} String;

typedef struct {
	enum {
		Constant_String,
		Constant_Number,
		Constant_Char,
	} type;
	union {
		String	Str;
		u32		Num;
		u8		Char;
	};
} constant;

/**
 * Declaration list of external(global access) variables.
 */
typedef struct {
	enum {
		Parameter,
		Auto,
		Local,
		Extrn
	} type;
	u32			Len;
	u32			Cur;
	u32			Index;
	u64*		HashNames;
} ScopeDeclarations;

typedef struct {
	ScopeDeclarations DeclaredIdentifiers;
} ast_function;
