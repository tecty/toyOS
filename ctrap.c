#include "io.h"
#include "asm.h"
void invalid_vector_entry(void)
{
    print("Invalid Entry\n");
    system_off();
}
void cur_el_sync(void)
{
}
void cur_el_irq(void)
{
}
void cur_el_serr(void)
{
}
void lower_el_sync(void)
{
}
void lower_el_irq(void)
{
}
void lower_el_serr(void)
{
}