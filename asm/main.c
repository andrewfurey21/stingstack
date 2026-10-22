
/*

rv32i assembler.

*/

#include "../include/sting.h"

enum Token_Type {
  ADDI,
  REGISTER,
  INTEGER_LITERAL,
  COMMA,
  LABEL, // (name:)
};

union Token_Info {
  i32 register_index;
  i32 integer_literal;
};

struct Token {

  enum Token_Type type;
  union Token_Info info;

  u32 file_index;
  u32 bytes_used;
  file_location location;

};

typedef struct Token Token;

struct Tokens {
  Token *tokens;
  u32   tokens_reserved;
  u32   tokens_used;
  file_location location;
};

typedef struct Tokens Tokens;

Tokens tokens_setup(void *memory, u32 bytes_reserved) {
  return (Tokens) {
    .tokens = (Token *)memory,
    .tokens_reserved = bytes_reserved / sizeof(Token),
    .tokens_used = 0,
    .location.line = 1,
    .location.column = 0,
  };
}

u32 tokens_append(Tokens *tokens, Token t) {
  CALL;
  PANIC_IF(tokens->tokens_used >= tokens->tokens_reserved,
           "Trying to append new token when reserved %d tokens, but used %d already.",
           tokens->tokens_reserved, tokens->tokens_used);
  tokens->tokens[tokens->tokens_used] = t;
  RETURN_VAL(t.bytes_used);
}

b8 is_asm_comment(u8 byte) {
  return byte == '#';
}

b8 is_alpha(u8 byte) {
  return (byte >= 'A' && byte <= 'Z') || (byte >= 'a' || byte <= 'z');
}

Token tokenize_number(file *f, file_location *location, u32 byte_index) {
  b8 is_negative = file_read_byte(f, byte_index) == '_';

}

Token tokenize_inst_or_reg_or_label(file *f, file_location *location, u32 byte_index) {

}

void tokenize(file *f, Tokens *tokens) {
  CALL;
  tokens->tokens_used = 0;

  u32 token_length = 0;
  u32 byte_index = 0;
  for (; byte_index < f->bytes_used; byte_index += token_length) {
    i8 byte = file_read_byte(f, byte_index);

    if (is_whitespace(byte)) {
      token_length =
        skip_whitespace(f, &tokens->location, byte_index);
    } else if (is_asm_comment(byte)) {
      token_length =
        num_bytes_until_matching_byte_or_eof(f, byte_index, '\n');
    } else if (is_alpha(byte) || byte == '_') {
      token_length =
        tokens_append(tokens, tokenize_inst_or_reg_or_label(f, &tokens->location, byte_index));
    } else if (is_number(byte) || byte == '-') {
      token_length =
        tokens_append(tokens, tokenize_number(f, &tokens->location, byte_index));
    } else if (byte == ',') {
      Token comma = {
        .type = COMMA,
        .info = 0,
        .location = tokens->location,
      };

      token_length = tokens_append(tokens, comma);
    }

    // TODO: Should be an error regardless of debug.
    PANIC_IF(token_length == 0, "Unknown token. Token length is 0 when byte = %c (%x).", byte, byte);
  }

  PANIC_IF(byte_index != f->bytes_used,
           "File size is %d, but ended at %d.",
           f->bytes_used, byte_index);

  RETURN;
}

i32 main(i32 num_args, char **args) {
  CALL;
  if (num_args <= 1) {
    printf(RED("Error") ": Expected file names.");
    return 0;
  }

  // To avoid annoying memory issues, allocate one block (either on stack/heap), then free once at the end.
  // NOTE: Could calibrate these at runtime, and use a vla.
  const u32 memory_size = 32 KB;
  const u32 file_size   =  1 KB;
  const u32 tokens_size =  1 KB;
  u8 memory[memory_size];

  Arena arena = arena_setup(memory, memory_size);

  file f = file_setup(arena_alloc(&arena, file_size, 1), file_size, NULL);
  Tokens tokens =
    tokens_setup(arena_alloc(&arena, tokens_size, 1), tokens_size);

  for (u32 i = 0; i < num_args; i++) {

    file_change_path(&f, args[i]);
    file_read_into_memory(&f);

    if (file_failed(&f)) {
      printf(RED("Error") ": could not read %s.", f.path);
      file_close(&f);
      continue;
    }

    tokenize(&f, &tokens);

    file_close(&f);
  }

  RETURN_VAL(0);
}
