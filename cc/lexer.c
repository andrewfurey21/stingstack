#include "lexer.h"

void tokens_setup(Tokens *tokens, file *f, void *buffer, u32 capacity) {
  tokens->tokens = buffer;
  tokens->f = f;
  tokens->num_tokens = 0;
  tokens->location.line = 1;
  tokens->location.column = 0;
  tokens->capacity = capacity;

  error_set(&tokens->tokenizing_error, NO_ERROR_YET, NULL);
}

void token_build(Tokens *tokens,
                 TokenType type, u32 file_index, u32 length,
                 const char *error_msg) {

  PANIC_IF(tokens->num_tokens >= tokens->capacity,
           "Reached maximum number of tokens (%d).", tokens->capacity);

  tokens->tokens[tokens->num_tokens].type = type;
  tokens->tokens[tokens->num_tokens].length = length;
  tokens->tokens[tokens->num_tokens].file_index = file_index;

  tokens->tokens[tokens->num_tokens].location.column =
    tokens->location.column;
  tokens->tokens[tokens->num_tokens].location.line =
    tokens->location.line;

  PANIC_IF(type != TOKEN_ERROR && error_msg != NULL,
           "Error message should be null for non-error token (%d), but contains %s.", type, error_msg);

  PANIC_IF(type == TOKEN_ERROR && error_msg == NULL,
           "Error message should not be null for error token.");

  tokens->tokens[tokens->num_tokens].error_msg = error_msg;

  tokens->num_tokens++;
}

void tokenize(Tokens *tokens) {

  if (file_failed(tokens->f)) {
    error_set(&tokens->tokenizing_error, FAILED, "tokenize(): File read failed.");
    return;
  }

  u32 byte_index = 0;
  for (u32 token_num_bytes = 0; byte_index < tokens->f->bytes_used;
       byte_index += token_num_bytes) {

    // consume everything needed. byte_index + token_num_bytes should produce a byte not read yet.
    token_num_bytes = 0;
    const i8 byte = file_read_byte(tokens->f, byte_index);

    if (is_whitespace(byte)) {
      token_num_bytes =
        skip_whitespace(tokens->f, &tokens->location, byte_index);
      continue;
    }

    if (is_start_of_comment(byte)) {
      token_num_bytes = skip_if_comment(tokens, byte_index);
      if (token_num_bytes > 0) continue;
    }

    if (is_c_string_literal_related(byte)) {
      token_num_bytes = tokenize_string_literal(tokens, byte_index);
      if (token_num_bytes > 0) continue; // TODO: check this returns correctly.
    }

    token_build(tokens, TOKEN_ERROR, byte_index, 1, "Unknown token.");
    token_num_bytes = 1;
  }

  PANIC_IF(byte_index != tokens->f->bytes_used,
           "Did not correctly finish reading. Length of %s is %d but read until %d.",
           tokens->f->path, tokens->f->bytes_used, byte_index);

  token_build(tokens, TOKEN_EOF, 0, byte_index, NULL);

  NOTE("End location line: %d, column: %d.",
       tokens->location.line, tokens->location.column);

  error_set(&tokens->tokenizing_error, SUCCESS, NULL);
}


// Each token generation function has to return the number of bytes it consumed

u32 skip_whitespace(file *f, file_location *location, u32 start_of_whitespace) {
  u32 whitespace_in_bytes = 0;
  i8 whitespace = file_read_byte(f, start_of_whitespace);

  PANIC_IF(!is_whitespace(whitespace),
           "Cannot skip byte 0x%x since it is not whitespace.", whitespace, whitespace);

  while (is_whitespace(whitespace)) {
    if (whitespace == '\n') file_location_add_new_line(location);
    whitespace_in_bytes++;
    whitespace = file_read_byte_or_eof(f, start_of_whitespace + whitespace_in_bytes);
  }

  return whitespace_in_bytes;
}

