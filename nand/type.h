#ifndef _TYPE_H_
#define _TYPE_H_

#define NULL ((void*)0)

/* Note:
 * Currently, not support 64-bits on my board, use u32 instead.
 */
//typedef unsigned long long  u64;
typedef unsigned long  u64;
typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

//typedef signed long long  s64;
typedef signed long  s64;
typedef signed long  s32;
typedef signed short s16;
typedef signed char  s8;

typedef unsigned long size_t;

#endif /* _TYPE_H_ */
