#include <kernel/printf.h>
#include <kernel/mm.h>
#include <arch/timer.h>
#include <kernel/trap.h>
#include <kernel/serial.h>
#include <kernel/string.h>

#define BUFFER_SIZE 256
#define SHELL_COMMAND_BUFFER_SIZE 256

void echo_command(char *arg) {
	serial_puts(arg);
	serial_puts("\r\n");
}

void alarm_command(char *arg) {
	u64 secs = strtou64(arg, 10);
	timer_set_alarm(secs);
}

void uptime_command() {
	char buf[32];
	u64 secs = timer_read() / TIMER_FREQ;
	snprintf(buf, sizeof(buf), "%lus\r\n", secs);
	serial_puts(buf);
}

void not_found_command(char *arg) {
	serial_puts("Error: Command not available: ");
	serial_puts(arg);
	serial_puts("\r\n");
}

void command_router(char *command) {
	if (command[0] == '\0') {
		return;
	} else if (strncmp(command, "echo ", 5) == 0) {
		echo_command(command + 5);
	} else if (strcmp(command, "echo") == 0) {
		echo_command("");
	} else if (strncmp(command, "alarm ", 6) == 0) {
		alarm_command(command + 6);
	} else if (strcmp(command, "uptime") == 0) {
		uptime_command();
	} else {
		not_found_command(command);
	}
}

void shell()
{
	char cmd_buf[SHELL_COMMAND_BUFFER_SIZE];
	char buffer[BUFFER_SIZE];
	size_t cmd_len = 0;

	serial_puts("> "); // prompt inicial

	while(1) {
		size_t n = serial_read(buffer);
		for (size_t i = 0; i < n; i++) {
			char c = buffer[i];
			if (c == '\r' || c == '\n') {
				serial_puts("\r\n");
				cmd_buf[cmd_len] = '\0';
				command_router(cmd_buf);
				cmd_len = 0;
				serial_puts("> ");
			} else if (cmd_len < SHELL_COMMAND_BUFFER_SIZE - 1) {
				cmd_buf[cmd_len++] = c;
				serial_putc(c);
			}
		}
	}
}

extern int _hartid[];
void kmain()
{
	printk_set_level(LOG_DEBUG);
	info("entered S-mode\n");
	info("booting on hart %d\n", _hartid[0]);
	info("setting up virtual memory...\n");
	vm_init();

	info("enabling traps...\n");
	trap_setup();
	info("enabling timer...\n");
	timer_irq_enable();
	info("enabling serial...\n");
	serial_init();
	serial_irq_enable();

	/* implement your shell here */
	shell();
}
