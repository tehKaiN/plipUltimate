#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#define F_CPU 20000000
#include <util/delay.h>

int main(void) {
	// Disable watchdog & interrupts
	cli();
	wdt_reset();
	MCUSR=0;
	WDTCSR|=_BV(WDCE) | _BV(WDE);
	WDTCSR=0;

	// Let Amiga give me a sign for flashing
	PORTC = _BV(PC5);
	_delay_ms(3000);

	// No response from Ami - exit bootloader
	DDRC |= _BV(PC5);
	_delay_ms(20);
	DDRC = 0;
	PORTC &= ~_BV(PC5);
	asm("jmp 0000");
	return 0;
}
