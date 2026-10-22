
/*

rv32i assembler.

*/

/*

TODO

- Fix no newline bug
- Double check locations
- Adversarial tests
- Add a couple more instructions
- Generate binary
- Test on qemu
- Add rest of rv32i

*/

#include "../include/sting.h"

// TODO: perfect hash function generator. These should be generated.
// https://www.corsix.org/content/using-a-lookup-table-to-correct-an-approximation
#define MAX_INSTRUCTION_LEN 4
const char *instructions[] = {
  "addi",
};

#define MAX_REGISTER_LEN 4
const char *registers[32] = {
  "zero",
  "ra",
  "sp",
  "gp",
  "tp",
  "t0",
  "t1",
  "t2",
  "s0", // TODO: 8 need to also check for fp
  "s1",
  "a0",
  "a1",
  "a2",
  "a3",
  "a4",
  "a5",
  "a6",
  "a7",
  "s2",
  "s3",
  "s4",
  "s5",
  "s6",
  "s7",
  "s8",
  "s9",
  "s10",
  "s11",
  "t3",
  "t4",
  "t5",
  "t6",
};

i8 match_register(i8 *str, u32 length) {
  PANIC_IF(STATIC_ARRAY_LENGTH(registers) > 255, "Too many registers.");

  for (i8 i = 0; i < STATIC_ARRAY_LENGTH(registers); i++) {
    if (length != string_length(registers[i])) continue;
    if (string_equal(registers[i], str, length)) {
      return i;
    }
  }

  return -1;
}

i8 match_inst(i8 *str, u32 length) {
  PANIC_IF(STATIC_ARRAY_LENGTH(instructions) > 255, "Too many instructions.");

  for (i8 i = 0; i < STATIC_ARRAY_LENGTH(instructions); i++) {
    if (length != string_length(instructions[i])) continue;
    if (string_equal(instructions[i], str, length)) {
      return i;
    }
  }

  return -1;
}

//

#define NUM_INSTRUCTIONS 1
enum Token_Type {
  ADDI = 0,

  REGISTER,
  INTEGER_LITERAL,
  COMMA,
  LABEL,
  NEWLINE,
  SEMICOLON,
  COLON,
  UNKNOWN,
};

union Token_Info {
  u32 register_index;
  i32 integer_literal;
};

struct Token {

  enum Token_Type type;
  union Token_Info info;

