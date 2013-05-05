#include "type.h"
#include "vaargs.h"

#if 0
/* needed while link */
static void *memcpy(void *dest, const void *src, u32 count)
{
	char *temp = dest;
	const char *s = src;

	while (count--)
		*temp++ = *s++;
	return dest;
}
#endif

#define ADDCH(__str, __ch) 	do{ \
			*(__str) = (__ch); \
			++(__str); \
		} while (0)

#define is_digit(ch) (((ch) >= '0') && ((ch) <= '9'))

/*
 * Return from skip_atoi(), s(fmt) already points to next character.
 *   That's the meaning of "skip".
 */
static int skip_atoi(const char **s)
{
	int i = 0;
	for(;is_digit(**s); ++(*s))
		i = i*10 + **s - '0';

	return i;
}

static int strnlen(const char *s, size_t count)
{
	const char *sc;

	for(sc = s; count-- && *sc != '\0'; ++sc)
		;
	return sc - s;
}

/* TODO: only support n <= 0xFFFF, 16-bits */
static int putdec(char *buf, u64 n)
{
	char p[4];

	n &= 0xFFFF;

	for(p[0] = '0' - 1; n >= 0; n -= 10000) ++p[0];
	for(p[1] = '9' + 1; n <  0; n += 1000 ) --p[1];
	for(p[2] = '0' - 1; n >= 0; n -= 100  ) ++p[2];
	for(p[3] = '9' + 1; n <  0; n += 10   ) --p[3];

	ADDCH(buf, p[0]);
	ADDCH(buf, p[1]);
	ADDCH(buf, p[2]);
	ADDCH(buf, p[3]);
	ADDCH(buf, n + '0');

	return 0;
}

static char *string(char *buf, char *end, char *s,
		int prec, int field_width, int flags)
{
	int len;

	if (s == NULL)
		s = "NULL";

	len = strnlen(s, prec);

	/* align to right ? */
	if(!(flags & FT_LEFT)) {
		while(len < field_width--)
			ADDCH(buf, ' ');
	}

	while (len-- && ((buf + len) < end))
		ADDCH(buf, *s++);

	/* align to left ? */
	while(len < field_width--)
		ADDCH(buf, ' ');

	return buf;
}

static char *number(char *buf, char *end, u64 num,
		int base, int prec, int width, int flags)
{
	/* we are called with base 8, 10 or 16, only, thus don't need "G..."  */
	static const char digits[] = "0123456789ABCDEF";

	char temp[24];
	char lowercase, sign;
	int need_prefix = (flags & FT_SPECIAL) && (base != 10);
	int pos;

	/*
	 * FT_LOWERCASE must equal to 0x20 (1<<5).
	 *   because: lowercase = uppercase | 0x20 = uppercase + 0x20;
	 */
	lowercase = flags & FT_LOWERCASE;
	/* if align to left, zero pad don't need. */
	if(flags & FT_LEFT)
		flags &= ~FT_ZEROPAD;

	sign = 0;
	if(flags & FT_SIGNED_NUM) {
		if((u64)num < 0) {
			sign = '-';
			num = -(s64)num;
			width--;
		} else if(flags & FT_PLUS) {
			sign = '+';
			width--;
		} else if(flags & FT_SPACE) {
			sign = ' ';
			width--;
		}
	}
	if(need_prefix) {
		width--;	/* reserved for 8 based(0) */
		if(base == 16)
			width--;	/* reserved for 16 based(0x/0X) */
	}

	/* generate full string in temp[], in reverse order! */
	pos = 0;
	if (num == 0) {
		temp[pos++] = '0';
	} else if (base != 10) {	/* 8/16 base */
		int mask = base - 1;
		/* don't use shift variable there, otherwise, you need implement "__lshrdi3()" */

		do {
			temp[pos++] = (digits[((unsigned char)num) & mask]) | lowercase;
			if(base == 8) {
				num >>= 3;
			} else {
				num >>= 4;
			}
		} while (num);
	} else { /* base 10 */
//		pos = putdec(&temp[pos], num);
		pos = putdec(temp, num);
	}

	/* printing 100 using %.2d gives "100", not "00" */
	if(pos > prec)
		prec = pos;
	/* leading zero padding */
	width -= prec;
	if(!(flags & (FT_ZEROPAD + FT_LEFT))) {
		while(--width >= 0)
			ADDCH(buf, ' ');
	}

	/* has sign ? */
	if(sign)
		ADDCH(buf, sign);
	/* "0x" / "0" prefix */
	if (need_prefix) {
		ADDCH(buf, '0');
		if(base == 16)
			ADDCH(buf, 'X' | lowercase);
	}
	/* zero or space padding */
	if(!(flags & FT_LEFT)) {
		char __ch = (flags & FT_ZEROPAD) ? '0' : ' ';
		while(--width >= 0)
			ADDCH(buf, __ch);
	}
	/* Hoo, more zero padding? */
	while(pos <= --prec)
		ADDCH(buf, '0');

	/* actual digits of result */
	while (--pos >= 0)
		ADDCH(buf, temp[pos]);

	/* trailing space padding */
	while(--width >= 0)
		ADDCH(buf, ' ');
	return buf;
}

