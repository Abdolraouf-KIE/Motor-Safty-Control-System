#include "mbed.h"

#define DEBUG

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
bool startflashing=      0; //states whether to start flashing or not

//Define ISRs for interrupts
void start_handler(){
	running =   1;
}

void stoped_handler(){//when stop button is pressed
	ready =     0;
	running =   0;
	stoped=     1;
	#ifdef DEBUG
	printf("DEBUG:      ready=%i,stoped=%i,running=%i,guard_fault=%i,temp_fault=%i,\n     :      guard_open=%i,temp_high=%i,MotorWasRunning=%i,startflashing=%i\n",ready,stoped,running,guard_fault,temp_fault,guard_open,temp_high,MotorWasRunning,startflashing);
    #endif
    
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
    led_temp_fault = 0; //turn off after 0.5second
}

void gaurd_fault_alert() {
    led_guard_fault = 0; //turn off after 0.5seconds
    
}

// MAIN FUNCTION **************************
int main() {
 while(1){
    #ifdef DEBUG
    printf("                    START%s\n", "");
    #endif
    //Interrupt handlers
	start.rise(&start_handler);
	
	stop.rise(&stoped_handler);
	stop.fall(&not_stoped_handler);
	
	guard.rise(&guard_open_handler);
	guard.fall(&guard_close_handler);
	
	temp.rise(&temp_high_handler);
	temp.fall(&temp_low_handler);

    startflashing=1;
    led_motor_ready=0;
    
    while (ready==0) {
        led_motor_running=0; //turn off the motor
        
        if(startflashing==1){
            flipper.attach(&flip, 2.0); // the address of the function to be attached (flip) and the interval (2 seconds)
            startflashing=0;
            #ifdef DEBUG
            printf("NOT READY:  start flashing%s\n","");
            #endif
        }

        if (temp_fault==1){
       	    led_temp_fault=1;
       	    temp_timeout.attach(&temp_fault_alert, 0.5); // timeout of 0.5s
       	    temp_fault=0;
            printf("FAULT:  TEMP%s\n","");
        }
        
        if (guard_fault==1){
       	    if(MotorWasRunning){
       		    led_guard_fault=1;
       		    gaurd_timeout.attach(&gaurd_fault_alert, 0.5); // timeout of 0.5s
       		    MotorWasRunning=0;
       		    #ifdef DEBUG
       		    printf("FAULT:  GUARD%s\n","");
       		    #endif
       	    }
       	guard_fault=0;
        }
        
        if(guard_open==0 && temp_high==0 && stoped==0){
        	ready=1;
        // 	printf("STATUS CHANGE: Ready Now%s\n", "");
        }
	wait_ms(200);
    }//end of not ready loop
    
    #ifdef DEBUG
      bool debug1=1;
      bool debug2=1;
	#endif
	
    while(ready==1){//ready loop
        
        #ifdef DEBUG
        if(debug1==1){
            printf("READY%s\n", "");
            debug1=0;
        }
        #endif
        
	    led_temp_fault=0;
        led_guard_fault=0;
	    led_motor_ready=1;
	    flipper.detach();
	
	    while(running==1 && ready==1){
	        #ifdef DEBUG
	        if(debug2==1){
	            printf("RUNNING%s\n", "");
	            debug2=0;
	        }
	        #endif
		    led_motor_running=1;
		    led_motor_ready=0;
		    MotorWasRunning=1;
	        wait_ms(200);
	    }
	    wait_ms(200);    
    }
    #ifdef DEBUG
    printf("                    END%s\n", "");
    #endif
}//full while loop
}