  file_location location;
  u32 file_index;
  u32 bytes_used;

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

u32 tokens_append(file *f, Tokens *tokens, Token t) {
  CALL;
  PANIC_IF(tokens->tokens_used >= tokens->tokens_reserved,
           "Trying to append new token when reserved %d tokens, but used %d already.",
           tokens->tokens_reserved, tokens->tokens_used);
  tokens->tokens[tokens->tokens_used] = t;
  tokens->tokens_used++;

  file_location_move_until(&tokens->location, f, t.file_index, t.bytes_used);
  RETURN_VAL(t.bytes_used);
}

b8 is_asm_comment(u8 byte) {
  return byte == '#';
}

b8 is_alpha(u8 byte) {
  return (byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z');
}

Token tokenize_number(file *f, file_location *location, u32 byte_index) {
  CALL;
  const u32 start_index = byte_index;
  i8 byte = file_read_byte(f, byte_index);

  Token t = {
    .type = INTEGER_LITERAL,
    .info = 0,
    .location = *location,
    .file_index = byte_index,
    .bytes_used = 0,
  };

  i32 sign = 1;
  i32 number = 0;
  if (byte == '0') {
    i8 next_byte = peek_next_byte(f, byte_index);

    if (next_byte != 'x' && !is_number(next_byte)) {
      t.bytes_used = 1;
      RETURN_VAL(t);
    }

    PANIC_IF(next_byte != 'x' && is_number(next_byte),
             "Can't start an integer literal with a 0.");

    // TODO: shouldn't allow 0x

    byte_index += 2;
    while (is_hexadecimal(byte = file_read_byte(f, byte_index))) {
      number = number * 16 + ascii_hex_to_i32(byte);
      byte_index++;
    }

  } else {

    while (is_number(byte) || (byte_index == start_index && byte == '-')) {
      if (byte == '-') {
        i8 next_byte = peek_next_byte(f, byte_index);

        PANIC_IF(!is_number(next_byte) || next_byte == '0', "Expected a integer literal not 0, found '%c'.", next_byte);
        sign = -1;
        goto read_next_byte; // Could remove.
      }

      number = number * 10 + (byte & 0xf);
    read_next_byte:
      byte_index++;
      byte = file_read_byte(f, byte_index);
    }

  }
  t.info.integer_literal = number * sign; // probably theres a cool trick for this. would also be cool if compiler could figure it out.
  t.bytes_used = byte_index - start_index + 1; // TODO: check this.

  file_location_move_until(location, f, start_index, t.bytes_used);
  RETURN_VAL(t);
}

b8 is_inst_or_reg_or_label(i8 byte) {
  return is_alpha(byte) || is_number(byte) || byte == '_';
}

Token tokenize_inst_or_reg_or_label(file *f, file_location *location, u32 byte_index) {
  CALL;
  const u32 start_index = byte_index;
  const i8 first_byte = file_read_byte(f, byte_index);

  Token t = {
    .type = UNKNOWN,
    .info = 0,
    .location = *location,
    .file_index = byte_index,
    .bytes_used = 0
  };

  // Reg, Instruction, Label.

  i8 byte;
  while (is_inst_or_reg_or_label(byte = file_read_byte(f, byte_index))) {
    byte_index++;
  }

  const u32 length = byte_index - start_index;
  i8 register_index, inst_index;

  // This could probably be done better.
  if (length <= MAX_REGISTER_LEN && (register_index = match_register(f->data + start_index, length)) != -1) {
    t.info.register_index = register_index;
    t.type = REGISTER;
  } else if (length <= MAX_INSTRUCTION_LEN && (inst_index = match_inst(f->data + start_index, length)) != -1) {
    PANIC_IF(((u8)inst_index) >= NUM_INSTRUCTIONS,
             "%d matches as an instruction, but there are only %d instructions",
             inst_index, NUM_INSTRUCTIONS);
    t.type = inst_index;
  } else {
    // For simplicity, x0-31 cannot be labels.
    // This should be redone.
    t.type = LABEL;

    if (first_byte == 'x' && length <= 3) {
      if (length == 2) {
        i8 second = f->data[start_index + 1];
        u32 register_index = (second - '0');
        if (register_index >= 0 && register_index <= 9) {
          t.type = REGISTER;
          t.info.register_index = register_index;
        }
      } else if (length == 3) {
        i8 second = f->data[start_index + 1];
        i8 third = f->data[start_index + 2];
        u32 register_index = (second - '0') * 10 + (third - '0');

        if (register_index >= 0 && register_index <= 31) {
          t.type = REGISTER;
          t.info.register_index = register_index;
        }
      }
    }

  }

  t.bytes_used = length;
  file_location_move_until(location, f, start_index, t.bytes_used);
  RETURN_VAL(t);
}

void token_display(file *f, Token *t) {
  if (t->type < NUM_INSTRUCTIONS) {
    printf(RED("Instruction: %s\n"), instructions[t->type]);
    return;
  }

  switch (t->type) {
    case REGISTER: {
      printf(YELLOW("Register: x%d"), t->info.register_index);
    } break;
    case INTEGER_LITERAL: {
      printf(GREEN("Integer: " "%d"), t->info.integer_literal);
    } break;
    case COMMA: {
      printf("COMMA");
    } break;
    case LABEL: {
      printf(BLUE("Label: %.*s"), t->bytes_used, f->data + t->file_index);
    } break;
    case NEWLINE: {
      printf("NEWLINE");
    } break;
    case SEMICOLON: {
      printf("SEMICOLON");
    } break;
    case COLON: {
      printf("COLON");
    } break;
    default: PANIC("Trying to display unkown token %d.", t->type);
  }
  printf(" --- Location: line %d column %d\n", t->location.line, t->location.column);
}

void tokenize(file *f, Tokens *tokens) {
  CALL;
  tokens->tokens_used = 0;

  u32 token_length = 0;
  u32 byte_index = 0;
  for (; byte_index < f->bytes_used; byte_index += token_length) {
    token_length = 0;
    i8 byte = file_read_byte(f, byte_index);

    if (is_non_newline_whitespace(byte)) {
      token_length =
        skip_non_newline_whitespace(f, &tokens->location, byte_index);
    } else if (is_asm_comment(byte)) {
      token_length =
        num_bytes_until_matching_byte_or_eof(f, byte_index, '\n');
      file_location_add_new_line(&tokens->location);
    } else if (byte == '\n') {
      Token newline = {
        .bytes_used = 1,
        .file_index = byte_index,
        .type = NEWLINE,
      };
      token_length = tokens_append(f, tokens, newline);
    } else if (byte == ':') {
      Token colon = {
        .bytes_used = 1,
        .file_index = byte_index,
        .type = COLON,
      };
      token_length = tokens_append(f, tokens, colon);
    } else if (byte == ';') {
      Token semicolon = {
        .bytes_used = 1,
        .file_index = byte_index,
        .type = SEMICOLON,
      };
      token_length = tokens_append(f, tokens, semicolon);
    } else if (is_alpha(byte) || byte == '_') {
      token_length =
        tokens_append(f, tokens, tokenize_inst_or_reg_or_label(f, &tokens->location, byte_index));
    } else if (is_number(byte) || byte == '-') {
      token_length =
        tokens_append(f, tokens, tokenize_number(f, &tokens->location, byte_index));
    } else if (byte == ',') {
      Token comma = {
        .type = COMMA,
        .info = 0,
        .location = tokens->location,
        .file_index = byte_index,
        .bytes_used = 1,
      };

      token_length = tokens_append(f, tokens, comma);
      file_location_next_column(&tokens->location);
    }

    // TODO: Should be an error regardless of debug.
    PANIC_IF(token_length == 0, "Unknown character %c (%x).", byte, byte);
  }

  PANIC_IF(byte_index != f->bytes_used,
           "File size is %d, but ended at %d.",
           f->bytes_used, byte_index);

  RETURN;
}

i32 main(i32 num_args, char **args) {
  CALL;
  if (num_args <= 1) {
    printf(RED("Error") ": Expected assembly file names.\n");
    return 0;
  }

  // NOTE: Could calibrate these at runtime, and use a vla.
  const u32 memory_size = 32 KB;
  const u32 file_size   =  1 KB;
  const u32 tokens_size =  6 KB;
  u8 memory[memory_size];

  Arena arena = arena_setup(memory, memory_size);

  file f = file_setup(arena_alloc(&arena, file_size, 1), file_size, NULL);
  Tokens tokens =
    tokens_setup(arena_alloc(&arena, tokens_size, 1), tokens_size);

  for (u32 i = 1; i < num_args; i++) {

    file_change_path(&f, args[i]);
    file_read_into_memory(&f);

    if (file_failed(&f)) {
      printf(RED("Error") ": could not read %s.", f.path);
      file_close(&f);
      continue;
    }

    tokenize(&f, &tokens);

    for (u32 i = 0; i < tokens.tokens_used; i++) {
      token_display(&f, &tokens.tokens[i]);
    }

    file_close(&f);
  }

  RETURN_VAL(0);
}
