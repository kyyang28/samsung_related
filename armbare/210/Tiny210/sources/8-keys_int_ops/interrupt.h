#ifndef __INTERRUPT_H
#define __INTERRUPT_H

/* Interrupt Sources */
#define GPH2CON							(*(volatile unsigned long *)0xE0200C40)
#define GPH2DAT							(*(volatile unsigned long *)0xE0200C44)
#define GPH3CON							(*(volatile unsigned long *)0xE0200C60)
#define GPH3DAT							(*(volatile unsigned long *)0xE0200C64)

#define EXT_INT_2_CON					(*(volatile unsigned long *)0xE0200E08)
#define EXT_INT_3_CON					(*(volatile unsigned long *)0xE0200E0C)

#define EXT_INT_2_MASK					(*(volatile unsigned long *)0xE0200F08)
#define EXT_INT_3_MASK					(*(volatile unsigned long *)0xE0200F0C)

#define EXT_INT_2_PEND					(*(volatile unsigned long *)0xE0200F48)
#define EXT_INT_3_PEND					(*(volatile unsigned long *)0xE0200F4C)

/* Interrupt Controller */
/* VIC0 */
#define VIC0VECTADDR_BASE				(0xF2000100)
#define VIC0IRQSTATUS					(*(volatile unsigned long *)0xF2000000)
#define VIC0INTSELECT					(*(volatile unsigned long *)0xF200000C)
#define VIC0INTENABLE					(*(volatile unsigned long *)0xF2000010)
#define VIC0INTENCLEAR					(*(volatile unsigned long *)0xF2000014)
#define VIC0VECTADDR16					(*(volatile unsigned long *)0xF2000140)
#define VIC0ADDRESS						(*(volatile unsigned long *)0xF2000F00)

/* VIC1 */
#define VIC1VECTADDR_BASE				(0xF2100100)
#define VIC1IRQSTATUS					(*(volatile unsigned long *)0xF2100000)
#define VIC1INTSELECT					(*(volatile unsigned long *)0xF210000C)
#define VIC1INTENABLE					(*(volatile unsigned long *)0xF2100010)
#define VIC1INTENCLEAR					(*(volatile unsigned long *)0xF2100014)
#define VIC1VECTADDR16					(*(volatile unsigned long *)0xF2100140)
#define VIC1ADDRESS						(*(volatile unsigned long *)0xF2100F00)

/* VIC2 */
#define VIC2VECTADDR_BASE				(0xF2200100)
#define VIC2IRQSTATUS					(*(volatile unsigned long *)0xF2200000)
#define VIC2INTSELECT					(*(volatile unsigned long *)0xF220000C)
#define VIC2INTENABLE					(*(volatile unsigned long *)0xF2200010)
#define VIC2INTENCLEAR					(*(volatile unsigned long *)0xF2200014)
#define VIC2VECTADDR16					(*(volatile unsigned long *)0xF2200140)
#define VIC2ADDRESS						(*(volatile unsigned long *)0xF2200F00)

/* VIC3 */
#define VIC3VECTADDR_BASE				(0xF2300100)
#define VIC3IRQSTATUS					(*(volatile unsigned long *)0xF2300000)
#define VIC3INTSELECT					(*(volatile unsigned long *)0xF230000C)
#define VIC3INTENABLE					(*(volatile unsigned long *)0xF2300010)
#define VIC3INTENCLEAR					(*(volatile unsigned long *)0xF2300014)
#define VIC3VECTADDR16					(*(volatile unsigned long *)0xF2300140)
#define VIC3ADDRESS						(*(volatile unsigned long *)0xF2300F00)

#define EINT_16_31						(16)

#define EXCEPTION_VECTOR_ENTRY_ADDR		(0xD0037400)
#define EXCEPTION_RESET_ENTRY			(*(volatile unsigned long *)(EXCEPTION_VECTOR_ENTRY_ADDR + 0x00))
#define EXCEPTION_UNDEF_ENTRY			(*(volatile unsigned long *)(EXCEPTION_VECTOR_ENTRY_ADDR + 0x04))
#define EXCEPTION_SWI_ENTRY				(*(volatile unsigned long *)(EXCEPTION_VECTOR_ENTRY_ADDR + 0x08))
#define EXCEPTION_PREFETCHABORT_ENTRY	(*(volatile unsigned long *)(EXCEPTION_VECTOR_ENTRY_ADDR + 0x0C))
#define EXCEPTION_DATAABORT_ENTRY		(*(volatile unsigned long *)(EXCEPTION_VECTOR_ENTRY_ADDR + 0x10))
#define EXCEPTION_RESERVED_ENTRY		(*(volatile unsigned long *)(EXCEPTION_VECTOR_ENTRY_ADDR + 0x14))
#define EXCEPTION_IRQ_ENTRY				(*(volatile unsigned long *)(EXCEPTION_VECTOR_ENTRY_ADDR + 0x18))
#define EXCEPTION_FIQ_ENTRY				(*(volatile unsigned long *)(EXCEPTION_VECTOR_ENTRY_ADDR + 0x1C))


int interrupt_init_ll(void);
void clearVecAddress(void);

#endif

