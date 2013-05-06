#ifndef _VAARGS_H_
#define _VAARGS_H_

typedef char* va_list;

#define  _AUPBND                (sizeof(long) - 1)
#define  _ADNBND                (sizeof(long) - 1)

/* Variable argument list macro definitions */
#define _bnd(X, bnd)            (((sizeof (X)) + (bnd)) & (~(bnd)))
#define va_arg(ap, T)           (*(T *)(((ap) += (_bnd (T, _AUPBND))) - (_bnd (T,_ADNBND))))
#define va_end(ap)              (ap = (va_list) NULL)
#define va_start(ap, A)         (void) ((ap) = (((char *) &(A)) + (_bnd (A,_AUPBND))))

/* flag types(max. <<31) */
#define FT_LEFT       (1 << 0)
#define FT_PLUS       (1 << 1)
#define FT_SPACE      (1 << 2)
#define FT_SPECIAL    (1 << 3)
#define FT_ZEROPAD    (1 << 4)
/*
 * FT_LOWERCASE must equel to 0x20 (1<<5)
 *   because: lowercase = uppercase | 0x20;
 *   also: lowercase = uppercase + 32;
 */
#define FT_LOWERCASE  (1 << 5)
#define FT_SIGNED_NUM (1 << 6)

#endif /* _VAARGS_H_ */
