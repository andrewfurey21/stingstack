
/*

rv32i assembler.

*/

/*

TODO:

- add file locations to instructions
- generate the binary for each implemented instruction
- fix labels
- generate correctly with elf format
- testing to catch parsing errors, binary errors + compare with gas.
- Add rest of rv32i (and pseudoinstructions), look at baremetal
- benchmarks?

*/

/*

Possible errors to check:
- Labels with the same name. also, labels can't start with numbers.
- 0x parses as hexadecimal 0.
- unknown instruction
- multiple successive labels should point to the same place.
- since offsets are parsed in their own case, test lw at the end of test.s.

*/

#include "../include/sting.h"

// TODO: perfect hash function generator. These should be generated.
// https://www.corsix.org/content/using-a-lookup-table-to-correct-an-approximation

#define MAX_INSTRUCTION_LEN 5
#define NUM_INSTRUCTIONS 9
#define MAX_REGISTER_LEN 4
#define MAX_NUM_LABELS 10
#define MAX_NUM_LATE_LABELS 32
#define MAX_NUM_ARGS_PER_INSTRUCTION 3
// NOTE: Maintain order between Instruction_Name and instruction_strings and available_instructions
static const char *instruction_strings[NUM_INSTRUCTIONS] = {
  "addi", "lw", "beq", "sra", "slli", "sh", "jal", "auipc", "ecall"
};

enum Instruction_Name {
  INST_ADDI, INST_LW, INST_BEQ, INST_SRA, INST_SLLI, INST_SH, INST_JAL, INST_AUIPC, INST_ECALL
};

enum Arg_Type {
  ARG_OFFSET = 0,
  ARG_REGISTER,
  ARG_INTEGER_LITERAL,
  ARG_LABEL,
  ARG_INSTRUCTION
};

union Arg_Info {
  i32 offset;
  u32 register_index;
  i32 integer_literal;
  u32 label_index;
  u32 instruction_index;
};

struct Instruction_Arg {
  enum Arg_Type type;
  union Arg_Info info;
};

enum Instruction_Type { INST_TYPE_INSTRUCTION, INST_TYPE_LABEL };

// TODO: add file location (copy from instruction token)
struct Instruction {
  file_location location;
  enum Instruction_Type type;
  enum Instruction_Name name;
  u32 args_used;
  struct Instruction_Arg args[MAX_NUM_ARGS_PER_INSTRUCTION];
};

typedef struct Instruction Instruction;

#define INST(_name, num_args, type0, type1, type2)                   \
  (struct Instruction) {                                             \
    .type = INST_TYPE_INSTRUCTION,                                   \
    .name = _name,                                                   \
    .args_used = num_args,                                           \
    .args = { {.type = type0 }, {.type = type1 }, {.type = type2 } } \
  }

static const Instruction available_instructions[NUM_INSTRUCTIONS] = {
  INST(INST_ADDI, 3, ARG_REGISTER, ARG_REGISTER, ARG_INTEGER_LITERAL),
  INST(INST_LW, 3, ARG_REGISTER, ARG_INTEGER_LITERAL, ARG_REGISTER),

  INST(INST_BEQ, 3, ARG_REGISTER, ARG_REGISTER, ARG_LABEL),
  INST(INST_SRA, 3, ARG_REGISTER, ARG_REGISTER, ARG_REGISTER),
  INST(INST_SLLI, 3, ARG_REGISTER, ARG_REGISTER, ARG_INTEGER_LITERAL),
  INST(INST_SH, 3, ARG_REGISTER, ARG_INTEGER_LITERAL, ARG_REGISTER),
  INST(INST_JAL, 2, ARG_REGISTER, ARG_LABEL, 0),
  INST(INST_AUIPC, 2, ARG_REGISTER, ARG_INTEGER_LITERAL, 0),
  INST(INST_ECALL, 0, 0, 0, 0),
};


void panic_if_bad_instruction(Instruction good, Instruction bad) {
  PANIC_IF(good.args_used != bad.args_used,
           "Expected %d args used, but got %d on line %d, %d",
           good.args_used, bad.args_used, bad.location.line, bad.location.column);
  for (u32 i = 0; i < good.args_used; i++) {
    PANIC_IF(good.args[i].type != bad.args[i].type, "Expected arg %d type %d, but got %d on line %d:%d.",
             i, good.args[i].type, bad.args[i].type, bad.location.line, bad.location.column);
  }
}


