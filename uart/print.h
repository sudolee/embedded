#ifndef _PRINT_H_
#define _PRINT_H_
/* Export for other modules */

#include "vaargs.h"
#include "type.h"

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);


#endif /* _PRINT_H_ */
