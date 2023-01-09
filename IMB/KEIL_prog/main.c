/*Projekt: SoapCar
//Part: IMB - Intelligent Motor Bridge
//Head: Elias Dillinger
//Time Intervall: to long xD*/
#include "stm32f4xx.h"
#include <stdio.h>
#include <math.h>
int USART2_write(int c);
int USART2_read(void);
int32_t countP1 = 0; //THIS NEEDS to be int32_t, otherwise it will overflow
int32_t countP2 = 0;
//before the Interrupt kicks in-> took me serveral hours to find out xD
void delayMs(int n);
void USART2_init(void);
int period = 0;
int frequency = 0;
int df = 100;
int de = 200;
int fa = 20000;
int d = 0;
int deof = 0;
int main(void) {
	  int last = 0;
    int current;
    int n; //Counter Variable for UART2
    char str[80];//String for URART2
	  int num = 123;
	  char StrP[5];
	  char StrF[5];

    __disable_irq();                    /* global disable IRQs */
    RCC->AHB1ENR |= 0x7;                  /* enable GPIO A, B C clock */
    RCC->APB2ENR |= 0x4000;             /* enable SYSCFG clock */
		RCC->APB1ENR |= 0x2001F; //TIM 2,3,4,5,6 UART2 enabled
		
		
		//configure global GPIOA MODER Reg: 
		GPIOA-> MODER |= 0x5A4A4;
		//configure global GPIOB MODER Reg
		GPIOB-> MODER |= 0x0002;
		
		USART2_init();
    printf("Test I/O functions by Printing: ELEKTROMINATI\r\n");
	  printf("        X        Elektrominiati:\r\n");
	  printf("       X X       make.\r\n");
	  printf("      X _ X      something.\r\n");
	  printf("     X |_| X     cool.\r\n");
	  printf("    X_______X    today.\r\n");
	
    // setup TIM4 for CLI output 
		//RCC->APB1ENR |= 0x4; ^^ to global APB1ENR
    TIM4->PSC = 2000 - 1;           // divided by prescaler
    TIM4->ARR = 100001 - 1;          // divided by reload register
    TIM4->CNT = 0;                  /* clear timer counter */
    TIM4->CR1 = 1;                  /* enable TIM4 */
		TIM4->DIER |= 1;                /* enable UIE */
    NVIC_EnableIRQ(TIM4_IRQn);      /* enable interrupt in NVIC */
		
    /* configure PA4 for rising interrupt */
    //GPIOA->MODER &= ~0x300;        /* clear pin mode to input mode */
    //SYSCFG->EXTICR[1] &= ~0x00F0;       /* clear port selection for EXTI4 */
    SYSCFG->EXTICR[1] |= 0000;        /* select port A for EXTI4 */
    EXTI->IMR |= 0x10;                /* unmask EXTI4 0x10 == 0d00010000*/
		//EXTI->FTSR |= 0x10;               /* select falling edge trigger */
    EXTI->RTSR |= 0x10;               /* select rising edge trigger */
		//NVIC->ISER[1] = 0x00000100;         /* enable IRQ40 (bit 8 of ISER[1]) */
    NVIC_EnableIRQ(EXTI4_IRQn);
		
		// configure PB2 for rising interrupt
		SYSCFG->EXTICR[0] |= 0x100; //slect port B for EXTI2
		EXTI->IMR |= 0x04; //0x4 == 0d0100
		EXTI->RTSR |= 0x04;
		NVIC_EnableIRQ(EXTI2_IRQn);

		
    __enable_irq();                     /* global enable IRQs */
		
		
		// configure PA6 as input of TIM3 CH1
    //RCC->AHB1ENR |=  1;             /* enable GPIOA clock^^ to global AHB1ENR*/
    //GPIOA->MODER &= ~0x00003000;    /* clear pin mode */
    //GPIOA->MODER |=  0x00002000;    /* set pin to alternate function ^^ global MODER*/
    GPIOA->AFR[0] &= ~0x0F000000;   /* clear pin AF bits */
    GPIOA->AFR[0] |= 0x02000000;    /* set pin to AF2 for TIM3 CH1 */
    // configure TIM3 to do input capture with prescaler ...
    //RCC->APB1ENR |= 0x2;              /* enable TIM3 clock ^^ global APB1ENR*/
    TIM3->PSC = 16 - 1;          /* divided by 16000 f=1MHz...1µs request rate*/
    TIM3->CCMR1 = 0x41;             /* set CH1 to capture at every edge */
    TIM3->CCER = 0x0B;              /* enable CH 1 capture both edges */
    TIM3->CR1 = 1;                  /* enable TIM3 */
    
    while(1) {
			while (!(TIM3->SR & 2)) {}  // wait until input edge is captured
        current = TIM3->CCR1;       // read captured counter value
        period = (current - last) * 2;    // calculate the period
        last = current;
        frequency = 1000000 / period;
        last = current;
				
    }
}
void EXTI4_IRQHandler(void) {
    
    // setup TIM2
    //Clock by default at 16 MHz/
    //RCC->APB1ENR |= 1;              //enable TIM2 clock ^^ global APB1ENR
    TIM2->PSC = 160 - 1;          // divided by 160-> f = 100 kHz
    TIM2->ARR = 5 - 1;           // divided by 5-> f2 = 10 kHz max! Res = 50 µs-> 1/2 Period time
    TIM2->CR1 = 0x0011;      // enable counter in HEX please (debug reason)
    TIM2->DIER |= 1;                //enable UIE 
    NVIC_EnableIRQ(TIM2_IRQn);      //enable interrupt in NVIC
    EXTI->PR = 0x10;          //clear interrupt pending flag
}

