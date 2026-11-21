#include <stm32f031x6.h>
#include "musical_notes.h"
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode);
void delay(volatile uint32_t dly);
void playNote(uint32_t Freq)
{
	// Counter is running at 65536 Hz
	TIM2->ARR = (uint32_t)65536/((uint32_t)Freq);
	TIM2->CCR1 = TIM2->ARR/2;
	TIM2->CNT = 0;
	TIM2->CCER |= (1 << 0);  // enable output
	TIM2->CR1  |= (1 << 0);  // start counter
}
void stopNote()
{
	TIM2->CR1  &= ~(1 << 0); // stop counter
	TIM2->CCER &= ~(1 << 0); // disable output so pin goes silent
}
void soundLifeLost()
{
	playNote(G4); delay(150); stopNote();
}

void soundGameOver()
{
	playNote(D5); delay(200); stopNote();
	playNote(G4); delay(200); stopNote();
	playNote(G3); delay(400); stopNote();
}

void initSound()
{
	// Power up TIM2 (APB1ENR bit 0)
	RCC->APB1ENR |= (1 << 0);
	pinMode(GPIOA, 0, 2); // Assign alternate function to PA0
	GPIOA->AFR[0] &= ~(0x0fu << 0); // clear AF bits for PA0
	GPIOA->AFR[0] |=  (2u   << 0);  // AF2 = TIM2_CH1 on PA0
	TIM2->CR1  = 0; // timer off
	TIM2->CCER = 0; // output disabled — keeps pin silent until playNote()
	TIM2->CCMR1 = (1 << 6) + (1 << 5); // PWM mode 1
	TIM2->PSC = 48000000UL/65536UL; // counter running at 65536 Hz
	TIM2->ARR = 65536/440; // default ARR (A4), not used until playNote()
	TIM2->CCR1 = TIM2->ARR/2;
	TIM2->CNT = 0;
}