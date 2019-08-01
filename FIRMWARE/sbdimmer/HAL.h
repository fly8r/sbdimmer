/*
 * HAL.h
 *
 * Created: 01.04.2019 11:53:01
 *  Author: fly8r
 */
#ifndef HAL_H_
#define HAL_H_

#include <avr/io.h>

//------------------------------ Macros
#define HI(x)							((x)>>8)
#define LO(x)							((x)& 0xFF)
#define STATE_ON						1
#define STATE_OFF						0

//------------------------------ System timer configuration for 1mS interrupt
#define SYSTICK_TIME_MS					1
#define SYSTICK_PRESCALER				64UL
#define SYSTICK_OCR_CONST				((F_CPU*SYSTICK_TIME_MS) / (SYSTICK_PRESCALER*1000))
#define SYSTICK_TIMER_COUNTER           TCNT1
#define SYSTICK_TIMER_OCR               OCR1A
#define SYSTICK_TIMER_INIT()            { TCCR1A=0; TCCR1B=(1<<WGM12|1<<CS11|1<<CS10); SYSTICK_TIMER_OCR=SYSTICK_OCR_CONST; SYSTICK_TIMER_COUNTER=0; }
#define SYSTICK_INTERRUPT_ENABLE()      { TIMSK |= 1<<OCIE1A; }
#define SYSTICK_INTERRUPT_DISABLE()     { TIMSK &= ~(1<<OCIE1A); }

//------------------------------ PWM configuration
#define PWM_TIMER_COUNT_REG				TCNT0
#define PWM_TIMER_OCR					OCR0A
#define PWM_DDR							DDRB
#define PWM_PORT						PORTB
#define PWM_MASK						(1<<2)
#define PWM_INIT()						{ PWM_DDR|=PWM_MASK; TCCR0A=(1<<WGM01|1<<WGM00); TCCR0B=(1<<CS00); }
#define PWM_SET_LEVEL(lvl)				{ PWM_TIMER_OCR=lvl; PWM_TIMER_COUNT_REG=0; }
#define PWM_ON(lvl)						{ PWM_SET_LEVEL(lvl); TCCR0A|=(1<<COM0A1); }
#define PWM_OFF()						{ TCCR0A&=~(1<<COM0A1);  }

//------------------------------ Encoder configuration
#define ENC_DDR							DDRD
#define ENC_PIN							PIND
#define ENC_MASK						(1<<0|1<<1)
#define ENC_INIT()						{ ENC_DDR &= ~(ENC_MASK); }
#define ENC_STATE						(ENC_PIN & ENC_MASK)

//------------------------------ Encoder button configuration
#define BTN_DDR							DDRD
#define BTN_PIN							PIND
#define BTN_MASK						(1<<2)
#define BTN_INIT()						{ BTN_DDR &= ~(BTN_MASK); }
#define BTN_PRESSED						(!(BTN_PIN & BTN_MASK))



#endif /* HAL_H_ */