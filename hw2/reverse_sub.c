/*
 * =====================================================================================
 *
 *       Filename:  reverse_sub.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/13/2023 05:10:59 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Benjamin Gillmore (bg), bggillmore@gmail.com
 *
 * =====================================================================================
 */
#include <stdio.h>
float reverse_subtract_1(float in1, float in2){
	float ret = 0.0;
	asm ("fsubr %2, %0" : "=&t" (ret) : "%0" (in1), "u" (in2));
	return ret;
}


int main(){
	float out1 = reverse_subtract_1(1.3, 0.6);
	printf("1.3 - 0.6 = %f\n", out1);
	return 0;
}