void EXTI2_IRQHandler (void){
		// setup TIM5
		//RCC->APB1ENR |= 1; //enable TIM5 CLK
		TIM5->PSC = 160-1;//divide by 160-> f = 100kHz
		TIM5->ARR = 5-1; //divide by 5
		TIM5->CR1 = 0x0011; //enable Counter + Direction UP
		TIM5-> DIER |= 1;
		NVIC_EnableIRQ(TIM5_IRQn);
		EXTI->PR = 0x04;
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
	//detection f = 20 kHz
	countP1 += 1;//raise the value every time the Handler starts-> every 1ms
	if(countP1 == (d/50) - 1){//d*50-1 because d[1µs] EXTI f=20kHz 
		//GPIOA->BSRR = 0x00000020;   /* turn on LED PA5*/
		GPIOA->BSRR |= 0x2; //turn on PA1 outTriacP1
	}
	if(countP1 == deof){//one count step is 50µs => f=20kHz
		//GPIOA->BSRR = 0x00200000;   /* turn off LED PA5*/
		GPIOA->BSRR |= 0x20000; //turn off PA1 outTriacP1
		//IMPORTANT
		countP1 = 0; //resets counter^^ so we're ready for the next EXTI
		TIM2->CR1 = 0x0000; //DISABLE the Timer, so we can Synch & Enable the Timer with the next EXTI
		PID();
	}
	
}

void TIM5_IRQHandler(void){
	TIM5->SR = 0;
	countP2 += 1;
	if(countP2 == 80-1){
		GPIOA->BSRR |= 0x100; //turn on PA8 outTriacP2
	}
	if(countP2 == 100-1){
		GPIOA->BSRR |= 0x1000000;//turn off PA8
		countP2 = 0; 
		TIM5-> CR1 = 0x0000;
	}
}
void TIM4_IRQHandler(){
	TIM4->SR=0;
	printOutCLI(period,frequency);

	
}
int PID(){
	d = (period/510) * df -de;
	deof = (d+de)/50 -1;
}
int printOutCLI(period, frequency){
	printf("Periode T: ");
	printf("%d", (int)period); //convert int to string
	//printf("%s", period);
	printf(" micros\t");
	printf("Frequenz f: ");
	printf("%d", (int)((d+de)/50) - 1); //convert int to string
	//printf("%s", frequency);
	printf(" Hz\r\n");
}
/* initialize USART2 to transmit at 9600 Baud */
void USART2_init (void) {
    //RCC->AHB1ENR |= 1;          /* Enable GPIOA clock ^^global AHB1ENR*/
    //RCC->APB1ENR |= 0x20000;    /* Enable USART2 clock ^^global APB1ENR*/
    /* Configure PA2, PA3 for USART2 TX, RX */
    GPIOA->AFR[0] &= ~0xFF00;
    GPIOA->AFR[0] |=  0x7700;   /* alt7 for USART2 */
    //GPIOA->MODER  &= ~0x00F0;
    //GPIOA->MODER  |=  0x00A0;   /* enable alt. function for PA2, PA3 */
		//See global GPIOA-> MODER config  ^^
    USART2->BRR = 0x0683;       /* 9600 baud @ 16 MHz */
    USART2->CR1 = 0x000C;       /* enable Tx, Rx, 8-bit data */
    USART2->CR2 = 0x0000;       /* 1 stop bit */
    USART2->CR3 = 0x0000;       /* no flow control */
    USART2->CR1 |= 0x2000;      /* enable USART2 */
}
/* Write a charact			er to USART2 */
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
