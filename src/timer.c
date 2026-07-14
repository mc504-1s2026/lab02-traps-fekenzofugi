#include <arch/timer.h>
#include <kernel/panic.h>
#include <arch/csr.h>

static volatile bool alarm_on;
static volatile u64 next_alarm;

u64 timer_read()
{
	/* implemented */
	return csr_read(CSR_TIME);
}

void timer_irq_enable()
{
	/* implemented */
	csr_write(CSR_STIMECMP, timer_read() + 1 * TIMER_FREQ); // gera interrupção no tempo de agora + 1 segundo
	csr_set(CSR_SIE, CSR_SIE_STIE);
	hart_irq_enable();
}

void timer_irq_disable()
{
	/* implemented */
	csr_clear(CSR_SIE, CSR_SIE_STIE);
}

void timer_set_alarm(u64 secs)
{
	/* implemented */
	next_alarm = (secs * TIMER_FREQ) + timer_read();
	alarm_on = true;
}

void timer_irq()
{
	/* implemented */
	if (alarm_on == true) {
		if (next_alarm <= timer_read()) {
            alarm_on = false;
            serial_puts("alarm\r\n");
        }
	}
	csr_write(CSR_STIMECMP, timer_read() + 1 * TIMER_FREQ); // gera interrupção no tempo de agora + 1 segundo
}
