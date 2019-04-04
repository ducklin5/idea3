#define ARDUINO_ARCH_AVR
#include <Arduino.h>
#include "Servo.h"
#include "Servo.cpp"
#define BUFFER_SIZE 150

char buffer[BUFFER_SIZE];
int bufferCount = 0;


float pydist(float x1, float y1, float x2, float y2 ){
	return sqrt(sq(x2-x1)  + sq(y2-y1));
}
void lerpArray(float start, float end, int k, float* ary){
	for(int i=0; i<k; i++ )
		ary[i] = start + (end-start) * (i+1)/k;
}
// returns angle of dy/dx as a value from 0...2PI
float atan3(float dy,float dx) {
	float a = atan2(dy,dx);
	if(a<0) a = (PI*2.0)+a;
	return a;
}

class Plotter{
	public:
		Servo xServo; Servo yServo; Servo zServo;
		int xpin; int ypin; int zpin;
		float width; float height; float depth;
		float xpos; float ypos; float zpos;
		int minPW; int maxPW;
		float speed;
		bool absMode; bool absRotMode;
		int StepIncPower = 3;
		float StepInc = (1/ pow(2,StepIncPower));

		Plotter(int xp, int yp, int zp, float w, float h, float d, int mnpw, int mxpw):
			xpin(xp), ypin(yp), zpin(zp), width(w), height(h), depth(d), minPW(mnpw), maxPW(mxpw) {}

		void begin(float spd = 20){
			setSpeed(spd);
			xServo.attach(xpin,minPW,maxPW);
			yServo.attach(ypin,minPW,maxPW);
			zServo.attach(zpin,minPW,maxPW);
			xpos= 0; ypos= 0; zpos = 0;
			absMode = true; absRotMode = false;
			home();
			delay(3000);
		}
		void setSpeed(float spd){
			speed = (float)spd / (float) 1000; // mm/s to mm/ms 
			speed = 0.2;
		}
		void home(){
			rMove('x',0);
			rMove('y',0);
			rMove('z',0);
		}

		void end(){
			xServo.detach();
			yServo.detach();
			zServo.detach();
		}

		void printPos(){
			Serial.print("Position: <");
			Serial.print(xpos);
			Serial.print(", ");
			Serial.print(ypos);
			Serial.print(", ");
			Serial.print(zpos);
			Serial.println(">");
		}

		void rMove(char axis, float posEnd){
			float rotEnd;
			switch(axis){
				case 'x': case 'X':
					posEnd = constrain(posEnd, 0, width);
					rotEnd = map(posEnd, 0, width, 0, 180);
					xServo.write(rotEnd);
					xpos = posEnd;
					break;

				case 'y': case 'Y':
					posEnd = constrain(posEnd, 0, height);
					rotEnd = map(posEnd, 0, height, 0, 180);
					yServo.write(rotEnd);
					ypos= posEnd;
					break;

				case 'z': case 'Z':
					//posEnd = constrain(posEnd, 0, depth);
					rotEnd = map(posEnd, 0, depth, 0, 180);
					zServo.write(rotEnd);
					zpos= posEnd;
					break;

				default: break;
			}
		}
		// using bresenham's line algorithm
		// http://rosettacode.org/wiki/Bitmap/Bresenham's_line_algorithm
		// https://github.com/MakerBlock/TinyCNC-Sketches/blob/master/TinyCNC_Gcode/TinyCNC_Gcode.ino
		void lMove(float xEnd, float yEnd){
			int StepDelay = 1;

			float xrot = map(xpos, 0, width, 0, 180);
			float yrot = map(ypos, 0, height, 0, 180);
			float xrotEnd = map(xEnd, 0, width, 0, 180);
			float yrotEnd = map(yEnd, 0, height, 0, 180);
			
			float dx = abs(xrotEnd-xrot), sx = xrot<xrotEnd ? StepInc : -StepInc;
			float dy = abs(yrotEnd-yrot), sy = yrot<yrotEnd ? StepInc : -StepInc;
			float err = (dx>dy ? dx : -dy)/2, e2;
			while(true){
				if (xrot==xrotEnd && yrot==yrotEnd) break;
				e2 = err;
				if (e2 >-dx) {
					err -= dy;
					xrot += sx;
				}
				if (e2 < dy) {
					err += dx;
					yrot += sy;
				}
				xServo.write(xrot);
				yServo.write(yrot);
				delay(StepDelay);           //delay for settling
			}
			//  Update the positions
			xpos = map(xrot, 0, 180, 0, width);
			ypos = map(yrot, 0, 180, 0, height);
		}

		void arcMove(float cx,float cy,float xEnd,float yEnd, bool CCW) {
			int mmPerSeg = 2;

			float radius  = pydist(xpos, ypos, cx, cy);

			float angleS = atan3(ypos-cy,xpos-cx);
			float angleE = atan3(yEnd-cy,xEnd-cx);

			float sweep = angleE-angleS;
			if(CCW && sweep<0) angleE+=2*PI;
			else if(!CCW && sweep>0) angleS+=2*PI;

			sweep = angleE-angleS;

			float arcLen = abs(sweep) * radius;
			int segNum = arcLen / mmPerSeg;
			float segAngle[segNum];
			lerpArray(angleS, angleE, segNum, segAngle);

			float segX; float segY;

			for (int i=0; i<segNum; i++){
				segX = cx + cos(segAngle[i]) * radius;
				segY = cy + sin(segAngle[i]) * radius;
				// make a line to that intermediate position
				lMove(segX, segY);
			}
			lMove(xEnd, yEnd);
		}
};

