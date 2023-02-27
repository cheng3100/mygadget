#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char *t[4] = {
	"",
	"",
	"avc",
	NULL,
};

int main() {
	/** for (int i=0; i<4; i++) { */
	/**     printf("the string %s val is %x and %x\n", t[i], t[i], &t[i]); */
	/** } */
	/** if (t[2]=="avc") */
	/**     printf("empty str"); */
    /**  */
	/** char *t1 = calloc(4, 1); */
	/** char *t2 = malloc(4); */
    /**  */
	/** for (int i=0; i<4; i++) */
	/**     printf("0x%x ", *t1++); */
	/** for (int i=0; i<4; i++) */
	/**     printf("0x%x ", *t2++); */

	char s[] = "abc-=-def";
	char *x=NULL;
	x = strtok(s, "-");
	printf("%s,", x);
	x=strtok(NULL, "-=");
	printf("%s,", x);
	x=strtok(NULL, "=");
	printf("%s,", x);
	printf("\n%s", s);

	int len = strcspn("geeks-for geeks","-x");
   printf("Length of initial segment matching : %d\n", len );
   printf("%s\n", strtok("geeks-for geeks", "-x"));
}
