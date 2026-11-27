/*

rv32i assembler.

*/

#include "../include/sting.h"


// this should be somewhere in utilities. make include utilities.
b8 is_alpha(u8 byte) {
  return (byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z');
}

enum Token_Type : u32 {

  TT_IDENTIFIER = 128,

  TT_INTEGER_LITERAL,
  TT_STRING_LITERAL,

  TT_LOCATION_COUNTER,

  TT_AT_IDENTIFIER,
  TT_PERC_IDENTIFIER,

  TT_LEFT_SHIFT,
  TT_RIGHT_SHIFT,

  TT_UNKNOWN,

  TT_COMMA = ',',
  TT_COLON = ':',
  TT_SEMICOLON = ';',
  TT_LEFT_PAREN = '(',
  TT_RIGHT_PAREN = ')',
  TT_NEWLINE = '\n',

  TT_PLUS = '+',
  TT_MINUS = '-',
  TT_STAR = '*',
  TT_SLASH = '/',
  TT_AND = '&',
  TT_OR = '|',
  TT_XOR = '^',
  TT_NOT = '~',
  TT_MOD = '%',

};

typedef enum Token_Type Token_Type;

union Token_Info {

  u32 instruction_index;
  u32 register_index;
  u32 directive_index;
  u32 at_index;
  u32 perc_index;

  i32 integer_literal;
  i32 section_location;

};

typedef union Token_Info Token_Info;

struct Token {

  enum Token_Type type;
  union Token_Info info;
  file_location location;

  u32 byte_index;
  u32 bytes_used;

};

typedef struct Token Token;

struct Tokens {
  Token *tokens;
  u32   reserved;
  u32   used;
  file_location current_location;
};

typedef struct Tokens Tokens;

Tokens tokens_setup(Token *memory, u32 bytes_reserved) {
  PANIC_IF((uintptr_t)memory % alignof(Token) != 0, "Incorrect alignment for token memory.");
  return (Tokens) {
    .tokens = memory,
    .reserved = bytes_reserved / sizeof(Token),
    .used = 0,
    .current_location.line = 1,
    .current_location.column = 1,
  };
}

Token tokens_get(Tokens *tokens, u32 token_index) {
  PANIC_IF(token_index >= tokens->used,
          "Accessing token at index %d when length is %d.",
           token_index, tokens->used);

  return tokens->tokens[token_index];
}

// returns number of bytes Token t consumed.
u32 tokens_append(file *f, Tokens *tokens, Token t) {
  PANIC_IF(tokens->used >= tokens->reserved,
           "Used %d tokens, but only allocated %d",
           tokens->used, tokens->reserved);

  tokens->tokens[tokens->used] = t;
  tokens->tokens[tokens->used].location = tokens->current_location;

  tokens->used++;
  file_location_move_until(&tokens->current_location, f, t.byte_index, t.bytes_used);
  return t.bytes_used;
}

// Try produce location counter, identifier, integer, @type, %reloc, modulo
// Else, return unknown token with 0 length.
Token tokenize_loc_ident_int_at_perc_mod(file *f, u32 byte_index) {

}

Token tokenize_string(file *f, u32 byte_index) {

}

Token tokenize_shift(file *f, u32 byte_index) {

}

#define ONE_BYTE_TOKEN(_byte, _index, _location) \
  (Token) { .type = _byte, .location = _location, .byte_index = _index, .bytes_used = 1 }

void tokenize(file *f, Tokens *tokens) {
  tokens->tokens = 0;

  u32 token_length = 0;
  u32 byte_index = 0;

  while (byte_index < f->bytes_used) {
    i8 current_byte = file_read_byte(f, byte_index);

    switch (current_byte) {
      // Excludes '.' because may be part of label. Similar with %.
      // If there's an error, token_length = 0 & token_type = TT_UNKNOWN
      case ',': case ':':
      case ';': case '(':
      case ')': case '\n':
      case '+': case '-':
      case '*': case '/':
      case '&': case '|':
      case '^': case '~':
        token_length =
          tokens_append(f,
                        tokens,
                        ONE_BYTE_TOKEN(current_byte, byte_index, tokens->current_location));
        break;
      case '>':
      case '<':
        token_length = tokens_append(f, tokens, tokenize_shift(f, byte_index));
        break;
      case '#':
        token_length = num_bytes_until_matching_byte_or_eof(f, byte_index, '\n');
        file_location_move_until(&tokens->current_location, f, byte_index, token_length);
        break;
      case '"':
        token_length = tokens_append(f, tokens, tokenize_string(f, byte_index));
        break;
      default: {
        Token t = tokenize_loc_ident_int_at_perc_mod(f, byte_index);
        token_length = tokens_append(f, tokens, t);
      }
    }

    byte_index += token_length;
    PANIC_IF(token_length == 0, "Unknown character %c at line %d:%d.",
             current_byte,
             tokens->current_location.line,
             tokens->current_location.column);
  }

  PANIC_IF(byte_index != f->bytes_used,
           "File size is %d, but ended at %d.",
           f->bytes_used, byte_index);
}

void tokens_display(Tokens *tokens) {

}

i32 main(i32 argc, char **argv) {

  i32 num_files = argc - 1;
  PANIC_IF(num_files < 2 || num_files % 2 != 0,
           "Expected assembly file names followed by binary name.");

  // TODO: calibrate these at runtime
  const u32 file_size   =  2 KB;
  const u32 binary_size =  2 KB;
  const u32 tokens_size =  8 KB;
  const u32 memory_size = 32 KB;
  u8 memory[memory_size];

  Arena arena = arena_setup(memory, memory_size);

  file assembly = file_setup(arena_alloc(&arena, file_size, 1), file_size, NULL);
  Tokens tokens = tokens_setup(arena_alloc(&arena, tokens_size, alignof(Token)), tokens_size);

  for (u32 i = 0; i < num_files; i += 2) {
    file_change_path(&assembly, argv[i + 1]);

    file_read_into_memory(&assembly);

    if (file_failed(&assembly)) {
      NOTE("Could not read %s.", assembly.path);
      file_close(&assembly);
      continue;
    }

    tokenize(&assembly, &tokens);
    tokens_display(&tokens);

  }
}
