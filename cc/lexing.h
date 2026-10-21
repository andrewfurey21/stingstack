#include "../utils/utils.h"
#include "file.h"

b8 is_whitespace(i8 byte);
b8 is_start_of_comment(i8 byte);
b8 is_number(i8 byte);
b8 is_hexadecimal(i8 byte);
b8 is_octal(i8 byte);
b8 is_number_related(i8 byte);
b8 is_floating_point_suffix(i8 byte);
b8 is_c_string_literal_related(i8 byte);

u32 num_bytes_until_matching_byte_or_eof(file_read *f, u32 file_index, i8 match);
// TODO: look at boyer-moore, knuth-morrs-partt, two-way, bitap
u32 num_bytes_until_end_of_substring_or_eof(file_read *f, u32 file_index, const char *substring);

b8 match_next_bytes(file_read *f, u32 file_index, const char *match);
b8 match_next_byte(file_read *f, u32 file_index, char match);
i8 peek_next_byte(file_read *f, u32 file_index);
