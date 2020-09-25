#include "types.h"
#include "io.h"
// Data register of first UART (PL011)

/* need a fake array to get the pointer from the linker script */
extern char data_start[1];
extern char data_end[1];
extern char bss_start[1];
extern char bss_end[1];
extern char arm_vector_table[1];

size_t data[] = {1, 2, 3, 4, 5};

// Stop guest so Qemu terminates
void system_off(void);

// /**
//  * Copy the content from start to end to the dest
// */
// void copy(void *start, void *end, void *dest)
// {
// }

// /**
//  * Copy the content from start to end to the dest
// */
// void copy_size_t(size_t *start, size_t *end, size_t *dest)
// {
// 	end = (void *)((size_t)end & (!((1 << 8) - 1)));
// 	for (size_t *index = start; index != end; index++)
// 	{
// 		*index = *dest++;
// 	}
// }

// /**
//  * set all the content from start to end to zero
//  */
// void memClear(void *start, void *end)
// {
// }

int main()
{
	putPtr(arm_vector_table);
	print("\nHello world!\n");

	// turrning off the system
	// comment this to halt for human interupt
	system_off();
	return 0;
}
