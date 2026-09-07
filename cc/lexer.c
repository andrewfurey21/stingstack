#include "lexer.h"
#include "alloc.h"
#include "file.h"
#include "utils.h"

void tokens_setup(Tokens *tokens, file *f, void *buffer) {
  tokens->tokens = buffer;
  tokens->f = f;
  tokens->num_tokens = 0;
}

void token_build(Tokens *tokens, file_location *location,
                 TokenType type, u32 file_index, u32 length,
                 const char *error_msg) {

  PANIC_IF(tokens->num_tokens >= CC_MAX_NUM_TOKENS,
           "Reached maximum number of tokens (%d).", CC_MAX_NUM_TOKENS);

  tokens->tokens[tokens->num_tokens].type = type;
  tokens->tokens[tokens->num_tokens].length = length;
  tokens->tokens[tokens->num_tokens].file_index = file_index;

  tokens->tokens[tokens->num_tokens].location.column = location->column;
  tokens->tokens[tokens->num_tokens].location.line = location->line;

  PANIC_IF(type != TOKEN_ERROR && error_msg != NULL,
           "Error message should be null for non-error token (%d), but contains %s.", type, error_msg);

  PANIC_IF(type == TOKEN_ERROR && error_msg == NULL,
           "Error message should not be null for error token.");

  tokens->tokens[tokens->num_tokens].error_msg = error_msg;

  tokens->num_tokens++;

}

void tokenize(Tokens *tokens, file *f, GlobalArena *arena) {
  void *tokens_buffer =
    global_arena_aligned_alloc(arena,
                               sizeof(Token) * CC_MAX_NUM_TOKENS,
                               alignof(Token));
  tokens_setup(tokens, f, tokens_buffer);

  u64 byte_index = 0;
  file_location location = { .line = 1, .column = 0 };
  for (u32 token_num_bytes = 0; byte_index < tokens->f->num_bytes;
       byte_index += token_num_bytes) {

    token_num_bytes = 0;
    const i8 byte = file_read_byte(tokens->f, byte_index);

    if (is_whitespace(byte)) {
      token_num_bytes = skip_whitespace(tokens->f, &location, byte_index);
      continue;
    }

    if (is_start_of_comment(byte)) {
      token_num_bytes = skip_if_comment(tokens, &location, byte_index);
      if (token_num_bytes > 0) continue;
    }

    if (is_number_related(byte)) {
      token_num_bytes = tokenize_number(tokens, &location, byte_index);
      if (token_num_bytes > 0) continue;
    }

    token_build(tokens, &location, TOKEN_ERROR, byte_index, 1, "Unknown token.");
    token_num_bytes = 1;
  }

  PANIC_IF(byte_index != tokens->f->num_bytes,
           "Did not correctly finish reading. Length of %s is %d but read until %d.",
           tokens->f->path, tokens->f->num_bytes, byte_index);

  token_build(tokens, &location, TOKEN_EOF, 0, byte_index, NULL);

  NOTE("End location line: %d, column: %d.",
       location.line, location.column);
}


b8 is_whitespace(i8 byte) {
  return byte == '\t' || byte == '\n' || byte == '\v' ||
         byte == '\f' || byte == '\r' || byte == ' ';
}

b8 is_start_of_comment(i8 byte) {
  return byte == '/';
}

b8 is_number(i8 byte) {
  return (byte >= '0' && byte <= '9');
}

b8 is_number_related(i8 byte) {
  return is_number(byte) || byte == '.';
}

b8 is_floating_point_suffix(i8 byte) {
  return byte == 'f' || byte == 'F';
}

// Each token generation function has to return the number of bytes it consumed

u32 skip_whitespace(file *f, file_location *location, u32 start_of_whitespace) {
  u32 whitespace_in_bytes = 0;
  i8 whitespace = file_read_byte(f, start_of_whitespace);

  PANIC_IF(!is_whitespace(whitespace),
           "Cannot skip byte 0x%x since it is not whitespace.", whitespace, whitespace);

  while (is_whitespace(whitespace)) {
    if (is_new_line(whitespace)) file_location_add_new_line(location);
    whitespace_in_bytes++;
    whitespace = file_read_byte_or_eof(f, start_of_whitespace + whitespace_in_bytes);
  }

  return whitespace_in_bytes;
}

u32 skip_if_comment(Tokens *tokens, file_location *location, u32 possible_start_of_comment) {
  const i8 byte_after_start_of_comment =
    file_read_byte_or_eof(tokens->f, possible_start_of_comment + 1);

  if (byte_after_start_of_comment == CC_END_OF_FILE_CHAR) {
    // Token error, early eof.
    token_build(tokens,
                location,
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
                                           CC_NEW_LINE_CHAR);
    const u32 end_of_comment_or_eof =
      possible_start_of_comment + num_bytes_to_skip_from_start_of_comment;
    if (!file_at_end(tokens->f, end_of_comment_or_eof)) {
      file_location_add_new_line(location);
      num_bytes_to_skip_from_start_of_comment++;
    } else {
      NOTE("Comment ended with eof.");
    }
  }

  // Multi-line comment. C does not have nested block comments.
  // Should return error token if no ending '*/'.
  else if (byte_after_start_of_comment == '*') {
    num_bytes_to_skip_from_start_of_comment =
      num_bytes_until_end_of_matching_string_or_eof(tokens->f,
                                             possible_start_of_comment + 2,
                                             "*/") + 2;

    const u32 end_of_comment_or_eof =
      possible_start_of_comment + num_bytes_to_skip_from_start_of_comment;

    if (file_at_end(tokens->f, end_of_comment_or_eof)) {
      token_build(tokens,
                  location,
                  TOKEN_ERROR,
                  possible_start_of_comment,
                  num_bytes_to_skip_from_start_of_comment,
                  "Multi-line comment should end with */, found end of file.");
    }
    file_location_move_until(tokens->f,
                             location,
                             possible_start_of_comment,
                             num_bytes_to_skip_from_start_of_comment);
  } else {
    // Not a comment.
    num_bytes_to_skip_from_start_of_comment = 0;
  }

  return num_bytes_to_skip_from_start_of_comment;
}

