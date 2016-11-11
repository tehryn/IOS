/*
 * Author: Jiri Matejka
 * Login: xmatej52
 * School: VUT FIT, Brno
 * Date: 02-05-2016
 * Module: IOS-PROJ2 proj2_shared.c
 * gcc version 4.9.2 (Debian 4.9.2-10)
 */
#include <stdio.h>
#include "proj2.h"

/* Check if arguments are valid. */
int check_arguments(int argc, char **argv) {
	if (argc != 5) return 0;
	if (!is_uint(argv[1]) || !is_uint(argv[2]) || !is_uint(argv[3]) || !is_uint(argv[4]))
		return 0;
	return 1;
}

/* Save value of arguments into addresses. In case values are not valid,
   return 0 otherwise return 1. */
int get_arguments_values(int *P, int *C, int *PT, int *RT, char **argv) {
	*P = string_to_uint(argv[1]);
	if (*P <= 0) return 0;
	*C = string_to_uint(argv[2]);
	if (*C <= 0 || *C >= *P || *P % *C != 0) return 0;
	*PT = string_to_uint(argv[3]);
	if (*PT < 0 || *PT > 5000) return 0;
	*RT = string_to_uint(argv[4]);
	if (*RT < 0 || *RT > 5000) return 0;
	return 1;
}

/******************************************************************************/


/* Check if string stands for unsigned number. */
int is_uint(char *string) {
	int i=0;
	if (string[i] == '+') i++;
	for(; string[i] != '\0'; i++)
		if (string[i] < '0' || string[i] > '9')
			return 0;

	return 1;
}

/* Convert string into int or return -1 in case of overflow. */
int string_to_uint(char *string) {
	int long long value = 0;
	int test = 0;
	int i=0;
	if (string[i] == '+') i++;
	for (; string[i] != '\0'; i++){		
		/* testing overflow */
		test = value * 10 + ( string[i] - '0' );
		if (test < value) return -1;
		/* increasing value */
		value = value * 10 + ( string[i] - '0' );		
	}
	return(value);
}
