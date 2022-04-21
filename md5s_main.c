/* 
 * Operating Systems (2INC0) Practical Assignment
 * Interprocess Communication
 *
 * Auxiliary program to create your own hash codes 
 * (such that they can be pasted into settings.h)
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "md5s.h"

 
int 
main (int argc, char * argv[])
{
	char	min = 0x7f;	
	char	max = 0x0;
	
    if (argc == 1)
	{
		fprintf (stderr, "Usage: %s [ <string> ]*\n", argv[0]);
	}
	else
	{
		printf("static uint128_t md5_list[] =\n");
		printf("{\n");
		
		// calculate MD5 hash for each argument
		for (int j = 1; j < argc; j++)
		{
			uint128_t	hash;
			
			hash = md5s (argv[j], strlen (argv[j]));
			printf("    UINT128(0x%016lx,0x%016lx),    /* '%s' */\n", HI(hash), LO(hash), argv[j]);
			
			// find the alphabet-boundaries
			for (int k = 0; k < strlen(argv[j]); k++)
			{
				char c = argv[j][k];
				if (c < min) min = c;
				if (c > max) max = c;
			}
		}
		printf("};\n\n");
		printf("#define ALPHABET_START_CHAR '%c'\n", min);
		printf("#define ALPHABET_END_CHAR   '%c'\n", max);	
	}
    return 0;
}


