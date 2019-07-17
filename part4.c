/*
 * test.c
 *
 * Created: 2019/7/6 18:06:56
 * Author : Coco
 */ 
#include <avr/io.h>
#include "scheduler.h"
#include "bit.h"
#include "io.c"
#include "io.h"
#include "keypad.h"
//#include "lcd_8bit_task.h"
//#include "queue.h"
//#include "seven_seg.h"
//#include "stack.h"
#include "timer.h"
//#include "usart.h"
static task task1;
task *tasks[] = {&task1};
enum pauseButtonSM_States {pauseButton_wait, pauseButton_press, pauseButton_release};

unsigned char i = 0;
unsigned char position = 0;

void writedata(unsigned char row, unsigned char col, unsigned char simbol)
{
		LCD_Cursor(col+1+(row-1)*15);
		LCD_WriteData(simbol);
}
unsigned char press = '\0';
//Monitors button connected to PA0.
//When button is pressed, shared variable "pause" is toggled.
int pauseButtonSMTick(int state){
	press = GetKeypadKey();
	switch(state){
		case pauseButton_wait:
			state = (press == '\0')? pauseButton_wait: pauseButton_press; break;
		case pauseButton_press:
			state = pauseButton_release; break;
		case pauseButton_release:
			state = (press == '\0')? pauseButton_wait: pauseButton_release; break;
		default:
			state = pauseButton_wait;break;
	}
	switch(state){
		case pauseButton_wait:
		break;
		case pauseButton_press:
		if(position <16){
			writedata(1, position, press);
			position++;
			}
		else{
			position = 0;
			writedata(1, position, press);
			}
			
		break;
		case pauseButton_release:
		break;
		default:
		break;
	}
	return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	/*unsigned char x;
	DDRB = 0xFF; PORTB = 0x00;*/
	DDRC = 0xFF; PORTC  = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	DDRA = 0xF0; PORTA = 0x0F;
//	DDRB = 0xFF; PORTB = 0x00;
	LCD_init();
	const unsigned char con[16] = "Congratulations!";
	LCD_DisplayString(1, con);
	//LCD_DisplayString(1,string);
    /* Insert your solution below */
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    //Task 1 (pauseButtonToggleSM)
    
    task1.state =  pauseButton_wait;//Task initial state
    task1.period = 5;//Task Period
    task1.elapsedTime = task1.period;//Task current elapsed time.
    task1.TickFct = &pauseButtonSMTick;//Function pointer for the tick
    //Set the timer and turn it on
   
	unsigned long int GCD = 5;
    TimerSet(5);
    TimerOn();
    
    unsigned short i;//Scheduler for-loop iterator
    while (1) {
		for(i = 0; i < numTasks; i++){//Scheduler code
			if(tasks[i]->elapsedTime == tasks[i]->period){//Task is ready to tick
				tasks[i]->state= tasks[i]->TickFct(tasks[i]->state);//set next state
				tasks[i]->elapsedTime = 0;//Reset the elapsed time for next tick;
			}else tasks[i]->elapsedTime += GCD; 
			//GCD = findGCD(tasks[i]->period, GCD);
		}
		while(!TimerFlag);
		TimerFlag = 0;
    }
    return 1;
}
