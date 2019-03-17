/*
 * Pong.c
 */ 

#include <avr/interrupt.h>
#include <avr/io.h>
#include "io.c"
#include "io.h"

//TIMER
// TimerISR() sets this to 1. C programmer should clear to 0.
volatile unsigned char TimerFlag = 0;
// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
void TimerOn() {
	// AVR timer/counter controller register TCCR1
	// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s
	TCCR1B = 0x0B;
	// AVR output compare register OCR1A.
	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	OCR1A = 125;
	// AVR timer interrupt mask register
	// bit1: OCIE1A -- enables compare match interrupt
	TIMSK1 = 0x02;
	//Initialize avr counter
	TCNT1=0;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds
	_avr_timer_cntcurr = _avr_timer_M;
	//Enable global interrupts: 0x80: 1000000
	SREG |= 0x80;
}
void TimerOff() {
	// bit3bit1bit0=000: timer off
	TCCR1B = 0x00;
}
void TimerISR() {
	TimerFlag = 1;
}
// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1
	// (every 1 ms per TimerOn settings)
	// Count down to 0 rather than up to TOP (results in a more efficient comparison)
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		// Call the ISR that the user uses
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void adc_init(){
	ADMUX = (1<<REFS0);
	
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

unsigned short readadc(uint8_t ch)
{
	ch&=0b00000111;         //ANDing to limit input to 7
	ADMUX = (ADMUX & 0xf8)|ch;  //Clear last 3 bits of ADMUX, OR with ch
	ADCSRA|=(1<<ADSC);        //START CONVERSION
	while((ADCSRA)&(1<<ADSC));    //WAIT UNTIL CONVERSION IS COMPLETE
	return(ADC);        //RETURN ADC VALUE
}

void transmit_data_C(unsigned char data) {
	int i;
	for (i = 0; i < 8 ; ++i) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTC = 0x08;
		// set SER = next bit of data to be sent.
		PORTC |= ((data >> i) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTC |= 0x02;
	}
	// set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
	PORTC |= 0x04;
	// clears all lines in preparation of a new transmission
	PORTC = 0x00;
}

void transmit_data_D(unsigned char data) {
	int i;
	for (i = 0; i < 8 ; ++i) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTD = 0x08;
		// set SER = next bit of data to be sent.
		PORTD |= ((data >> i) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTD |= 0x02;
	}
	// set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
	PORTD |= 0x04;
	// clears all lines in preparation of a new transmission
	PORTD = 0x00;
}

int joystickPos1() {
	int position;
	unsigned short valX;
	
	valX = readadc(0);
	
	if(valX >= 800) {
		position = 1; // right
	}
	else if(valX <= 80) {
		position = 2; // left
	}
	else {
		position = 0;
	}
	return position;
}

int joystickPos2() {
	int position;
	unsigned short valX;
	
	valX = readadc(1);
	
	if(valX >= 800) {
		position = 1; // right
	}
	else if(valX <= 80) {
		position = 2; // left
	}
	else {
		position = 0;
	}
	return position;
}

//Global Variables ----------------------
unsigned char global_column_sel1 = 0xC7; 
unsigned char global_column_sel2 = 0xC7; 
	
unsigned char column_val1 = 0x01;
unsigned char column_sel1 = 0xC7;
	
unsigned char column_val2 = 0x80;
unsigned char column_sel2 = 0xC7;

unsigned char column_val3 = 0x02; 
unsigned char column_sel3 = 0xEF;

unsigned char i = 2;
unsigned char j = 2;
unsigned char z = 1;

unsigned char softReset = 0;
unsigned char data_reset_1 = 0;
unsigned char data_reset_2 = 0;

unsigned char player1_scored;
unsigned char player2_scored;
unsigned char p1Score;
unsigned char p2Score;

unsigned char cnt = 0;
unsigned char cnt1 = 0;

unsigned char winner_cnt = 0;

unsigned char paddleMovement[5] = {0x1F, 0x8F, 0xC7, 0xE3, 0xF8};
//Global Variables ------------------------------------------------

// TickFct Enums-------------------------------------------------------
enum SM_States {SM_Start, SM_wait, SM_RIGHT, SM_LEFT} SM_State1;
static unsigned char pos1 = 0;

enum SM_States2 {SM_Start2, SM_wait2, SM_RIGHT2, SM_LEFT2} SM_State2;
static unsigned char pos2 = 0;

enum SM_States3 {SM_Start3, SM_Display, SM_UP3, SM_DOWN3} SM_State3;

enum SM_States4 {SM_Start4, SM_Display4, SM_PLAYER1, SM_PLAYER2, SM_P1_WIN, SM_P2_WIN} SM_State4;

enum SM_States5 {SM_Start5, SM_Wait, SM_Reset} SM_State5;
// TickFct Enums-------------------------------------------------------

TickFct_PaddleShift1() {

	switch(SM_State1) { // Transitions
		case SM_Start:
		if(cnt1 == 26) {
			SM_State1 = SM_wait;
		}
		else {
			SM_State1 = SM_Start;
		}
		break;
		case SM_wait:
		if(data_reset_1 == 1) {
			global_column_sel1 = 0xC7;
			column_sel1 = 0xC7;
			i = 2;
			data_reset_1 = 0;
			SM_State1 = SM_wait;
		}
		else if (pos1 == 1) {
			SM_State1 = SM_RIGHT;
		}
		else if (pos1 == 2) {
			SM_State1 = SM_LEFT;
		}
		break;
		case SM_RIGHT:
		if (1) {
			SM_State1 = SM_wait;
		}
		break;
		case SM_LEFT:
		if (1) {
			SM_State1 = SM_wait;
		}
		break;
		default:
		SM_State1 = SM_wait;
	} // Transitions

	switch(SM_State1) { // State actions
		case SM_wait:
		break;
		case SM_RIGHT:
		if(column_sel1 != 0xF8) {
			i++;
			column_sel1 = paddleMovement[i]; // shift right
			global_column_sel1 = paddleMovement[i];
		}
		break;
		case SM_LEFT:
		if(column_sel1 != 0x1F) {
			i--;
            column_sel1 = paddleMovement[i]; // shift left
			global_column_sel1 = paddleMovement[i];
		}
		break;
		default:
		break;
	} // State actions
	if(i ==0) {
		transmit_data_C(paddleMovement[0]);
	}
	else if(i == 1) {
		transmit_data_C(paddleMovement[1]);
	}
	else if(i == 2) {
		transmit_data_C(paddleMovement[2]);
	}
	else if(i == 3) {
		transmit_data_C(paddleMovement[3]);
	}
	else if(i == 4) {
		transmit_data_C(paddleMovement[4]);
	}
	else if(i == 5) {
		transmit_data_C(paddleMovement[5]);
	}
	transmit_data_D(column_val1);
}

TickFct_PaddleShift2() {
	
	switch(SM_State2) { // Transitions
		case SM_Start2:
		if(cnt1 == 26) {
			SM_State2 = SM_wait2;
		}
		else {
			SM_State2 = SM_Start2;
		}
		break;
		case SM_wait2:
		if(data_reset_2 == 1) {
			global_column_sel2 = 0xC7;
			column_sel2 = 0xC7;
			j = 2;
			data_reset_2 = 0;
			SM_State2 = SM_wait2;
		}
		else if (pos2 == 1) {
			SM_State2 = SM_RIGHT2;
		}
		else if (pos2 == 2) {
			SM_State2 = SM_LEFT2;
		}
		break;
		case SM_RIGHT2:
		if (1) {
			SM_State2 = SM_wait2;
		}
		break;
		case SM_LEFT2:
		if (1) {
			SM_State2 = SM_wait2;
		}
		break;
		default:
		SM_State2 = SM_wait2;
	} // Transitions

	switch(SM_State2) { // State actions
		case SM_wait2:
		break;
		case SM_RIGHT2:
		if(column_sel2 != 0xF8) {
			j++;
			column_sel2 = paddleMovement[j]; // shift right
			global_column_sel2 = paddleMovement[j];
		}
		break;
		case SM_LEFT2:
		if(column_sel2 != 0x1F) {
			j--;
			column_sel2 = paddleMovement[j]; // shift left
			global_column_sel2 = paddleMovement[j];
		}
		break;
		default:
		break;
	} // State actions
	if(j ==0) {
		transmit_data_C(paddleMovement[0]);
	}
	else if(j == 1) {
		transmit_data_C(paddleMovement[1]);		
	}
	else if(j == 2) {
		transmit_data_C(paddleMovement[2]);		
	}
	else if(j == 3) {
		transmit_data_C(paddleMovement[3]);		
	}
	else if(j == 4) {
		transmit_data_C(paddleMovement[4]);		
	}
	else if(j == 5) {
		transmit_data_C(paddleMovement[5]);		
	}
	transmit_data_D(column_val2);
}

TickFct_Ball() {
	
	switch(SM_State3) { // Transitions
		case SM_Start3:
		if(cnt1 == 26) {
		SM_State3 = SM_Display;
		}
		else {
			SM_State3 = SM_Start3;
		}
		break;
		
		case SM_Display:
		if(z % 2 == 0) {
			SM_State3 = SM_UP3;
		}
		else if(z % 2 != 0) {
			SM_State3 = SM_DOWN3;
		}
		break;
		
		case SM_UP3:
		if(1) {
			SM_State3 = SM_Display;
		}
		break;
		case SM_DOWN3:
		if(1) {
			SM_State3 = SM_Display;
		}
		break;
		default:
		SM_State3 = SM_Display;
	} // Transitions

	switch(SM_State3) { // State actions
		case SM_Display:
		break;
		case SM_UP3:
		if (column_val3 != 0x02) {
			column_val3 = column_val3 >> 1; // shift up illuminated LED one row
		}
		else if(column_val3 == 0x02 && (global_column_sel1 == 0x8F || global_column_sel1 == 0xC7 || global_column_sel1 == 0xE3)) {
			z++;
		}
		else if(column_val3 == 0x02 && (global_column_sel1 != 0x8F && global_column_sel1 != 0xC7 && global_column_sel1 != 0xE3)) {
			data_reset_1 = 1;
			data_reset_2 = 1;
			
			column_val3 = 0x02;
			column_sel3 = 0xEF;
			
			player2_scored = 1;
			p2Score++;
		}
		break;
		case SM_DOWN3:
		if (column_val3 != 0x40) {
			column_val3 = column_val3 << 1; // shift down illuminated LED one row
		}
		else if(column_val3 == 0x40 && (global_column_sel2 == 0x8F || global_column_sel2 == 0xC7 || global_column_sel2 == 0xE3)) {
			z++;
		}
		else if(column_val3 == 0x40 && (global_column_sel2 != 0x8F && global_column_sel2 != 0xC7 && global_column_sel2 != 0xE3)) {
			data_reset_1 = 1;
			data_reset_2 = 1;
			
			column_val3 = 0x02;
			column_sel3 = 0xEF;
			
			player1_scored = 1;
			p1Score++;
		}
		break;
		default:
		break;
	} // State actions
	transmit_data_C(column_sel3);
	transmit_data_D(column_val3);
}

TickFct_LCDScreen() {

	switch(SM_State4) { // Transitions
		case SM_Start4:
		if(cnt1 <= 25) {
			cnt1++;
			SM_State4 = SM_Start4;
		}
		else if(cnt1 == 26) {
			LCD_ClearScreen();
			cnt1++;
			SM_State4 = SM_Display4;
		}
		break;

		case SM_Display4:
		if(p1Score == 7) {
			SM_State4 = SM_P1_WIN;
			p1Score = 0;
		}
		else if(p2Score == 7) {
			SM_State4 = SM_P2_WIN;
			p2Score = 0;
		}
		else if(player1_scored == 1) {
			SM_State4 = SM_PLAYER1;
			player1_scored = 0;
		}
		else if(player2_scored == 1) {
			SM_State4 = SM_PLAYER2;
			player2_scored = 0;
		}
		break;
		case SM_PLAYER1:
		if (1) {
			SM_State4 = SM_Display4;
		}
		break;
		case SM_PLAYER2:
		if (1) {
			SM_State4 = SM_Display4;
		}
		break;
		case SM_P1_WIN:
		if(winner_cnt <= 2) {
			SM_State4 = SM_P1_WIN;
			winner_cnt++;
		}
		else {
			SM_State4 = SM_Display4;
			winner_cnt = 0;
		}
		break;
		case SM_P2_WIN:
		if(winner_cnt <= 2) {
			SM_State4 = SM_P2_WIN;
			winner_cnt++;
		}
		else {
			SM_State4 = SM_Display4;
			winner_cnt = 0;
		}
		break;
		default:
		SM_State4 = SM_Display4;
	} // Transitions

	switch(SM_State4) { // State actions
		case SM_Display4:
		if(cnt == 0) {
		LCD_DisplayString(1, "Score:          P1)0       P2)0");
		cnt++;
		}
		break;
		case SM_PLAYER1:
		LCD_Cursor(20);
		LCD_WriteData(p1Score + '0');
		break;
		case SM_PLAYER2:
		LCD_Cursor(31);
		LCD_WriteData(p2Score + '0');
		break;
		case SM_P1_WIN:
		LCD_ClearScreen();
		LCD_DisplayString(1, "Player 1: Winner Winner Winner");
		cnt = 0;
		break;
		case SM_P2_WIN:
		LCD_ClearScreen();
		LCD_DisplayString(1, "Player 2: Winner Winner Winner");
		cnt = 0;
		break;
		default:
		break;
	} // State actions
}

TickFct_ResetCheck() {
	const unsigned char tmpA6 = ~PINA & 0x40;

	switch(SM_State5) { // Transitions
		case SM_Start5:
		SM_State5 = SM_Wait;
		break;

		case SM_Wait:
		if(tmpA6) {
			SM_State5 = SM_Reset;
		}
		else {
			SM_State5 = SM_Wait;
		}
		break;

		case SM_Reset:
		if(!tmpA6) {
			//Initial States:
			SM_State1 = SM_Start;
			SM_State2 = SM_Start2;
			SM_State3 = SM_Start3;
			SM_State4 = SM_Start4;
			
			//Reset Global Values
			global_column_sel1 = 0xC7;
			global_column_sel2 = 0xC7;
			
			column_val1 = 0x01;
			column_sel1 = 0xC7;
			
			column_val2 = 0x80;
			column_sel2 = 0xC7;
			
			i = j = 2;
			z = 1;

			column_val3 = 0x02;
			column_sel3 = 0xEF;

			softReset = 0;
			data_reset_1 = 0;
			data_reset_2 = 0;

			player1_scored;
		    player2_scored;
			p1Score;
			p2Score;

			cnt = 0;
			cnt1 = 0;

			winner_cnt = 0;
			
			SM_State5 = SM_Wait;
		}
		else {
			SM_State5 = SM_Reset;
		}
		break;
		default:
		SM_State5 = SM_Wait;
	} // Transitions

	switch(SM_State5) { // State actions
		case SM_Wait:
		break;
		case SM_Reset:
		break;
		default:
		break;
	} // State actions
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // Configure port A's pins as inputs and outputs,
	DDRB = 0xFF; PORTB = 0x00; // Configure port B's 8 pins as outputs,
	DDRC = 0xFF; PORTC = 0x00; // Configure port C's 8 pins as outputs,
	DDRD = 0xFF; PORTD = 0x00; // Configure port D's 8 pins as outputs,
	
	unsigned long paddle1_elapsedTime = 10;
	unsigned long paddle2_elapsedTime = 10;
	unsigned long ball_elapsedTime = 20;
	unsigned long lcdScreen_elapsedTime = 10;
	unsigned long resetCheck_elapsedTime = 10;
	const unsigned long timerPeriod = 10;
	
	// Initializes the LCD display
	LCD_init();
	
	// Starting at position 1 on the LCD screen, writes Hello World
	LCD_DisplayString(1, "Start Pong!");

	// Analog-Digital_Conversion
	adc_init();

	//Initialize Timer		
	TimerSet(timerPeriod);
	TimerOn();
    
	//Initial States
    SM_State1 = SM_Start;
	SM_State2 = SM_Start2;
	SM_State3 = SM_Start3;
	SM_State4 = SM_Start4;
	
    while (1) 
    {
		pos1 = joystickPos1();
		pos2 = joystickPos2();
		
        if (paddle1_elapsedTime >= 10) {
	        TickFct_PaddleShift1(); 
	        paddle1_elapsedTime = 0;
        }
        if (paddle2_elapsedTime >= 10) { 
	        TickFct_PaddleShift2(); 
	        paddle2_elapsedTime = 0;
        }
		if (ball_elapsedTime >= 20) { 
			TickFct_Ball(); 
			ball_elapsedTime = 0;
		}
		if(lcdScreen_elapsedTime >= 10) {
			TickFct_LCDScreen();
			lcdScreen_elapsedTime = 0;
		}
		if(resetCheck_elapsedTime >= 10) {
			TickFct_ResetCheck();
			resetCheck_elapsedTime = 0;
		}
		
        while (!TimerFlag){}   // Wait for timer period
        TimerFlag = 0;         // Lower flag raised by timer
		
        paddle1_elapsedTime += timerPeriod;
        paddle2_elapsedTime += timerPeriod;
		ball_elapsedTime += timerPeriod;
		lcdScreen_elapsedTime += timerPeriod;
		resetCheck_elapsedTime += timerPeriod;
    }
}