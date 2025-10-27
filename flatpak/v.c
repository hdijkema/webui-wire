#include <stdio.h>

typedef int bool;

#include "../webui_wire.h"

int main()
{
	printf("v%s\n", WEB_WIRE_VERSION);
	return 0;
}
