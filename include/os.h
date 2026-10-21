#ifndef OS_H
#define OS_H

#ifdef __linux__
  #include "linux.h"
#endif

#define PAGE_SIZE LINUX_PAGE_SIZE
#define os_file_open linux_file_open
#define os_file_read linux_file_read
#define os_file_write linux_file_write
#define os_file_close linux_file_close
#define os_page_alloc linux_page_alloc
#define os_page_dealloc linux_page_dealloc

#endif
