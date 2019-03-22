/* Sweep
   by BARRAGAN <http://barraganstudio.com>
   This example code is in the public domain.
   modified 8 Nov 2013
   by Scott Fitzgerald
http://www.arduino.cc/en/Tutorial/Sweep
*/
#define ARDUINO_ARCH_AVR
#include <Arduino.h>
#include "Servo.h"
#include "Servo.cpp"

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position

void setup() {
	init();
	myservo.attach(12);  // attaches the servo on pin 9 to the servo object
	Serial.begin(9600);
	Serial.println("Serial is Ready");
}

int main() {
	setup();
	while(true){
		float start = 0;
		float stop = 200;
		Serial.print(start);
		Serial.print(" to ");
		Serial.println(stop);
		for (pos = start; pos <= stop; pos += 1) { // goes from 0 degrees to 180 degrees
			// in steps of 1 degree
			myservo.write(pos);              // tell servo to go to position in variable 'pos'
			delay(15);                       // waits 15ms for the servo to reach the position
		}
		Serial.print(stop);
		Serial.print(" to ");
		Serial.println(start);
		for (pos = stop; pos >= start; pos -= 1) { // goes from 180 degrees to 0 degrees
			myservo.write(pos);              // tell servo to go to position in variable 'pos'
			delay(30);                       // waits 15ms for the servo to reach the position
		}
	}
	Serial.flush();
	Serial.end();
}
