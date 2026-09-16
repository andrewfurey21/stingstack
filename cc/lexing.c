#include "lexing.h"

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

b8 is_hexadecimal(i8 byte) {
  return (byte >= '0' && byte <= '9') ||
         (byte >= 'a' && byte <= 'f') ||
         (byte >= 'A' && byte <= 'F');
}

b8 is_octal(i8 byte) {
  return byte >= '0' && byte <= '7';
}

b8 is_c_string_literal_related(i8 byte) {
  // return (byte == 'L' || byte == '\"');
  return byte == '\"';
}

b8 match_next_bytes(file_read *f, u32 start_of_token_in_file,
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

b8 match_next_byte(file_read *f, u32 start_of_token_in_file, char match) {
  i8 next_byte = peek_next_byte(f, start_of_token_in_file);
  return next_byte == match;
}

i8 peek_next_byte(file_read *f, u32 start_of_token_in_file) {
  return file_read_byte(f, start_of_token_in_file + 1);
}

u32 num_bytes_until_matching_byte_or_eof(file_read *f, u32 start_index, i8 match) {
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

u32 num_bytes_until_end_of_substring_or_eof(file_read *f,
                                           const u32 start_index,
                                           const char *substring) {
  // Typically substring is tiny compared to file. So naive is probably fine.
  const u32 string_len = strlen(substring);
  u32 substring_index = start_index;

  b8 substring_match = false;
  while (!substring_match && !file_read_at_end(f, substring_index + string_len - 1)) {
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

  NOTE_IF(!(file_read_at_end(f, substring_index + string_len - 1) || substring_match),
          "Should either be at the end of the file, or found a substring match.");

  if (file_read_at_end(f, substring_index + string_len - 1)) {
    return (substring_index + string_len - 1) - start_index;
  } else {
    return substring_index - start_index + string_len;
  }
}

