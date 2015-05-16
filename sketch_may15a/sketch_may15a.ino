//Audio out with 38.5kHz sampling rate
//by Amanda Ghassaei
//http://www.instructables.com/id/Arduino-Vocal-Effects-Box/
//July 2012

/*
 * Phil Isaac 2015
 * Looper pedal using SD card
 */

#include <SPI.h>
#include <SD.h>

File myFile;
int recBtn = 9;
int prevPos;
int curPos;
char val;
int startMicros;

void setup(){
  //////////////////////////////////////////////////////////////////////////////
  // SD related setup
  Serial.begin(9600);
  while(!Serial) {
    ; // Just wait for the serial port to connect (Leonardo only)
  }
  Serial.print("Initializing SD card");
  
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // Delete the old file
  SD.remove("track1");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("track1", FILE_WRITE);

  //////////////////////////////////////////////////////////////////////////////
  // Adudio I/O related 
  DDRD=0xFF;//set port d (digital pin s0-7) as outputs
  DDRC=0x00;//set all analog pins as inputs
  
  //set up continuous sampling of analog pin 0
  
  //clear ADCSRA and ADCSRB registers
  ADCSRA = 0;
  ADCSRB = 0;
  
  ADMUX |= (1 << REFS0); //set reference voltage
  ADMUX |= (1 << ADLAR); //left align the ADC value- so we can read highest 8 bits from ADCH register only
  
  ADCSRA |= (1 << ADPS2) | (1 << ADPS0); //set ADC clock with 32 prescaler- 16mHz/32=500kHz
  ADCSRA |= (1 << ADATE); //enabble auto trigger
  ADCSRA |= (1 << ADEN); //enable ADC
  ADCSRA |= (1 << ADSC); //start ADC measurements

  //////////////////////////////////////////////////////////////////////////////
  // Set up other buttons
  pinMode(recBtn, INPUT);
  prevPos = digitalRead(recBtn);
  
}

void loop(){
  
  startMicros = micros();
  curPos = digitalRead(recBtn);

  // Check to see if a transition just occured
  if (curPos != prevPos) {
    Serial.write("Transition\n");
    // up pos: playback
    if (curPos) {
      Serial.write("Up\n");
      myFile.close();
      myFile = SD.open("track1");
    }
    else {
      // down pos: record
      Serial.write("Down\n");
      myFile.close();
      SD.remove("track1");
      myFile = SD.open("track1", FILE_WRITE);
    }
  }

  // Now either play or record
  if (curPos) {
    // up pos: play
    val = myFile.read();
    // end condition
    if (val == -1) {
      myFile.close();
      myFile = SD.open("track1");
      val = myFile.read();
    }
    PORTD = val;
  }
  else {
    // down pos: record
    val = ADCH;
    myFile.write(val);
  }

  prevPos = curPos;
  
  // 35.7 kHz. This will overflow after ~70 minutes
  while (micros() - startMicros < 28) {
    ;
  }

}


