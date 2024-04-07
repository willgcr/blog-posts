// overflow5.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int checkLicense (void);
void licensingSucceeds (void);
void licensingFails (void);

int main (void) {
	if (!checkLicense ()) {
		licensingFails ();
		return (1);
	} else {
		licensingSucceeds ();
		return (0);
	}
}

void licensingSucceeds (void) {
	printf ("Software Licensed!\n");
	return;
}

void licensingFails (void) {
	printf ("Invalid Key!\n");
	return;
}

int checkLicense (void) {
	char serial[32];
	int total = 0;	
	printf ("License Key: ");
	scanf ("%s", serial);
	for (int i = 0; i < strlen(serial); i++) {
		total += serial[i];
	}
	if (total % 77 == 11)
		return (1);
	else
		return (0);
}

// To compile: gcc -o overflow5 overflow5.c -fno-stack-protector -O0