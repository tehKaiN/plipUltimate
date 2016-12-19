#include <avr/io.h>
#define F_CPU 20000000L
#include <util/delay.h>

int main(void) {
	PORTC |= _BV(PC5);

	// Let Amiga give me a sign for flashing
	_delay_ms(5000);

	// No response from Ami - exit bootloader
	DDRC |= _BV(PC5);
	_delay_ms(20);
	DDRC = 0;
	PORTC &= ~_BV(PC5);
	asm("jmp 0000");
	return 0;
}
