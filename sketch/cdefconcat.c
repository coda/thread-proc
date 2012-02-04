#include <stdio.h>

#define def(x) "hello world: " #x

int main(int argc, char * argv[])
{
	printf(def(hello) "\nand " def() "and hi\n");
	return 0;
}
