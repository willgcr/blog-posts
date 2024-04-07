// overflow4.c

#include <stdio.h>

void readString (void){
	char myArray[32];
	scanf ("%s", myArray);
	printf ("%s\n", myArray);
	return;
}

int main (void){
	readString ();
	return (0);
}

// To compile: gcc -o overflow4 overflow4.c -fno-stack-protector -O0