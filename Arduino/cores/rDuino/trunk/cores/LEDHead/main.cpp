#include <WProgram.h>

int main(void) __attribute__ ((OS_main));
int main(void)
{
	init();

	setup();
    
	for (;;)
		loop();
        
	return 0;
}

