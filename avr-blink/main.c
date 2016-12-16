#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>

int main(void) {

    // Insert code
    DDRC = _BV(PD5);

    while(1) {
			PORTC ^= _BV(PD5);
			_delay_ms(1000);
    }

    return 0;
}