u32 skip_if_comment(Tokens *tokens, u32 possible_start_of_comment) {
  const i8 byte_after_start_of_comment =
    file_read_byte_or_eof(tokens->f, possible_start_of_comment + 1);

  if (byte_after_start_of_comment == END_OF_FILE_CHAR) {
    // Token error, early eof.
    token_build(tokens,
                TOKEN_ERROR,
                possible_start_of_comment,
                1,
                "Erroneous end of file at '/'.");
  }

  u32 num_bytes_to_skip_from_start_of_comment;
 // Single line comment
  if (byte_after_start_of_comment == '/') {
    num_bytes_to_skip_from_start_of_comment =
      num_bytes_until_matching_byte_or_eof(tokens->f,
                                           possible_start_of_comment,
                                           '\n');
    const u32 end_of_comment_or_eof =
      possible_start_of_comment + num_bytes_to_skip_from_start_of_comment;
    if (!file_at_end(tokens->f, end_of_comment_or_eof)) {
      file_location_add_new_line(&tokens->location);
      num_bytes_to_skip_from_start_of_comment++;
    } else {
      NOTE("Comment ended with eof.");
    }
  }

  // Multi-line comment. C does not have nested block comments.
  // Should return error token if no ending '*/'.
  else if (byte_after_start_of_comment == '*') {
    num_bytes_to_skip_from_start_of_comment =
      num_bytes_until_end_of_substring_or_eof(tokens->f,
                                             possible_start_of_comment + 2,
                                             "*/") + 2;

    const u32 end_of_comment_or_eof =
      possible_start_of_comment + num_bytes_to_skip_from_start_of_comment;

    if (file_at_end(tokens->f, end_of_comment_or_eof)) {
      token_build(tokens,
                  TOKEN_ERROR,
                  possible_start_of_comment,
                  num_bytes_to_skip_from_start_of_comment,
                  "Multi-line comment should end with */, found end of file.");
    }
    file_location_move_until(&tokens->location,
                             tokens->f,
                             possible_start_of_comment,
                             num_bytes_to_skip_from_start_of_comment);
  } else {
    // Not a comment.
    num_bytes_to_skip_from_start_of_comment = 0;
  }

  return num_bytes_to_skip_from_start_of_comment;
}


u32 tokenize_string_literal(Tokens *tokens, u32 start_of_token_index) {

  u32 current_index = start_of_token_index;
  i8 current_byte;
  // I know, I just wanna stress my compiler compiling itself.
  while ((current_byte = file_read_byte_or_eof(tokens->f, ++current_index)) != END_OF_FILE_CHAR) {
    if (current_byte == '\"') break;
    // TODO: simple and numeric escape sequences
  }

  u32 token_length;
  if (current_byte == END_OF_FILE_CHAR) {
    token_length = current_index - start_of_token_index;
    token_build(tokens, TOKEN_ERROR, start_of_token_index, token_length,
                "tokenize_string_literal(): Reached early eof when reading a string literal.");

  } else if (current_byte == '\"') {
    token_length = current_index - start_of_token_index + 1;
    token_build(tokens, TOKEN_STRING_LITERAL, start_of_token_index, token_length, NULL);
  } else {
    NOTE("tokenize_string_literal(): Should either reach eof, or \".");
  }

  // return token_length;
  return 0;
}

u32 tokens_display_errors(Tokens *tokens) {
  if (tokens->tokenizing_error.type == FAILED) {
    NOTE("A problem occurred when tokenizing: %s",
         tokens->tokenizing_error.message);
    return 1;
  }

  u32 num_token_errors = 0;
  for (u32 token_index = 0; token_index < tokens->num_tokens; token_index++) {
    num_token_errors += (u32)token_display_error(tokens, token_index);
  }

  if (num_token_errors > 0) {
    NOTE("Number of token errors: %d.", num_token_errors);
  } else {
    NOTE("No token errors.");
  }

  return num_token_errors;
}

u32 token_display_error(Tokens *tokens, u32 token_index) {
  Token token = tokens->tokens[token_index];
  if (token.type == TOKEN_ERROR) {
    NOTE("Token error at line %d, column %d: %s",
         token.location.line, token.location.column, token.error_msg);
    return 1;
  }

  return 0;
}

