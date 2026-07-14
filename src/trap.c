#include <kernel/trap.h>
#include <kernel/panic.h>
#include <kernel/printf.h>
#include <kernel/serial.h>
#include <arch/csr.h>
#include <arch/timer.h>
#include <arch/plic.h>

/* defined in src/trap_entry.S */
extern void trap_entry();

void handle_irq(u64 cause)
{
	/* implemented */
	switch (cause) {
		case TRAP_TIMER_IRQ:
			timer_irq();
			break;
		case TRAP_EXTERNAL_IRQ:
			u32 irq = plic_hart_claim_irq(0);
			switch (irq) {
				case IRQ_SERIAL:
					serial_irq();
					break;
				case 0:
					return;
				default:
					break;
			}
			plic_hart_complete_irq(0, irq);
			break;
		default:
			break;
	}
}

void handle_exception(u64 cause)
{
	/* implemented */
	u64 stval = csr_read(CSR_STVAL); // endereco onde quebrou
	u64 sepc = csr_read(CSR_SEPC); // linha onde quebrou

	/* Erros definidos no trap.h */
	switch (cause) {
		case EXCEPTION_INST_ACCESS_FAULT:
			error("Error Access Fault at line %p addr %p\n", sepc, stval);
			break;
		case EXCEPTION_INST_PAGE_FAULT:
			error("Error Page fault at line %p addr %p\n", sepc, stval);
			break;
		case EXCEPTION_LOAD_ACCESS_FAULT:
			error("Error Load Access fault at line %p addr %p\n", sepc, stval);
			break;
		case EXCEPTION_LOAD_PAGE_FAULT:
			error("Error Load Page fault at line %p addr %p\n", sepc, stval);
			break;
		case EXCEPTION_STORE_ACCESS_FAULT:
			error("Error Store Access fault at line %p addr %p\n", sepc, stval);
			break;
		case EXCEPTION_STORE_PAGE_FAULT:
			error("Error Store Page fault at line %p addr %p\n", sepc, stval);
			break;
		default:
			error("Error Not Defined Exception %lu at line %p addr %p\n", cause, sepc, stval);
			break;
	}
	panic("unhandled exception (scause=%#lx)\n", cause);
}

void trap_setup()
{
	/* implemented */
	hart_irq_disable();
	
	// stvec CSR como indicado no trap_entry.S
	csr_write(CSR_STVEC, (u64)trap_entry);
}

void handle_trap()
{
	/* implemented */
	u64 cause = csr_read(CSR_SCAUSE);
	if (cause & TRAP_IRQ_BIT)
		handle_irq(cause);
	else
		handle_exception(cause);
}

void hart_irq_enable()
{
	/* implemented */
	csr_set(CSR_SSTATUS, CSR_SSTATUS_SIE);
}

u64 hart_irq_save()
{
	/* implemented */
	u64 sstatus = csr_read(CSR_SSTATUS); // salva estado atual

	hart_irq_disable(); // desativa interrupcoes

	return sstatus & CSR_SSTATUS_SIE;
}

void hart_irq_restore(u64 flags)
{
	/* implemented */
	if (flags & CSR_SSTATUS_SIE) {
        hart_irq_enable();
    } else {
        hart_irq_disable();
    }
}

void hart_irq_disable()
{
	/* implemented */
	csr_clear(CSR_SSTATUS, CSR_SSTATUS_SIE);
}
