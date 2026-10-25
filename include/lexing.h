#ifndef LEXING_H
#define LEXING_H

#include "file.h"

/*

Tools to help with lexing.

*/

static inline b8 is_whitespace(i8 byte) {
  return byte == '\t' || byte == '\n' || byte == '\v' ||
         byte == '\f' || byte == '\r' || byte == ' ';
}

static inline b8 is_non_newline_whitespace(i8 byte) {
  // Compiler should be able to optimize this.
  return is_whitespace(byte) && (byte != '\n');
}


static inline b8 is_number(i8 byte) {
  return (byte >= '0' && byte <= '9');
}

static inline b8 is_number_related(i8 byte) {
  return is_number(byte) || byte == '.';
}

static inline b8 is_floating_point_suffix(i8 byte) {
  return byte == 'f' || byte == 'F';
}

static inline b8 is_hexadecimal(i8 byte) {
  return is_number(byte) ||
         (byte >= 'a' && byte <= 'f') ||
         (byte >= 'A' && byte <= 'F');
}

static inline b8 is_octal(i8 byte) {
  return byte >= '0' && byte <= '7';
}

static inline b8 is_c_string_literal_related(i8 byte) {
  return byte == '\"';
}

static inline i8 peek_next_byte(file *f, u32 start_of_token_in_file) {
  CALL;
  i8 byte = file_read_byte(f, start_of_token_in_file + 1);

  RETURN_VAL(byte);
}

static inline b8 match_next_byte(file *f, u32 start_of_token_in_file, char match) {
  CALL;
  i8 next_byte = peek_next_byte(f, start_of_token_in_file);

  RETURN_VAL(next_byte == match);
}

static inline u32 skip_whitespace(file *f, file_location *location, u32 start_of_whitespace) {
  CALL;

  u32 whitespace_in_bytes = 0;
  i8 whitespace = file_read_byte(f, start_of_whitespace);

  PANIC_IF(!is_whitespace(whitespace),
           "Cannot skip byte 0x%x since it is not whitespace.",
           whitespace);

  while (is_whitespace(whitespace)) {
    if (whitespace == '\n') file_location_add_new_line(location);
    else file_location_next(location, whitespace);
    whitespace_in_bytes++;
    whitespace = file_read_byte_or_eof(f, start_of_whitespace + whitespace_in_bytes);
  }

  RETURN_VAL(whitespace_in_bytes);
}

// Identical to above. Use fnptrs?
static inline u32 skip_non_newline_whitespace(file *f, file_location *location, u32 start_of_whitespace) {
  CALL;

  u32 whitespace_in_bytes = 0;
  i8 whitespace = file_read_byte(f, start_of_whitespace);

  PANIC_IF(!is_non_newline_whitespace(whitespace),
           "Cannot skip byte 0x%x since it is not whitespace.",
           whitespace);

  while (is_non_newline_whitespace(whitespace)) {
    file_location_next(location, whitespace);
    whitespace_in_bytes++;
    whitespace = file_read_byte_or_eof(f, start_of_whitespace + whitespace_in_bytes);
  }

  RETURN_VAL(whitespace_in_bytes);
}

static inline b8 match_next_bytes(file *f, u32 start_of_token_in_file, const char *match_string) {
  CALL;
  const u32 match_len = strlen(match_string);
  PANIC_IF(match_len + start_of_token_in_file >= f->bytes_used,
           "Cannot search for match %s in file %s because file length is %d"
           " and match would end at %d.",
           match_string, f->path, f->bytes_used,
           match_len + start_of_token_in_file);

  b8 found_match = true;

  for (u32 match_index = 0; match_index < match_len; match_index++) {
    const i8 match = match_string[match_index];
    if (!match_next_byte(f, start_of_token_in_file + match_index, match)) {
      found_match = false;
      break;
    }
  }

  RETURN_VAL(found_match);
}

static inline u32 num_bytes_until_matching_byte_or_eof(file *f, u32 start_index, i8 match) {
  CALL;
  u32 number_of_bytes = 0;
  i8 current_byte  = file_read_byte_or_eof(f, start_index + number_of_bytes);
  while (current_byte != match && current_byte != END_OF_FILE_CHAR) {
    number_of_bytes++;
    current_byte  = file_read_byte_or_eof(f, start_index + number_of_bytes);
  }

  NOTE_IF(current_byte == END_OF_FILE_CHAR,
          "Found end of file while matching 0x%x", match);
  RETURN_VAL(number_of_bytes);
}

static inline u32 num_bytes_until_end_of_substring_or_eof
  (file *f, const u32 start_index, const char *substring) {
  CALL;

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
    RETURN_VAL((substring_index + string_len - 1) - start_index);
  } else {
    RETURN_VAL(substring_index - start_index + string_len);
  }
}

#endif
