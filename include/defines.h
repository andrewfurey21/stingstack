#ifndef DEFINES_H
#define DEFINES_H

#define true  1
#define false 0

#define KB * 1024
#define MB KB KB
#define GB MB KB

#define MAX_FILE_NUM_BYTES 1 KB
#define END_OF_FILE_CHAR -1
#define COLUMN_LIMIT 255
#define LINE_LIMIT 255


#define GREEN(x) "\033[32m" x "\033[0m"
#define RED(x) "\033[31m" x "\033[0m"
#define YELLOW(x) "\033[33m" x "\033[0m"
#define BLUE(x) "\033[34m" x "\033[0m"

#define STATIC_ARRAY_LENGTH(arr) ((sizeof(arr) / sizeof(arr[0])))

#endif
