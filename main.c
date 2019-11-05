#include <avr/io.h>

void PWM_Init(void)
{
    //Set Pin6/PB1 to output
    DDRB = 1 << DDB1;
   
    //approximately a 700us pulse
    OCR1A = 0x4F;
    OCR1C = 0x9F;

    TCCR1 = 1 << CTC1 | //clear on match with OCR1C
	    1 << PWM1A | // set output PB1
	    1 << COM1A0 | // set output PB1
            6 << CS10;  //set prescaling to clk/16
}

int main(int argc, char** argv) {
    PWM_Init();

    while(1);

    return 0;
}

//#include <TinyWire.h>
//
//#define led_pin 1
//
//byte own_address = 10;
//
//
//void setup() {
//	// config led_pin as Output for driving an LED
//	pinMode(led_pin, OUTPUT);
//	
//	// config TinyWire library for I2C slave functionality
//	TinyWire.begin( own_address );
//	// sets callback for the event of a slave receive
//	TinyWire.onReceive( onI2CReceive );
//}
//
//void loop() {
//	
//}
//
///*
//I2C Slave Receive Callback:
//Note that this function is called from an interrupt routine and shouldn't take long to execute
//*/
//void onI2CReceive(int howMany){
//	// loops, until all received bytes are read
//	while(TinyWire.available()>0){
//		// toggles the led everytime, when an 'a' is received
//		if(TinyWire.read()=='a') digitalWrite(led_pin, !digitalRead(led_pin));
//	}
//}
//#include <TinyWire.h>
//
//byte own_address = 10;
//
//
//void setup() {
//	// config TinyWire library for I2C slave functionality
//	TinyWire.begin( own_address );
//	// register a handler function in case of a request from a master
//	TinyWire.onRequest( onI2CRequest );
//}
//
//void loop() {
//
//}
//
//// Request Event handler function
////  --> Keep in mind, that this is executed in an interrupt service routine. It shouldn't take long to execute
//void onI2CRequest() {
//	// sends one byte with content 'b' to the master, regardless how many bytes he expects
//	// if the buffer is empty, but the master is still requesting, the slave aborts the communication
//	// (so it is not blocking)
//	TinyWire.send('b');
//}