b8 match_next_bytes(file *f, u32 start_of_token_in_file,
                    const char *match_string) {
  const u32 match_len = strlen(match_string);
  PANIC_IF(match_len + start_of_token_in_file >= f->num_bytes,
           "Cannot search for match %s in file %s because file length is %d"
           " and match would end at %d.",
           match_string, f->path, f->num_bytes,
           match_len + start_of_token_in_file);

  b8 found_match = true;

  for (u32 match_index = 0; match_index < match_len; match_index++) {
    const i8 match = match_string[match_index];
    if (!match_next_byte(f, start_of_token_in_file + match_index, match)) {
      found_match = false;
      break;
    }
  }

  return found_match;
}

b8 match_next_byte(file *f, u32 start_of_token_in_file, char match) {
  i8 next_byte = peek_next_byte(f, start_of_token_in_file);
  return next_byte == match;
}

i8 peek_next_byte(file *f, u32 start_of_token_in_file) {
  return file_read_byte(f, start_of_token_in_file + 1);
}

u32 num_bytes_until_matching_byte_or_eof(file *f, u32 start_index, i8 match) {
  u32 number_of_bytes = 0;
  i8 current_byte  = file_read_byte_or_eof(f, start_index + number_of_bytes);
  while (current_byte != match && current_byte != CC_END_OF_FILE_CHAR) {
    number_of_bytes++;
    current_byte  = file_read_byte_or_eof(f, start_index + number_of_bytes);
  }

  NOTE_IF(current_byte == CC_END_OF_FILE_CHAR,
          "Found end of file while matching 0x%x", match);
  return number_of_bytes;
}

u32 num_bytes_until_end_of_matching_string_or_eof(file *f,
                                           const u32 start_index,
                                           const char *substring) {
  // Typically substring is tiny compared to file. So naive is probably fine.
  const u32 string_len = strlen(substring);
  u32 substring_index = start_index;

  b8 substring_match = false;
  while (!substring_match && !file_at_end(f, substring_index + string_len - 1)) {
    substring_match = true;

    for (u32 string_index = 0; string_index < string_len; string_index++) {
      const i8 current_file_byte =
        file_read_byte(f, substring_index + string_index);
      const i8 current_substring_byte = substring[string_index];
      if (current_file_byte != current_substring_byte) {
        substring_match = false;
        break;
      }
    }

    if (!substring_match) substring_index++;
  }

  NOTE_IF(!(file_at_end(f, substring_index + string_len - 1) || substring_match),
          "Should either be at the end of the file, or found a substring match.");

  if (file_at_end(f, substring_index + string_len - 1)) {
    return (substring_index + string_len - 1) - start_index;
  } else {
    return substring_index - start_index + string_len;
  }
}

u32 tokenize_number(Tokens *tokens, file_location *location, u32 start_of_token_index) {
  return 0;
  // TODO: add floating point hexadecimal support
  Token *current_token = &tokens->tokens[tokens->num_tokens - 1];
  i8 current_byte = file_read_byte(tokens->f, start_of_token_index);
  b8 found_point_already = false;

  TokenType type = TOKEN_INT_LITERAL;

  // Can't be floats: 0b/0x/0o, so for 0x, don't check float prefix.
  if (current_byte == '.') {
    found_point_already = true;
    if (!is_number(peek_next_byte(tokens->f, start_of_token_index))) {
      return 0;
    }
  }

  // keep going until second dot or not numeric.
  u32 end_of_token_index = start_of_token_index;
  while (is_number_related(peek_next_byte(tokens->f, end_of_token_index))) {
    current_byte = peek_next_byte(tokens->f, end_of_token_index);
    end_of_token_index++;

    if (current_byte == '.') {
      if (found_point_already) {

      } else {

      }
    }
  }

  // check float prefix. only allowed with dot = true;
  // if alpha, keep going until non-alphanumeric, generate error token.

  u32 token_length = end_of_token_index - start_of_token_index;
  NOTE_IF(token_length == 0,
          "Length of number token should not be zero.");

  token_build(tokens, location, type, start_of_token_index, token_length, NULL);
  return token_length;
}

u32 tokens_display_errors(Tokens *tokens) {
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

b8 token_display_error(Tokens *tokens, u32 token_index) {
  Token token = tokens->tokens[token_index];
  if (token.type == TOKEN_ERROR) {
    NOTE("Token error at line %d, column %d: %s",
         token.location.line, token.location.column, token.error_msg);
    return true;
  }
  return false;
}
