//Projekt: SoapCar
//Part: IMB - Intelligent Motor Bridge
//Head: Elias Dillinger
//Time Intervall: to long xD

#include "stm32f4xx.h"
#include <stdio.h>


int USART2_write(int c);
int USART2_read(void);
int32_t pulseCount = 0; //THIS NEEDS to be int32_t, otherwise it will overflow
//before the Interrupt kicks in-> took me serveral hours to find out xD

void delayMs(int n);
void USART2_init(void);

int main(void) {
    int n; //Counter Variable for UART2
    char str[80];//String for URART2
	
    USART2_init();
    printf("Test I/O functions by Printing: ELEKTROMINATI\r\n");
	
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
    TIM2->CR1 = 0x00010001;      // enable counter in HEX please (debug reason)

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
		//IMPORTANT
		pulseCount = 0; //resets counter^^ so we're ready for the next EXTI
		TIM2->CR1 = 0x0000; //DISABLE the Timer, so we can Synch & Enable the Timer with the next EXTI
	}
	
}

/* initialize USART2 to transmit at 9600 Baud */
void USART2_init (void) {
    RCC->AHB1ENR |= 1;          /* Enable GPIOA clock */
    RCC->APB1ENR |= 0x20000;    /* Enable USART2 clock */

    /* Configure PA2, PA3 for USART2 TX, RX */
    GPIOA->AFR[0] &= ~0xFF00;
    GPIOA->AFR[0] |=  0x7700;   /* alt7 for USART2 */
    GPIOA->MODER  &= ~0x00F0;
    GPIOA->MODER  |=  0x00A0;   /* enable alt. function for PA2, PA3 */

    USART2->BRR = 0x0683;       /* 9600 baud @ 16 MHz */
    USART2->CR1 = 0x000C;       /* enable Tx, Rx, 8-bit data */
    USART2->CR2 = 0x0000;       /* 1 stop bit */
    USART2->CR3 = 0x0000;       /* no flow control */
    USART2->CR1 |= 0x2000;      /* enable USART2 */
}

/* Write a character to USART2 */
int USART2_write (int ch) {
    while (!(USART2->SR & 0x0080)) {}   // wait until Tx buffer empty
    USART2->DR = (ch & 0xFF);
    return ch;
}

/* Read a character from USART2 */
int USART2_read(void) {
    while (!(USART2->SR & 0x0020)) {}   // wait until char arrives
    return USART2->DR;
}

/* The code below is the interface to the C standard I/O library.
 * All the I/O are directed to the console, which is UART3.
 */
//struct __FILE { int handle; };
FILE __stdin  = {0};
FILE __stdout = {1};
FILE __stderr = {2};

/* Called by C library console/file input
 * This function echoes the character received.
 * If the character is '\r', it is substituted by '\n'.
 */
int fgetc(FILE *f) {
    int c;

    c = USART2_read();      /* read the character from console */

    if (c == '\r') {        /* if '\r', after it is echoed, a '\n' is appended*/
        USART2_write(c);    /* echo */
        c = '\n';
    }

    USART2_write(c);        /* echo */

    return c;
}

/* Called by C library console/file output */
int fputc(int c, FILE *f) {
    return USART2_write(c);  /* write the character to console */
}




