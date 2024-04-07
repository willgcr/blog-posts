// overflow1.c

#include <stdio.h>

void main (void){
	int array[6] = {1, 2, 3, 4, 5, 6};
	printf ("%d\n", array[6]);
	printf ("%d\n", *(array+6));
	return;
}

// To compile: gcc -o overflow1 overflow1.c