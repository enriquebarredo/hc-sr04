/*
 * HCSR05 Driver.c
 *
 * Created: 5/11/2019 5:35:20 AM
 * Author : Enrique
 */ 

#define F_CPU 1000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile float t_total = 20000; // ancho de pulso, si inicia en cero, puede que empieze con una interrupción si está suficientemente lejos.
volatile int flanco = 1; // Bandera, 1 es flanco de subida, 0 es flanco de bajada.

int main(void)
{
	//Pin del echo, resistencia pull-up para cancelacion ruido
	DDRB	&=	~(1<<DDB0);
	PORTB	|=	1 << PINB0;

	//Pin del trigger
	DDRB	|=	1 << DDB1;

	//Pin del LED comparador
	DDRC	|=	1 << DDC0;

	TCCR1B |= (1<< ICNC1)|(1<<ICES1)|(1<<CS10); // CS10: sin prescaler, ICNC1: eliminación de ruido, ICES1 = 1: la interrupcion es en el flanco de subida
	TIMSK1 |= (1 << ICIE1); // habilita input capture unit interrupt
	

	sei();			//interrupciones globales habilitadas
	while (1)
    {
				//		pulso de trigger	__/¨¨¨¨¨\__
		PORTB &= ~(1 << PORTB1);
		_delay_us(2);
		PORTB |= (1 << PORTB1);
		_delay_us(11);				//ancho de pulso deal menos 10 microsegundos
		PORTB &= ~(1 << PORTB1);
		_delay_us(2);
		
		if (t_total < 8000)
		{
			PORTC |= (1<<PORTC0);
		}
		else
		{
			PORTC &= ~(1<<PORTC0);
		}
		_delay_ms(60);
	}
}

ISR(TIMER1_CAPT_vect)
{
	uint16_t TCNT1_copy = TCNT1;		//Inmediatamente copia TCNT1

	if (flanco)	// Si la interrupción se generó en el flanco de subida...
	{
		TCCR1B &= ~(1 << ICES1);	//Cambia al flanco de bajada
		TCNT1 = 0;					//Resetea el contador
	}
	else        // Si la interrupción se generó en glanco de bajada
	{
		TCCR1B |= (1 << ICES1);		//Cambia al flanco de subida
		t_total = TCNT1_copy;	//como elcontador siempre es cero en el flanco de bajada, no hay necesidad de restarle lo del tiempo de inicio (siempre es cero)
	}
	flanco = !flanco;	// cambia la bandera del flanco
}