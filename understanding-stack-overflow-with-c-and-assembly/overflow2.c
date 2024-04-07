// overflow2.c

#include <stdio.h>

void main (void){
	int myArray[25];
	myArray[0] = 0;
	myArray[24] = 1;
	myArray[25] = 2;
	myArray[30] = 5;
	return;
}

// To compile: gcc -o overflow2 overflow2.c -fno-stack-protector -O0