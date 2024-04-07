// overflow3.c

#include <stdio.h>

void myFunc (int a, int b, int c, int d, int e, int f, int g, int h){
	int myArray [4];
	myArray[0] = 5;
	return;
}

int main (void){
	myFunc (1, 2, 3, 4, 5, 6, 7, 8);
	printf ("Hello World!\n");
	return (0);
}

// To compile: gcc -o overflow3 overflow3.c -fno-stack-protector -O0