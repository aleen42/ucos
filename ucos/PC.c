
#include  "includes.h"
#include  <string.h>
/*
*********************************************************************************************************
*                                           x86-protect
*********************************************************************************************************
*/
#define PIT_BASE	0x40
#define PIT_T0		0x00		/* PIT channel 0 count/status */
#define PIT_T1		0x01		/* PIT channel 1 count/status */
#define PIT_T2		0x02		/* PIT channel 2 count/status */
#define PIT_COMMAND	0x03		/* PIT mode control, latch and read back */

/* PIT Command Register Bit Definitions */

#define PIT_CMD_CTR0	0x00		/* Select PIT counter 0 */
#define PIT_CMD_CTR1	0x40		/* Select PIT counter 1 */
#define PIT_CMD_CTR2	0x80		/* Select PIT counter 2 */

#define PIT_CMD_LATCH	0x00		/* Counter Latch Command */
#define PIT_CMD_LOW	0x10		/* Access counter bits 7-0 */
#define PIT_CMD_HIGH	0x20		/* Access counter bits 15-8 */
#define PIT_CMD_BOTH	0x30		/* Access counter bits 15-0 in two accesses */

#define PIT_CMD_MODE0	0x00		/* Select mode 0 */
#define PIT_CMD_MODE1	0x02		/* Select mode 1 */
#define PIT_CMD_MODE2	0x04		/* Select mode 2 */
#define PIT_CMD_MODE3	0x06		/* Select mode 3 */
#define PIT_CMD_MODE4	0x08		/* Select mode 4 */
#define PIT_CMD_MODE5	0x0A		/* Select mode 5 */

#define TIMER0_VALUE 0x04aa /* 1kHz 1.9318MHz / 1000 */
#define TIMER2_VALUE 0x0a8e /* 440Hz */

// Intel 8259 ports
#define I8259_A0	0x020				// 8259 #1, port #1
#define I8259_A1	0x021				// 8259 #1, port #2
#define I8259_B0	0x0a0				// 8259 #2, port #1
#define I8259_B1	0x0a1				// 8259 #2, port #2

#define ucos_i386_outport_byte( _port, _value ) \
do { register unsigned short __port  = _port; \
     register unsigned char  __value = _value; \
     \
     asm volatile ( "outb %0,%1" : : "a" (__value), "d" (__port) ); \
   } while (0)


#define ucos_i386_inport_byte( _port, _value ) \
do { register unsigned short __port  = _port; \
     register unsigned char  __value = 0; \
     \
     asm volatile ( "inb %1,%0" : "=a" (__value) \
                                : "d"  (__port) \
                  ); \
     _value = __value; \
   } while (0)

#define ucBSP_OutByte( _port, _value ) ucos_i386_outport_byte( _port, _value )
#define ucBSP_InByte( _port, _value )  ucos_i386_inport_byte( _port, _value )

//在IDT表中设置中断处理例程
volatile void ucos_x86_idt_set_handler_asm()
{
	__asm__ __volatile__(".global ucos_x86_idt_set_handler\n\t"
		"ucos_x86_idt_set_handler:\n\t"
	        "pushl    %%ebp\n\t"
	        "movl     %%esp, %%ebp\n\t"
	        "pushw    %%fs\n\t"
	        "pushl    %%ebx\n\t"
	        "pushl    %%eax\n\t"
	        "movw     %0, %%bx\n\t"
	        "movw     %%bx, %%fs\n\t"
	        "movl     8(%%ebp), %%ebx\n\t"            /* get interrupt number*/
	        "shll     $3,%%ebx\n\t"                 /* *8, to get IDT offset*/
	        "movl     12(%%ebp), %%eax\n\t"            /* get offset   */
	        "movw     %%ax, %%fs:0(%%ebx)\n\t"
	        "shrl     $16, %%eax\n\t"
	        "movw     %%ax, %%fs:6(%%ebx)\n\t"
	        "movw     $0x18, %%ax\n\t"
	        "movw     %%ax, %%fs:2(%%ebx)\n\t"
	        "movw     16(%%ebp), %%ax\n\t"                /* get access word*/
	        "movw     %%ax, %%fs:4(%%ebx)\n\t"
	        "popl     %%eax\n\t"
	        "popl     %%ebx\n\t"
	        "popw     %%fs\n\t"
	        "popl     %%ebp\n\t"
		"ret"
		:
		:"i"(0x10));
}

void InitPIC()
{
	// Reprogram the master 8259.
	ucBSP_OutByte(I8259_A0, 0x11);

	ucBSP_OutByte(I8259_A1, 0x20);
	ucBSP_OutByte(I8259_A1, 0x04);
	ucBSP_OutByte(I8259_A1, 0x01);
	ucBSP_OutByte(I8259_A1, 0x00);

	// Reprogram the slave 8259.
	ucBSP_OutByte(I8259_B0, 0x11);

	ucBSP_OutByte(I8259_B1, 0x28);
	ucBSP_OutByte(I8259_B1, 0x02);
	ucBSP_OutByte(I8259_B1, 0x01);
	ucBSP_OutByte(I8259_B1, 0x00);
}

void ucos_timer_init(void)
{

	ucBSP_OutByte(PIT_CMD_CTR0|PIT_CMD_BOTH|PIT_CMD_MODE2, PIT_BASE + PIT_COMMAND);
	ucBSP_OutByte(TIMER0_VALUE&0xff, PIT_BASE + PIT_T0);
	ucBSP_OutByte(TIMER0_VALUE>>8, PIT_BASE + PIT_T0);	
}

void OSCpuInit(void)
{
	int i;
	extern 	DefIntHandler;
	InitPIC();

	//ucos_x86_idt_set_handler(0x20,(void *)OSTickISR,0x8e00);
	ucos_x86_idt_set_handler(0x80,(void *)OSCtxSw,0x8e00);			//OSCtxSw	
}
