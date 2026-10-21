
#ifndef CC_LEXER_H
#define CC_LEXER_H

#include "../utils/utils.h"
#include "file.h"
#include "lexing.h"

enum TokenType: u8 {

    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,

    TOKEN_STRING_LITERAL,
    TOKEN_CHAR_LITERAL,

    TOKEN_IDENTIFIER,

    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_ARROW,
    TOKEN_COLON,
    TOKEN_QUESTION,
    TOKEN_ELLIPSIS,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_INC,
    TOKEN_DEC,

    TOKEN_AMPERSAND,
    TOKEN_PIPE,
    TOKEN_CARET,
    TOKEN_TILDE,
    TOKEN_LEFT_SHIFT,
    TOKEN_RIGHT_SHIFT,

    TOKEN_LOGICAL_AND,
    TOKEN_LOGICAL_OR,
    TOKEN_LOGICAL_NOT,

    TOKEN_LESS_THAN,
    TOKEN_GREATER_THAN,
    TOKEN_LESS_OR_EQUAL,
    TOKEN_GREATER_OR_EQUAL,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,

    TOKEN_ASSIGN,
    TOKEN_PLUS_ASSIGN,
    TOKEN_MINUS_ASSIGN,
    TOKEN_STAR_ASSIGN,
    TOKEN_SLASH_ASSIGN,
    TOKEN_PERCENT_ASSIGN,
    TOKEN_AMPERSAND_ASSIGN,
    TOKEN_PIPE_ASSIGN,
    TOKEN_CARET_ASSIGN,
    TOKEN_LEFT_SHIFT_ASSIGN,
    TOKEN_RIGHT_SHIFT_ASSIGN,

    TOKEN_HASH,
    TOKEN_DOUBLE_HASH,
    TOKEN_PP_LINE,

    TOKEN_CHAR,
    TOKEN_DOUBLE,
    TOKEN_FLOAT,
    TOKEN_INT,
    TOKEN_LONG,
    TOKEN_SHORT,
    TOKEN_SIGNED,
    TOKEN_UNSIGNED,
    TOKEN_VOID,

    TOKEN_AUTO,
    TOKEN_BREAK,
    TOKEN_CASE,
    TOKEN_CONST,
    TOKEN_CONTINUE,
    TOKEN_DEFAULT,
    TOKEN_DO,
    TOKEN_ELSE,
    TOKEN_ENUM,
    TOKEN_EXTERN,
    TOKEN_FOR,
    TOKEN_GOTO,
    TOKEN_IF,
    TOKEN_INLINE,
    TOKEN_REGISTER,
    TOKEN_RESTRICT,
    TOKEN_RETURN,
    TOKEN_SIZEOF,
    TOKEN_STATIC,
    TOKEN_STRUCT,
    TOKEN_SWITCH,
    TOKEN_TYPEDEF,
    TOKEN_UNION,
    TOKEN_VOLATILE,
    TOKEN_WHILE,

    TOKEN_ALIGNAS,
    TOKEN_ALIGNOF,

    // TOKEN_ASSEMBLY,
    // ATTRIBUTE, ALWAYS_INLINE

    TOKEN_EOF,
    TOKEN_ERROR,
};

typedef enum TokenType TokenType;

typedef struct {
  TokenType type;
  u32 length;
  u32 file_index;
  const char *error_msg;
  file_location location; // location of token.
} Token;

typedef struct {
  Token *tokens;
  file_read *f;
  file_location location; // location of cursor.
  u32 num_tokens;
  u32 capacity;
  error tokenizing_error;
} Tokens;

void tokens_setup(Tokens *tokens, file_read *f, void *buffer, u32 capacity);
void tokenize(Tokens *tokens);
void token_build(Tokens *tokens,
                 TokenType type, u32 file_read_index, u32 length,
                 const char *error_msg);

// Token generation
u32 skip_whitespace(file_read *f, file_location *location, u32 start_of_whitespace);
u32 skip_if_comment(Tokens *tokens, u32 possible_start_of_comment);
u32 tokenize_string_literal(Tokens *tokens, u32 start_of_token_index);

u32 tokens_display_errors(Tokens *tokens);
u32 token_display_error(Tokens *tokens, u32 token_index);

#endif