/*
 * fmt: %[flags][width][.precision][length]specifier
 */
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
	u64 num;
	char *end, *str;
	int flags;	/* flags to number() */
	int field_width;	/* width of output field */

	int prec;	/* min of digits for integers;
				 * max of chars for string.
				 */
	int qua;	/* 'h', 'l', or 'L' for integer fields */
	int base;

	str = buf;
	end = buf + size;

	/* e.g: %#-13.24ld */
	for (; *fmt; ++fmt) {
		if (*fmt != '%') {
			ADDCH(str, *fmt);
			continue;
		}

		/* process flags */
		flags = 0;
next_flag:
		++fmt;	/* this also skip first '%' */
		switch(*fmt) {
			case '-':
				flags |= FT_LEFT;
				goto next_flag;
			case '+':
				flags |= FT_PLUS;
				goto next_flag;
			case ' ':
				flags |= FT_SPACE;
				goto next_flag;
			case '#':
				flags |= FT_SPECIAL;
				goto next_flag;
			case '0':
				flags |= FT_ZEROPAD;
				goto next_flag;
		}

		/* get field width */
		field_width = -1;
		if(is_digit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			++fmt;
			/* it's the next argument */
			field_width = va_arg(args, int);
			if(field_width < 0) {
				field_width = -field_width;
				flags |= FT_LEFT;
			}
		}

		/* get the precision */
		prec = -1;
		if(*fmt == '.') {
			++fmt;
			if(is_digit(*fmt))
				prec = skip_atoi(&fmt);
			else if(*fmt == '*') {
				/* it's the next argument */
				prec = va_arg(args, int);
			}
			if(prec < 0)
				prec = 0;
		}

		/* get the conversion qualifier */
		qua = -1;
		if(*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
			qua = *fmt;
			++fmt;
			if(qua == 'l' && *fmt == 'l') {
				qua = 'L';
				++fmt;
			}
		}

		/* default base */
		base = 16;

		switch (*fmt) {
		case '%':
			/* another '%' ? */
			ADDCH(str, '%');
			continue;

		case 'c':
			/* align to right ? */
			if(!(flags & FT_LEFT)) {
				while(--field_width > 0)
					ADDCH(str, ' ');
			}
			ADDCH(str, (char)va_arg(args, int));
			/* align to left ? */
			while(--field_width > 0)
				ADDCH(str, ' ');
			continue;

		case 's':
			str = string(str, end, va_arg(args, char *),
					prec, field_width, flags);
			continue;

		case 'p':
			/* not support currently */
			continue;

		case 'o':
			base = 8;
			break;

		case 'i':
		case 'd':
			flags |= FT_SIGNED_NUM;
		case 'u':
			base = 10;
			break;

		case 'x':
			flags |= FT_LOWERCASE;
		case 'X':
			break;

		default:
			/* No type matched! */
			ADDCH(str, '%');
			if (*fmt)
				ADDCH(str, *fmt);
			else
				--fmt;
			continue;
		}

		switch(qua) {
#if 0 /* 64-bits not used on my board */
			case 'L':	/* 64-bits */
				num = va_arg(args, u64);
				break;
#endif

			case 'l':
				num = va_arg(args, unsigned long);
				if(flags & FT_SIGNED_NUM)
					num = (signed long)num;
				break;

			case 'h':
				num = va_arg(args, unsigned short);
				if(flags & FT_SIGNED_NUM)
					num = (signed short)num;
				break;

			default:
				num = va_arg(args, unsigned int);
				if(flags & FT_SIGNED_NUM)
					num = (signed short)num;
		}
		str = number(str, end, num, base, prec, field_width, flags);
	}

	if (size > 0)
		*str = '\0';

	return str - buf;
}
