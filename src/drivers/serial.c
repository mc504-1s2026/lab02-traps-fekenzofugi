#include <kernel/trap.h>
#include <arch/io.h>
#include <arch/plic.h>
#include <arch/spinlock.h>
#include <arch/csr.h>
#include <kernel/serial.h>
#include <kernel/panic.h>

#define SERIAL_BUFFER_SIZE 256

struct spinlock serial_lock; 
char serial_buffer[256]; 
size_t serial_buffer_len;

static inline u8 serial_reg_read(u64 reg)
{
	return ioread8((void *)((u64)SERIAL_BASE + reg));
}

static inline void serial_reg_write(u64 reg, u8 val)
{
	iowrite8(val, (void *)((u64)SERIAL_BASE + reg));
}

void serial_init()
{
	/* implemented */
	spin_init(&serial_lock); // lock
	serial_buffer_len = 0; // buffer vazio
	serial_reg_write(SERIAL_IER, 0); // todas interrupcoes desativadas
	serial_reg_write(SERIAL_LCR, 0x3);
	serial_reg_write(SERIAL_FCR, SERIAL_FCR_FIFO_ENABLE | SERIAL_FCR_RX_FIFO_CLEAR | SERIAL_FCR_TX_FIFO_CLEAR);
	serial_reg_write(SERIAL_IER, SERIAL_IER_ERBFI);
}

void serial_irq_enable()
{
	/* implemented */
	plic_irq_set_priority(IRQ_SERIAL, 1);
	plic_hart_enable_irq(0, IRQ_SERIAL);
	plic_hart_set_threshold(0, 0);
	csr_set(CSR_SIE, CSR_SIE_SEIE);
	hart_irq_enable();
}

void serial_irq_disable()
{
	/* implemented */
	csr_clear(CSR_SIE, CSR_SIE_SEIE);
}

void serial_irq()
{
	/* implemented */
	spin_lock(&serial_lock);

	while (serial_reg_read(SERIAL_LSR) & SERIAL_LSR_DTR) {
		u8 c = serial_reg_read(SERIAL_RBR);

		if (serial_buffer_len >= SERIAL_BUFFER_SIZE) {
			continue;
		}

		serial_buffer[serial_buffer_len++] = (char)c;
	}
	spin_unlock(&serial_lock);
}

size_t serial_read(char *buf)
{
	/* implemented */
	u64 flags;
	size_t size;

	flags = spin_lock_irqsave(&serial_lock);

	size = serial_buffer_len;
	for (size_t i = 0; i < size; i++) {
		buf[i] = serial_buffer[i];
	}
	serial_buffer_len = 0;

	spin_unlock_irqrestore(&serial_lock, flags);

	return size;
}

void serial_puts(char *str)
{
	/* implemented */
	for (int i = 0; str[i] != '\0'; i++) {
			if (str[i] == '\n') {
				serial_putc('\r');
			}
			serial_putc(str[i]);
	}
}

void serial_putc(char c)
{
	/* implemented */
	u8 status;
    do {
        status = serial_reg_read(SERIAL_LSR);
    } while ((status & SERIAL_LSR_THRE) == 0);

 	serial_reg_write(SERIAL_THR, (u8)c); // Envia o caractere
}