Plotter pltr(10,9,8,100,100,50,540,2410);

/*
https://www.marginallyclever.com/2013/08/how-to-build-an-2-axis-arduino-cnc-gcode-interpreter/
*/
void info(){
	Serial.println(F("Commands:"));
	Serial.println(F("G00 [X(steps)] [Y(steps)] [F(feedrate)]; Rapid linear move"));
	Serial.println(F("G01 [X(steps)] [Y(steps)] [F(feedrate)]; - linear move"));
	Serial.println(F("G02 [X(steps)] [Y(steps)] [I(center X relative)] [J(center Y relative)]; - CW Rot"));
	Serial.println(F("G03 [X(steps)] [Y(steps)] [I(center X relative)] [J(center Y relative)]; - CCW Rot"));
	Serial.println(F("G04 P[seconds]; - delay"));
	Serial.println(F("G90; - absolute mode"));
	Serial.println(F("G91; - relative mode"));
	Serial.println(F("G92 [X(steps)] [Y(steps)]; - change logical position"));
	Serial.println(F("M3; - start motors"));
	Serial.println(F("M5; - disable motors"));
	Serial.println(F("M100; - this help message"));
	Serial.println(F("M114; - report position and feedrate"));
}

void ready() {
	bufferCount=0; // clear input buffer
	Serial.print("R > "); // signal ready to receive input
}

void setup() {
	init();
	Serial.begin(9600);
	Serial.println("Serial is Ready");
	pltr.begin();
	Serial.println("Pltr is Ready");
	info();
	ready();
}

/**
 * Look for character /code/ in the buffer and read the float that immediately follows it.
 * @return the value found.  If nothing is found, /val/ is returned.
 * @input code the character to look for.
 * @input val the return value if /code/ is not found.
 **/
float parseNumber(char code,float val) {
	for(int i = 0; i<BUFFER_SIZE; i++){
		bool isComment = (buffer[i] == '(' );
		while(isComment){
			isComment = (buffer[i] != ')');
			i++;
		}

		if(buffer[i]==code) {  // if you find code on your walk,
			return atof(buffer+i+1);  // convert the digits that follow into a float and return it
		}
	}
	return val;  // end reached, nothing found, return default val.
}

/**
 * Read the input buffer and find any recognized commands.  One G or M command per line.
 */
void parseBuffer() {
	float x,y,z,i,j;
	int cmd = parseNumber('G',-1);
	if (cmd != -1 ){
		x = parseNumber('X', (pltr.absMode? pltr.xpos: 0)) + (pltr.absMode? 0: pltr.xpos);
		y = parseNumber('Y', (pltr.absMode? pltr.ypos: 0)) + (pltr.absMode? 0: pltr.ypos);
		z = parseNumber('Z', (pltr.absMode? pltr.zpos: 0)) + (pltr.absMode? 0: pltr.zpos);
		i = parseNumber('I', (pltr.absRotMode? pltr.xpos: 0)) + (pltr.absRotMode? 0: pltr.xpos);
		j = parseNumber('J', (pltr.absRotMode? pltr.ypos: 0)) + (pltr.absRotMode? 0: pltr.ypos);
	}
	switch(cmd) {

		case  0: {
					 pltr.rMove('z', z);
					 pltr.rMove('x', x);
					 pltr.rMove('y', y);
					 break;
				 }
		case  1: { // line
					 pltr.setSpeed(parseNumber('F',pltr.speed));
					 pltr.rMove('z', z);
					 pltr.lMove(x, y);
					 break;
				 }
		case 2:
		case 3: {
					pltr.setSpeed(parseNumber('F',pltr.speed));
					pltr.rMove('z', z);
					pltr.arcMove(i,j,x,y,cmd==3);
					break;
				}
		case  4:  delay(parseNumber('P',0));  break;  // dwell
		case 90:  pltr.absMode=true;  break;  // absolute mode
		case 91:  pltr.absMode=false;  break;  // relative mode
		default:  break;
	}

	cmd = parseNumber('M',-1);
	switch(cmd) {
		case 3:  // program start enable motors
			pltr.begin(); break;
		case 5:  // program ended disable motors
			pltr.end(); break;
		case 100:  info();  break;
		case 114:  pltr.printPos();  break;
		default:  break;
	}
}

int main() {
	setup();
	while(true){
		if( Serial.available() ) { // if something is available
			char c = Serial.read(); // get it
			if (c=='\b'){
				if(bufferCount > 0){
					Serial.print("\b \b");
					buffer[bufferCount]=0;
					bufferCount--;
				}
				continue;
			}

			Serial.print(c); // optional: repeat back what I got for debugging
			// store the byte as long as there's room in the buffer.
			// if the buffer is full some data might get lost
			if(bufferCount < BUFFER_SIZE) buffer[bufferCount]=c;
			bufferCount ++;
			// if we got a return character (\n)  the message is done.
			if(c=='\n' or c=='\r') {
				Serial.print("\r\n"); // optional: send back a return for debugging

				// strings must end with a \0.
				buffer[bufferCount]=0;
				parseBuffer();
				delay(100);
				ready();
				bufferCount = 0;
			} 
		}
	}
	Serial.flush();
	Serial.end();
}
