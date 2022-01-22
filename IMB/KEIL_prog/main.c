
/* p6_1.c using SW0 interrupt
 *
 * User Switch B1 is used to generate interrupt through PC13.
 * The user button is connected to PC13. It has a pull-up resitor 
 * so PC13 stays high when the button is not pressed.
 * When the button is pressed, PC13 becomes low.
 * The falling-edge of PC13 (when switch is pressed) triggers an
 * interrupt from External Interrupt Controller (EXTI).
 * In the interrupt handler, the Timer 2 is started.
 * The green LED is on after 4ms, stays on for 2 ms and after that its off.
 * It serves as a crude way to debounce the switch.
 * The green LED (LD2) is connected to PA5.
 *
 * This program was tested with Keil uVision v5.24a with DFP v2.11.0.
 */

#include "stm32f4xx.h"
int8_t pulseCount = 0;
void delayMs(int n);

int main(void) {
    __disable_irq();                    /* global disable IRQs */

    RCC->AHB1ENR |= 4;	                /* enable GPIOC clock */
    RCC->AHB1ENR |= 1;                  /* enable GPIOA clock */
    RCC->APB2ENR |= 0x4000;             /* enable SYSCFG clock */

    /* configure PA5 for LED */
    GPIOA->MODER &= ~0x00000C00;        /* clear pin mode */
    GPIOA->MODER |=  0x00000400;        /* set pin to output mode */

    /* configure PC13 for push button interrupt */
    GPIOC->MODER &= ~0x0C000000;        /* clear pin mode to input mode */
    
    SYSCFG->EXTICR[3] &= ~0x00F0;       /* clear port selection for EXTI13 */
    SYSCFG->EXTICR[3] |= 0x0020;        /* select port C for EXTI13 */
    
    EXTI->IMR |= 0x2000;                /* unmask EXTI13 */
    EXTI->FTSR |= 0x2000;               /* select falling edge trigger */

//    NVIC->ISER[1] = 0x00000100;         /* enable IRQ40 (bit 8 of ISER[1]) */
    NVIC_EnableIRQ(EXTI15_10_IRQn);
    
    __enable_irq();                     /* global enable IRQs */
    
    while(1) {
    }
}

void EXTI15_10_IRQHandler(void) {
    
    /* setup TIM2
    Clock by default at 16 MHz*/
    RCC->APB1ENR |= 1;              /* enable TIM2 clock */
    TIM2->PSC = 160 - 1;          // divided by 160-> f = 100 kHz
    TIM2->ARR = 5 - 1;           // divided by 5-> f2 = 10 kHz max! Res = 50 µs-> 1/2 Period time
    TIM2->CR1 = 1;                  /* enable counter */

    TIM2->DIER |= 1;                /* enable UIE */
    NVIC_EnableIRQ(TIM2_IRQn);      /* enable interrupt in NVIC */

        EXTI->PR = 0x2000;          /* clear interrupt pending flag */
}

/* 16 MHz SYSCLK */
void delayMs(int n) {
    int i;
    for (; n > 0; n--)
        for (i = 0; i < 3195; i++) ;
}


void TIM2_IRQHandler(void) {
    TIM2->SR = 0;                   /* clear UIF */
  //GPIOA->ODR ^= 0x20;				/* toggle LED */
	//Incoming f for EXTI 50 Hz; detection f = 20 kHz
	pulseCount += 1;//raise the value every time the Handler starts-> every 1ms
	if(pulseCount == 80 - 1){//after 4ms-> 80-1 because we start to count form 0
		GPIOA->BSRR = 0x00000020;   /* turn on LED */
	}
	if(pulseCount == 100 - 1){//after 5ms-> 100-1 because we start to count form 0
		GPIOA->BSRR = 0x00200000;   /* turn off LED */
	}

}



