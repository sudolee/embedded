#ifndef _DEBUG_H_
#define _DEBUG_H_
/* Export for other modules */

/*
 * How lights show ?
 * 1 -> light on
 * 0 -> light off
 *
 * no=1 : 0001
 * no=2 : 0010
 * no=3 : 0011
 * no=4 : 0100
 * ....
 */
void lights(int no);

#endif /* _DEBUG_H_ */