static const char *integer_registers[32] = {
  "zero",
  "ra",
  "sp",
  "gp",
  "tp",
  "t0",
  "t1",
  "t2",
  "s0",
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
  PANIC_IF(STATIC_ARRAY_LENGTH(integer_registers) > 255, "Too many registers.");

  for (i8 i = 0; i < STATIC_ARRAY_LENGTH(integer_registers); i++) {
    if (length != string_length(integer_registers[i])) continue;
    if (string_equal(integer_registers[i], str, length)) {
      return i;
    }

    if (i == 8 && string_equal("fp", str, length)) {
      return i;
    }
  }

  return -1;
}

i8 match_inst_name(i8 *str, u32 length) {
  PANIC_IF(STATIC_ARRAY_LENGTH(instruction_strings) > 255, "Too many instructions.");

  for (i8 i = 0; i < STATIC_ARRAY_LENGTH(instruction_strings); i++) {
    if (length != string_length(instruction_strings[i])) continue;
    if (string_equal(instruction_strings[i], str, length)) {
      return i;
    }
  }

  return -1;
}

enum Token_Type : u32 {
  TT_INSTRUCTION = 0,

  TT_REGISTER,
  TT_INTEGER_LITERAL,
  TT_LABEL,

  TT_COMMA,
  TT_COLON,
  TT_NEWLINE,
  TT_SEMICOLON,
  TT_LEFT_BRACE,
  TT_RIGHT_BRACE,

  TT_UNKNOWN,
};

b8 skippable_token(enum Token_Type type) {
  return type == TT_SEMICOLON || type == TT_NEWLINE;
}

union Token_Info {
  u32 register_index;
  i32 integer_literal;
  u32 label_index;
  u32 instruction_index;
};

void instruction_append_arg(Instruction *inst, enum Arg_Type type, u32 data) {
  CALL;
  PANIC_IF(sizeof(data) != sizeof(union Arg_Info),
           "Internal: data does not match size of Arg_Info.");

  PANIC_IF(inst->args_used >= MAX_NUM_ARGS_PER_INSTRUCTION,
           "Maxinum number of args exceeded.");

  inst->args[inst->args_used].type = type;
  inst->args[inst->args_used].info = (union Arg_Info)data;
  inst->args_used++;

  RETURN;
}

void match_instruction_and_write(Instruction *instruction, file *b) {
  CALL;

  PANIC_IF(instruction->type != INST_TYPE_INSTRUCTION &&
           instruction->type != INST_TYPE_LABEL, "Expected label or instruction.");

  if (instruction->type == INST_TYPE_LABEL) {
    printf("label");
    RETURN;
  }

  printf("%s ", instruction_strings[instruction->name]);
  for (u32 i = 0; i < instruction->args_used; i++) {
    if (instruction->args[i].type == ARG_REGISTER) {
      printf("%s", integer_registers[instruction->args[i].info.register_index]);
    }
    else {
      printf("%d", instruction->args[i].info.integer_literal);
    }
    printf(", ");
  }
  printf("\n");

  u32 binary = 0;

  NOTE("Checking %d", instruction->type);
  panic_if_bad_instruction(available_instructions[instruction->name], *instruction);

  switch (instruction->name) {
    // - I [31 imm 20] [19 rs1 15] [14 funct3 12] [11 rd 7] [6 op 0]
    case INST_ADDI: {
    } break;
    default: PANIC("Unimplemented instruction id: %d", instruction->name);
  }

  file_append_bytes(b, (i8 *)(&binary), sizeof(binary));
  RETURN;
}

void fix_label_address(file *b, u32 instruction, u32 label_address) {
  NOTE("Fixing instrution %d label %d.", instruction, label_address);
}

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
    .location.column = 1,
  };
}

Token tokens_get(Tokens *tokens, u32 token_index) {
  CALL;
  PANIC_IF(token_index >= tokens->tokens_used, "Accessing token that is out of bounds.");
  RETURN_VAL(tokens->tokens[token_index]);
}

