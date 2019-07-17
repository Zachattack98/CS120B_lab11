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
unsigned char led0_output = 0x00;
unsigned char led1_output = 0x00;
unsigned char pause = 0;

unsigned char string[37]="CS120B is Legend... wait for it DARY!";
unsigned char string_length=37;
unsigned char pointer = 0;
unsigned char i = 0;
unsigned char display_string[16] = "";
//Monitors button connected to PA0.
//When button is pressed, shared variable "pause" is toggled.
enum tickStates{start,Lessthan16, Greaterthan16, Greaterthan36};
int tick(int state)
{
	
	switch(state)
	{
		case start: state = Lessthan16;break;
		case Lessthan16: state = (pointer>15)? Greaterthan16: Lessthan16; break;
		case Greaterthan16: state = (pointer >= 36)? Greaterthan36: Greaterthan16;break;
		case Greaterthan36: state = (pointer >= 52)? start: Greaterthan36;break;
		default: state = start;break;
	}
	switch(state)
	{
		case start:
			i = 0;
			pointer = 0;
			break;
		case Lessthan16:
			for(i = 0; i < pointer+1; i++)
			{
				display_string[15-pointer+i] = string[i];
			}
			for(i = 0; i < 15- pointer;i++)
			{
				display_string[i] = ' ';
			}
			pointer++;
			break;
		case Greaterthan16:
			for(i = 0; i < 16; i++)
			{
				display_string[i] = string[pointer-15+i];
			}
			pointer++;
			break;
		case Greaterthan36:
			for(i = 0; i < 52-pointer+1; i++)
			{
				display_string[i] = string[pointer-16+i];
			}
			for(; i < 16; i++)
			{
				display_string[i] = ' ';
			}
			pointer++;
			break;
		default:
			break;
	}
	return state;
}

enum displayStates{start1,display};
int displaySM(int state)
{
	
	switch(state)
	{
		case start1:
			state = display;
			break;
		case display:
			break;
		default:
			state = start;
			break;
	}
	switch(state)
	{
		case start1:
			break;
		case display:
			
			LCD_DisplayString(1, display_string);
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
//	DDRA = 0x00; PORTA = 0xFF;
//	DDRB = 0xFF; PORTB = 0x00;
	LCD_init();
	//LCD_DisplayString(1,string);
    /* Insert your solution below */
    static task task1, task2;
    task *tasks[] = {&task1, &task2};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    //Task 1 (pauseButtonToggleSM)
    
    task1.state = start;//Task initial state
    task1.period = 1;//Task Period
    task1.elapsedTime = task1.period;//Task current elapsed time.
    task1.TickFct = &tick;//Function pointer for the tick
    //Set the timer and turn it on
    
    task2.state = start1;//Task initial state
    task2.period = 1;//Task Period
    task2.elapsedTime = task2.period;//Task current elapsed time.
    task2.TickFct = &displaySM;//Function pointer for the tick
	unsigned long int GCD = 1;
    TimerSet(1);
    TimerOn();
    
    unsigned short i;//Scheduler for-loop iterator
    while (1) {
		for(i = 0; i < numTasks; i++){//Scheduler code
			if(tasks[i]->elapsedTime == tasks[i]->period){//Task is ready to tick
				tasks[i]->state= tasks[i]->TickFct(tasks[i]->state);//set next state
				tasks[i]->elapsedTime = 0;//Reset the elapsed time for next tick;
			}
			tasks[i]->elapsedTime += GCD; 
			GCD = findGCD(GCD, tasks[i]->period);
		}
		while(!TimerFlag);
		TimerFlag = 0;
    }
    return 1;
}