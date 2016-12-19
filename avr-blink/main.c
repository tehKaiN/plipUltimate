#define F_CPU 20000000UL

#include <avr/io.h>
#include <util/delay.h>

int main(void) {

    // Insert code
    DDRC = _BV(PC5);
    PORTC |= _BV(PC5);

    while(1) {
			DDRC ^= _BV(PC5);
			_delay_ms(1000);
    }

    return 0;
}

// zolty   czerw
// pomaran braz
// ziel    czarn
