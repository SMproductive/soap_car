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

int period;
int frequency;

int main(void) {
	  int last = 0;
    int current;
    int n; //Counter Variable for UART2
    char str[80];//String for URART2
	  int num = 123;
	  char StrP[5];
	  char StrF[5];
	
    USART2_init();
    printf("Test I/O functions by Printing: ELEKTROMINATI\r\n");
	  printf("        X        Elektrominiati:\r\n");
	  printf("       X X       make.\r\n");
	  printf("      X _ X      something.\r\n");
	  printf("     X |_| X     cool.\r\n");
	  printf("    X_______X    today.\r\n");
	
	
	
    __disable_irq();                    /* global disable IRQs */

    RCC->AHB1ENR |= 4;	                /* enable GPIOC clock */
    RCC->AHB1ENR |= 1;                  /* enable GPIOA clock */
    RCC->APB2ENR |= 0x4000;             /* enable SYSCFG clock */

    /* configure PA5 for LED */
    GPIOA->MODER &= ~0x00000C00;        /* clear pin mode */
    GPIOA->MODER |=  0x00000400;        /* set pin to output mode */
	
	
    // setup TIM4 
		RCC->APB1ENR |= 4;
    TIM4->PSC = 1600 - 1;           /* divided by 1600 */
    TIM4->ARR = 10000 - 1;          /* divided by 10000 */
    TIM4->CNT = 0;                  /* clear timer counter */
    TIM4->CR1 = 1;                  /* enable TIM4 */
		
		TIM4->DIER |= 1;                /* enable UIE */
    NVIC_EnableIRQ(TIM4_IRQn);      /* enable interrupt in NVIC */

    /* configure PC13 for push button interrupt */
    GPIOC->MODER &= ~0x0C000000;        /* clear pin mode to input mode */
    
    SYSCFG->EXTICR[3] &= ~0x00F0;       /* clear port selection for EXTI13 */
    SYSCFG->EXTICR[3] |= 0x0020;        /* select port C for EXTI13 */
    
    EXTI->IMR |= 0x2000;                /* unmask EXTI13 */
    EXTI->FTSR |= 0x2000;               /* select falling edge trigger */

//    NVIC->ISER[1] = 0x00000100;         /* enable IRQ40 (bit 8 of ISER[1]) */
    NVIC_EnableIRQ(EXTI15_10_IRQn);
    
    __enable_irq();                     /* global enable IRQs */
		
		
		 // configure PA6 as input of TIM3 CH1
    RCC->AHB1ENR |=  1;             /* enable GPIOA clock */
    GPIOA->MODER &= ~0x00003000;    /* clear pin mode */
    GPIOA->MODER |=  0x00002000;    /* set pin to alternate function */
    GPIOA->AFR[0] &= ~0x0F000000;   /* clear pin AF bits */
    GPIOA->AFR[0] |= 0x02000000;    /* set pin to AF2 for TIM3 CH1 */

    // configure TIM3 to do input capture with prescaler ...
    RCC->APB1ENR |= 2;              /* enable TIM3 clock */
    TIM3->PSC = 16000 - 1;          /* divided by 16000 */
    TIM3->CCMR1 = 0x41;             /* set CH1 to capture at every edge */
    TIM3->CCER = 0x0B;              /* enable CH 1 capture both edges */
    TIM3->CR1 = 1;                  /* enable TIM3 */
    
    while(1) {
			while (!(TIM3->SR & 2)) {}  // wait until input edge is captured
        current = TIM3->CCR1;       // read captured counter value
        period = (current - last) * 2;    // calculate the period
        last = current;
        frequency = 1000 / period;
        last = current;
				
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

int TIM4_IRQHandler(StrP, StrF){
	TIM4->SR=0;
	sprintf(StrP, "%d", period); //convert int to string
	sprintf(StrF, "%d", frequency); //convert int to string
	printf("Periode T: ");
	printf("%s", StrP);
	printf(" ms\t");
	printf("Frequenz f: ");
	printf("%s", StrF);
	printf(" Hz\r\n");
	
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



