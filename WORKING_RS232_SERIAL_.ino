#include <SoftwareSerial.h>
/*
  Serial Event example

  When new serial data arrives, this sketch adds it to a String.
  When a newline is received, the loop prints the string and clears it.

  A good test for this is to try it with a GPS receiver that sends out
  NMEA 0183 sentences.

  NOTE: The serialEvent() feature is not available on the Leonardo, Micro, or
  other ATmega32U4 based boards.

  created 9 May 2011
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/SerialEvent
*/

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete


EspSoftwareSerial::UART  mySerial (16,17,true);

void setup() {
  // initialize serial:
  Serial.begin(9600);
  mySerial.begin(9600,EspSoftwareSerial::SWSERIAL_8N1,16,17,false);
  Serial.println("START");  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  delay(200);
  mySerial.print("@");
  delay(200);
  mySerial.print("@");
  delay(200);

}
String TagData;
void loop() {
  // print the string when a newline arrives:
  /*if (stringComplete) {
    Serial.println(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
  
  while (mySerial.available()) {
    // get the new byte:
    
    char inChar = (char)mySerial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }*/
  
    while(mySerial.available() > 0) 
      {
          byte incomingData = mySerial.read();
          TagData = TagData + String((char)incomingData);
      }
      Serial.println(TagData);
      TagData="";
  


  
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  
}