u32 tokens_append(file *f, Tokens *tokens, Token t) {
  CALL;
  PANIC_IF(tokens->tokens_used >= tokens->tokens_reserved,
           "Trying to append new token when reserved %d tokens, but used %d already.",
           tokens->tokens_reserved, tokens->tokens_used);
  tokens->tokens[tokens->tokens_used] = t;
  tokens->tokens[tokens->tokens_used].location = tokens->location;
  tokens->tokens_used++;

  file_location_move_until(&tokens->location, f, t.byte_index, t.bytes_used);
  RETURN_VAL(t.bytes_used);
}

b8 is_asm_comment(u8 byte) {
  return byte == '#';
}

b8 is_alpha(u8 byte) {
  return (byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z');
}

Token tokenize_number(file *f, u32 byte_index) {
  CALL;
  const u32 start_index = byte_index;
  i8 byte = file_read_byte(f, byte_index);

  Token t = {
    .type = TT_INTEGER_LITERAL,
    .info = 0,
    .byte_index = byte_index,
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
  t.bytes_used = byte_index - start_index;

  RETURN_VAL(t);
}

b8 is_inst_or_reg_or_label(i8 byte) {
  return is_alpha(byte) || is_number(byte) || byte == '_';
}

Token tokenize_inst_or_reg_or_label(file *f, u32 byte_index) {
  CALL;
  const u32 start_index = byte_index;
  const i8 first_byte = file_read_byte(f, byte_index);

  Token t = {
    .type = TT_UNKNOWN,
    .info = 0,
    .byte_index = byte_index,
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
    t.type = TT_REGISTER;
  } else if (length <= MAX_INSTRUCTION_LEN && (inst_index = match_inst_name(f->data + start_index, length)) != -1) {
    PANIC_IF(((u8)inst_index) >= NUM_INSTRUCTIONS,
             "%d matches as an instruction, but there are only %d instructions",
             inst_index, NUM_INSTRUCTIONS);
    t.type = TT_INSTRUCTION;
    t.info.instruction_index = inst_index;
  } else {
    // For simplicity, x0-31 cannot be labels.
    // This should be redone.
    t.type = TT_LABEL;

    if (first_byte == 'x' && length <= 3) {
      if (length == 2) {
        i8 second = f->data[start_index + 1];
        u32 register_index = (second - '0');
        if (register_index >= 0 && register_index <= 9) {
          t.type = TT_REGISTER;
          t.info.register_index = register_index;
        }
      } else if (length == 3) {
        i8 second = f->data[start_index + 1];
        i8 third = f->data[start_index + 2];
        u32 register_index = (second - '0') * 10 + (third - '0');

        if (register_index >= 0 && register_index <= 31) {
          t.type = TT_REGISTER;
          t.info.register_index = register_index;
        }
      }
    }

  }

  t.bytes_used = length;
  RETURN_VAL(t);
}

void token_display(file *f, Token *t) {
  if (t->type == TT_INSTRUCTION) {
    printf(RED("Instruction: %s\n"), instruction_strings[t->info.instruction_index]);
    return;
  }

  switch (t->type) {
    case TT_REGISTER: {
      printf(YELLOW("Register: x%d"), t->info.register_index);
    } break;
    case TT_INTEGER_LITERAL: {
      printf(GREEN("Integer: " "%d"), t->info.integer_literal);
    } break;
    case TT_COMMA: {
      printf("COMMA");
    } break;
    case TT_LABEL: {
      printf(BLUE("Label: %.*s"), t->bytes_used, f->data + t->byte_index);
    } break;
    case TT_NEWLINE: {
      printf("NEWLINE");
    } break;
    case TT_SEMICOLON: {
      printf("SEMICOLON");
    } break;
    case TT_COLON: {
      printf("COLON");
    } break;
    case TT_LEFT_BRACE: {
      printf("(");
    } break;
    case TT_RIGHT_BRACE: {
      printf(")");
    } break;
    default: PANIC("Trying to display unkown token %d.", t->type);
  }
  printf(" --- Location: line %d column %d\n", t->location.line, t->location.column);
}

void tokens_display(file *f, Tokens *tokens) {
  printf("Num Tokens: %d\n", tokens->tokens_used);
  for (u32 i = 0; i < tokens->tokens_used; i++) {
    token_display(f, &tokens->tokens[i]);
  }
}

void tokenize(file *f, Tokens *tokens) {
  CALL;
  tokens->tokens_used = 0;

  u32 token_length = 0;
  u32 byte_index = 0;
  for (; byte_index < f->bytes_used; byte_index += token_length) {
    token_length = 0;
    i8 byte = file_read_byte(f, byte_index);

    if (byte == '\n') {
      Token newline = {
        .bytes_used = 1,
        .byte_index = byte_index,
        .type = TT_NEWLINE,
      };
      token_length = tokens_append(f, tokens, newline);
    } else if(byte == '(' || byte == ')') {
      Token newline = {
        .bytes_used = 1,
        .byte_index = byte_index,
        .type = byte == '(' ? TT_LEFT_BRACE : TT_RIGHT_BRACE,
      };
      token_length = tokens_append(f, tokens, newline);

    } else if (is_non_newline_whitespace(byte)) {
      token_length =
        skip_non_newline_whitespace(f, &tokens->location, byte_index);
    } else if (is_asm_comment(byte)) {
      token_length =
        num_bytes_until_matching_byte_or_eof(f, byte_index, '\n');
      file_location_move_until(&tokens->location, f, byte_index, token_length);
    } else if (byte == ':') {
      Token colon = {
        .bytes_used = 1,
        .byte_index = byte_index,
        .type = TT_COLON,
      };
      token_length = tokens_append(f, tokens, colon);
    } else if (byte == ';') {
      Token semicolon = {
        .bytes_used = 1,
        .byte_index = byte_index,
        .type = TT_SEMICOLON,
      };
      token_length = tokens_append(f, tokens, semicolon);
    } else if (is_alpha(byte) || byte == '_') {
      token_length =
        tokens_append(f, tokens, tokenize_inst_or_reg_or_label(f, byte_index));
    } else if (is_number(byte) || byte == '-') {
      token_length =
        tokens_append(f, tokens, tokenize_number(f, byte_index));
    } else if (byte == ',') {
      Token comma = {
        .type = TT_COMMA,
        .byte_index = byte_index,
        .bytes_used = 1,
      };

      token_length = tokens_append(f, tokens, comma);
    }

    // TODO: Should be an error regardless of debug.
    PANIC_IF(token_length == 0, "Unknown character %c (%x).", byte, byte);
  }

  PANIC_IF(byte_index != f->bytes_used,
           "File size is %d, but ended at %d.",
           f->bytes_used, byte_index);

  RETURN;
}

struct Label_Address {
  u32 byte_index;
  u32 label_length;
  u32 address;
  b8 occupied;
};

struct Label_Addresses {
  struct Label_Address *addresses;
  u32 used;
  u32 allocated;
};

typedef struct Label_Addresses Label_Addresses;

Label_Addresses label_addresses_setup(u8 *bytes, u32 num_bytes) {
  memset(bytes, 0, num_bytes); // Defaults each address.occupied to false.
  return (Label_Addresses) {
    .addresses = (struct Label_Address *)bytes,
    .allocated = num_bytes / sizeof(struct Label_Address),
    .used = 0
  };
}

void label_address_insert(Label_Addresses *addresses, file *f, Token token, u32 address) {
  CALL;

  PANIC_IF(token.type != TT_LABEL,
           "Internal: expected label token, found %d.", token.type);

  PANIC_IF(addresses->used >= addresses->allocated,
           "Internal: inserted maximum number of addresses.");

  const u32 start_index = fnv_1a_hash(&f->data[token.byte_index], token.bytes_used) % addresses->allocated;
  u32 index = start_index;
  b8 looped = false;

  while (!(start_index == index && looped)) {
    looped = true;

    if (addresses->addresses[index].occupied) {
      const u32 other_index = addresses->addresses[index].byte_index;
      const u32 other_length = addresses->addresses[index].label_length;
      const u32 byte_index = token.byte_index;
      const u32 length = token.bytes_used;

      PANIC_IF(token.bytes_used == other_length && string_equal(&f->data[byte_index], &f->data[other_index], length),
               "Duplicate label found %.*s at line %d:%d.", length, &f->data[byte_index],
               token.location.line, token.location.column);
    } else {
      addresses->addresses[index].byte_index = token.byte_index;
      addresses->addresses[index].label_length = token.bytes_used;
      addresses->addresses[index].occupied = true;
      addresses->addresses[index].address = address;
      addresses->used++;
      RETURN;
    }

    index++;
    if (index >= addresses->allocated) {
      index = 0;
    }
  }

  RETURN;
}

u32 label_addresses_get(Label_Addresses *addresses, file *f, Token token) {
  CALL;
  const u32 start_index = fnv_1a_hash(&f->data[token.byte_index], token.bytes_used) % addresses->allocated;
  u32 index = start_index;
  b8 looped = false;

  PANIC_IF(addresses->used == 0, "There are no labels.");

  while (addresses->addresses[index].occupied && !(start_index == index && looped)) {
    looped = true;
    struct Label_Address addr = addresses->addresses[index];
    if (token.bytes_used == addr.label_length &&
        string_equal(&f->data[token.byte_index], &f->data[addr.byte_index], addr.label_length)) {
      RETURN_VAL(addresses->addresses[index].address);
    }
    index++;
    if (index >= addresses->allocated) index = 0;
  }

  PANIC("Could not find label address %.*s. Start index: %d.", token.bytes_used, &f->data[token.byte_index], start_index);
  RETURN_VAL(0);
}

void label_addresses_display(Label_Addresses *addresses, file *f) {
  for (u32 i = 0; i < addresses->allocated; i++) {
    struct Label_Address addr = addresses->addresses[i];
    if (addr.occupied) {
      NOTE("Label %.*s Address: %d Index %d.", addr.label_length, &f->data[addr.byte_index], addr.address, i);
    }
  }
}

void parse_asm_and_generate_binary(file *const f, file *const b, Tokens *const tokens) {
  CALL;

  enum Parse_State {
    PARSE_STATEMENT = 0,
    PARSE_INSTRUCTION,
    PARSE_LABEL_DEFINITION,
    PARSE_END_LABEL_DEFINITION,
    PARSE_ARGUMENT,
    PARSE_NEXT_ARGUMENT,
    PARSE_OFFSET,
    PARSE_IMMEDIATE,
    PARSE_END_STATEMENT,
    PARSE_WRITE
  } state = PARSE_STATEMENT;

  u32 instruction_counter = 0;
  Instruction instruction = { 0 };

  const u32 label_mem_size = MAX_NUM_LABELS * sizeof(struct Label_Address);
  u8 label_memory[label_mem_size];
  Label_Addresses addresses = label_addresses_setup(label_memory, label_mem_size);

  u32 num_late_labels = 0;
  struct { u32 instruction; u32 token_index; }
    late_labels[MAX_NUM_LATE_LABELS];

  u32 tokens_consumed = 0;
  u32 token = 0;
  while (skippable_token(tokens_get(tokens, token).type)) token++;

  while(token < tokens->tokens_used) {
    // NOTE(YELLOW("Parse state:")" Token index: %d, parse state: %d", token, state);

    tokens_consumed = 0;

    Token current = tokens_get(tokens, token);
    b8 last_token = token >= tokens->tokens_used - 1;
    Token next = last_token ? (Token) { 0 } : tokens_get(tokens, token + 1);

    switch (state) {
      case PARSE_STATEMENT: {
        if (!last_token && current.type == TT_LABEL && next.type == TT_COLON) {
          state = PARSE_LABEL_DEFINITION;
        } else if (current.type == TT_INSTRUCTION){
          state = PARSE_INSTRUCTION;
        } else {
          PANIC("Expected instruction or label, found token type %d.", current.type);
        }
      } break;

      case PARSE_LABEL_DEFINITION: {
        PANIC_IF(last_token,
                 "Internal: should have at least 2 tokens left when reading a label, got %d.",
                 tokens->tokens_used - token);

        PANIC_IF(current.type != TT_LABEL && next.type != TT_COLON,
                 "Expected a label then a colon, got %*.s %*.s at line %d:%d.",
                 current.bytes_used, &f->data[current.byte_index],
                 next.bytes_used, &f->data[next.byte_index],
                 current.location.line, current.location.column);

        label_address_insert(&addresses, f, current, instruction_counter);
        tokens_consumed = 2;
        instruction.type = INST_TYPE_LABEL;

        if (token < tokens->tokens_used - 2 &&
            skippable_token(tokens->tokens[token + 2].type)) {
          state = PARSE_END_LABEL_DEFINITION;
        } else {
          state = PARSE_STATEMENT;
        }
      } break;

      case PARSE_INSTRUCTION: {
        const i8 *instruction_name = &f->data[current.byte_index];
        PANIC_IF(current.type >= NUM_INSTRUCTIONS,
                 "Unknown instruction, got %.*s at line %d:%d",
                 current.bytes_used, instruction_name, current.location.line);
        // This is why index into instruction_strings has to match Instruction_Name
        instruction.name = current.info.instruction_index;
        instruction.type = INST_TYPE_INSTRUCTION;
        instruction.location = current.location;
        tokens_consumed = 1;
        state = PARSE_ARGUMENT;
      } break;

      case PARSE_ARGUMENT: {
        PANIC_IF(current.type != TT_REGISTER &&
                 current.type != TT_LABEL &&
                 current.type != TT_INTEGER_LITERAL &&
                 !skippable_token(current.type),
                 "Expected register or immediate when parsing argument or ; or \\n, got token type %d.", current.type);

        PANIC_IF(last_token, "Bad ending when parsing statement");

        if (current.type == TT_LABEL || current.type == TT_INTEGER_LITERAL) {
          state = PARSE_IMMEDIATE;
          break;
        } else if (current.type == TT_REGISTER) {
          instruction_append_arg(&instruction, ARG_REGISTER, current.info.register_index);
        } else {
          state = PARSE_END_STATEMENT;
          break;
        }

        if (next.type == TT_SEMICOLON || next.type == TT_NEWLINE) {
          state = PARSE_END_STATEMENT;
          tokens_consumed = 1;
        } else if (next.type == TT_COMMA) {
          state = PARSE_NEXT_ARGUMENT;
          tokens_consumed = 1;
        } else {
          PANIC("Expected ';' or '\\n' or ';', found %.*s.",
                current.bytes_used, &f->data[current.byte_index]);
        }
      } break;

      case PARSE_IMMEDIATE: {
        // integer | label
        PANIC_IF(last_token, "Early end of instruction."); // NOTE: forces newline at the end.

        if (current.type == TT_INTEGER_LITERAL && next.type == TT_LEFT_BRACE) {
          state = PARSE_OFFSET;
          break;
        }

        if (current.type == TT_LABEL) {
          PANIC_IF(num_late_labels >= MAX_NUM_LATE_LABELS,
                   "Exceeded number of labels as immediates (%d).", num_late_labels);

          // NOTE("instruction: %d, label: %.*s", instruction_counter, current.bytes_used, &f->data[current.byte_index]);

          late_labels[num_late_labels].instruction = instruction_counter;
          late_labels[num_late_labels].token_index = token;
          num_late_labels++;

          instruction_append_arg(&instruction, ARG_LABEL, 0); // We don't know yet where the label is.
        } else if (current.type == TT_INTEGER_LITERAL) {
          instruction_append_arg(&instruction, ARG_INTEGER_LITERAL, current.info.integer_literal);
        } else {
          PANIC("Expected integer literal or label when parsing an immediate, found %.*s",
                current.bytes_used, &f->data[current.byte_index]);
        }

        if (next.type == TT_COMMA) {
          state = PARSE_NEXT_ARGUMENT;
        } else {
          state = PARSE_END_STATEMENT;
        }

        tokens_consumed = 1;
      } break;

      case PARSE_OFFSET: {
        // integer & ( & register & )
        b8 enough_tokens = token + 4 < tokens->tokens_used;
        PANIC_IF(!enough_tokens, "Early end of offset."); // NOTE: forces newline at the end.

        Token offset = current;
        Token left_brace = next;
        Token reg = tokens->tokens[token + 2];
        Token right_brace = tokens->tokens[token + 3];

        PANIC_IF(offset.type != TT_INTEGER_LITERAL ||
                 left_brace.type != TT_LEFT_BRACE ||
                 reg.type != TT_REGISTER ||
                 right_brace.type != TT_RIGHT_BRACE,
                 "Expected tokens %d %d %d %d, got %d %d %d %d.",
                 TT_INTEGER_LITERAL, TT_LEFT_BRACE, TT_REGISTER, TT_RIGHT_BRACE,
                 offset.type, left_brace.type, reg.type, right_brace.type);

        instruction_append_arg(&instruction, ARG_INTEGER_LITERAL, offset.info.integer_literal);
        instruction_append_arg(&instruction, ARG_REGISTER, reg.info.register_index);

        tokens_consumed = 4;

        Token next = tokens_get(tokens, token + 4);
        if (next.type == TT_COMMA) {
          state = PARSE_NEXT_ARGUMENT;
        } else {
          state = PARSE_END_STATEMENT;
        }

      } break;

      case PARSE_NEXT_ARGUMENT: {
        PANIC_IF(current.type != TT_COMMA,
                 "Expected ',' after an argument, found .*s.",
                 current.bytes_used, &f->data[current.byte_index]);
        tokens_consumed = 1;
        state = PARSE_ARGUMENT;
      } break;

      case PARSE_END_STATEMENT: {
        PANIC_IF(!skippable_token(current.type),
                 "Expected ';' or \\n after an instruction, found %.*s (token type %d).",
                 current.bytes_used, &f->data[current.byte_index], current.type);
        while (token + tokens_consumed < tokens->tokens_used &&
               skippable_token(tokens_get(tokens, token + tokens_consumed).type)){
          tokens_consumed++; // At start, it's zero.
        }
        state = PARSE_WRITE;
      } break;

      case PARSE_END_LABEL_DEFINITION: {
        PANIC_IF(!skippable_token(current.type),
                 "Expected ';' or \\n after a label definition, found %.*s (token type %d).",
                 current.bytes_used, &f->data[current.byte_index], current.type);
        while (token + tokens_consumed < tokens->tokens_used &&
               skippable_token(tokens_get(tokens, token + tokens_consumed).type)){
          tokens_consumed++; // At start, it's zero.
        }
        state = PARSE_STATEMENT;
      } break;

      case PARSE_WRITE: {
        match_instruction_and_write(&instruction, b);
        instruction_counter++;
        instruction = (struct Instruction) { 0 };
        state = PARSE_STATEMENT;
      } break;
      // TODO: have a panic for release, panic just for debug, and panic for internal error
      default: PANIC("Internal: Unknown asm parser state: %d.", state);
    }

    token += tokens_consumed;
  }

  PANIC_IF(token != tokens->tokens_used, "Did not end evenly when parsing tokens.");

  NOTE("Finished parsing %s.", f->path);

  label_addresses_display(&addresses, f);

  for (u32 label = 0; label < num_late_labels; label++) {
    const u32 late_address = label_addresses_get(&addresses, f, tokens->tokens[late_labels[label].token_index]);
    fix_label_address(b, late_labels[label].instruction, late_address);
  }

  RETURN;
}

i32 main(i32 num_args, char **args) {
  CALL;

  if (num_args < 3 || (num_args - 1) % 2 != 0) {
    printf(RED("Error") ": Expected assembly file names followed by binary name.\n");
    return 0;
  }

  // TODO: calibrate these at runtime
  const u32 file_size   =  2 KB;
  const u32 binary_size =  2 KB;
  const u32 tokens_size =  8 KB;
  const u32 memory_size = 32 KB;
  u8 memory[memory_size];

  Arena arena = arena_setup(memory, memory_size);

  file assembly = file_setup(arena_alloc(&arena, file_size, 1), file_size, NULL);
  file binary = file_setup(arena_alloc(&arena, binary_size, 4), binary_size, NULL);

  Tokens tokens = tokens_setup(arena_alloc(&arena, tokens_size, 1), tokens_size);

  for (u32 i = 1; i < num_args; i += 2) {

    file_change_path(&assembly, args[i]);
    file_read_into_memory(&assembly);

    if (file_failed(&assembly)) {
      file_close(&assembly);
      continue;
    }

    tokenize(&assembly, &tokens);

    tokens_display(&assembly, &tokens);

    file_change_path(&binary, args[2]);
    parse_asm_and_generate_binary(&assembly, &binary, &tokens);
    file_write_to_disk(&binary);
    file_close(&binary);

    file_close(&assembly);
  }

  RETURN_VAL(0);
}
