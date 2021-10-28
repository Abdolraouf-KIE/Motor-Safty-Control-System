#include "mbed.h"

//inputs
#define START_BUTTON_1 p5
#define STOP_BUTTON_2 p6 
#define GUARD_SWITCH_1 p7 
#define TEMP_SWITCH_2 p8 

//outputs
#define LED1_MOTOR_READY   LED1
#define LED2_MOTOR_RUNNIG  LED2
#define LED3_TMEP_FAULT    LED3
#define LED4_GUARD_FAULT   LED4


//declaring input and output pins 
DigitalOut led_motor_ready(LED1_MOTOR_READY);
DigitalOut led_motor_running(LED2_MOTOR_RUNNIG);
DigitalOut led_temp_fault(LED3_TMEP_FAULT);
DigitalOut led_guard_fault(LED4_GUARD_FAULT);

//tickers and timeouts
Ticker flipper; //flashing
Timeout temp_timeout;
Timeout gaurd_timeout;

//Define Interrupt Inputs
InterruptIn start(START_BUTTON_1);
InterruptIn stop(STOP_BUTTON_2);
InterruptIn guard(GUARD_SWITCH_1);
InterruptIn temp(TEMP_SWITCH_2);

//global variables. Their value determines initial state when motor is powered.
bool ready=              0; //not ready
bool stoped=             0; //0 means stop button not being held
bool running=            0; //motot not running
bool guard_fault=        0; //guard fault hasnt occured. This is used to trigger fualt LED3. 
bool temp_fault=         0; //temp fault hasnt occured. This is used to trigger fualt LED4. 
bool guard_open=         0; //guard is closed
bool temp_high=          0; //temperature is not excess
bool MotorWasRunning=    0; //gives the status of the motor just moments before.

//Define ISRs for interrupts
void start_handler(){
	running =   1;
}

void stoped_handler(){//when stop button is pressed
	ready =     0;
	running =   0;
	stoped=     1;
}

void not_stoped_handler(){ //when stop button is realeased
	stoped=     0;
}

void temp_high_handler(){ //when temp is higher than preset value
	ready =     0;
	running =   0;
    	temp_fault= 1;
    	temp_high=  1;
}

void temp_low_handler(){ //when temp is lower than preset value
	temp_high=0;
}

void guard_open_handler(){ //when guard is opened
	ready =     0;
	running =   0;
    	guard_fault= 1;
    	guard_open=  1;
}

void guard_close_handler(){ //when guard is closed
    	guard_open=  0;
}

void flip() {
    led_motor_ready = !led_motor_ready;
}

void temp_fault_alert() {
    led_temp_fault = 0; //turn off after 0.5seconds
}

void gaurd_fault_alert() {
    led_guard_fault = 0; //turn off after 0.5seconds
}

// MAIN FUNCTION
int main() {
    
    //Interrupt handlers
	start.rise(&start_handler);
	
	stop.rise(&stoped_handler);
	stop.fall(&not_stoped_handler);
	
	guard.rise(&guard_open_handler);
	guard.fall(&guard_close_handler);
	
	temp.rise(&temp_high_handler);
	temp.fall(&temp_low_handler);
	
    while (ready==0) {
        led_motor_running=0; //turn of the motor
        
	flipper.attach(&flip, 1.0); // the address of the function to be attached (flip) and the interval (2 seconds)
        
        if (temp_fault==1){
       	led_temp_fault=1;
       	temp_timeout.attach(&temp_fault_alert, 0.5); // timeout of 0.5s
        }
        
        if (guard_fault==1){
       	if(MotorWasRunning){
       		led_guard_fault=1;
       		gaurd_timeout.attach(&gaurd_fault_alert, 0.5); // timeout of 0.5s
       		MotorWasRunning=0;
       	}
       	guard_fault=0;
        }
        
        if(!guard_open && !temp_high && !stoped){
        	ready=1;
        }
	wait_ms(500);
    }//end of not ready loop
    
    while(ready==1){
	led_temp_fault=0;
	led_guard_fault=0;
	led_motor_ready=1;
	
	while(running==1 && ready==1){
		led_motor_running=1;
		MotorWasRunning=1;
	    wait_ms(500);
	    }
	wait_ms(500);    
    }
}
