#include <FastLED.h>

#include <SoftwareSerial.h>

/*
/Program for writing to NHD-0420DZW series display. 
/This code is written for the Arduino Uno (Microchip ATmega328P) using Serial Interface. 
/
/Newhaven Display invests time and resources providing this open source code,
/Please support Newhaven Display by purchasing products from Newhaven Display! 

* Copyright (c) 2020 Alee Shah - Newhaven Display International, Inc. 
*
* This code is provided as an example only and without any warranty by Newhaven Display. 
* Newhaven Display accepts no responsibility for any issues resulting from its use.
* The developer on the final application incorporating any parts of this 
*  sample code is responsible for ensuring its safe and correct operation
*   and for any consequences resulting from its use. 
* See the GNU General Public License for more details.
* 
* 
*/

/****************************************************
*      PINOUT: Input keys                           *
****************************************************/

//NOTE: These inputs need external pull-up resistors
//Unfortunately, Espressif messed up the ESP32's design
//And they don't have software-controllable internal ones
#define KEY_OPERATE 39 
#define KEY_4WSENSE 36 //Not a 2304A key. Controls 4 wire
                       //sense signal relay

#define KEY_ENTER 34
#define KEY_RIGHT 35
#define KEY_DOWN 32
#define KEY_UP 33
#define KEY_LEFT 25
#define KEY_SET 26
#define KEY_DISPLAY 27
#define KEY_POWER 14   //Not a 2304A key. Controls power
                       //to the 2304A via a relay.
          
#define KEY_MENU 12

#define LOCK 13        //An external input intended to
                       //disable keys in case of cats
                       //walking on keypad. Not used.




/****************************************************
*      PINOUT: Character OLED                       *
****************************************************/
//MPN: Newhaven Display NHD-0216KZW-AB5 16x2 OLED
//Connected over SPI

#define CS    5
#define SDI   23
#define SCL   18

/****************************************************
*              Serial connection to 2304A           *
****************************************************/

// Physical layer is RS-232
#define RS232_RX 16
#define RS232_TX 17

// buffers, serial objects, and serial stream control
String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
EspSoftwareSerial::UART  mySerial (RS232_RX,RS232_TX,true);
String inputData;

/****************************************************
*          WS2812B-compatible gamer LEDs!           *
****************************************************/
//uses FASTLED
CRGB leds[11];

#define LEDPIN 22

/****************************************************
*                 Test Strings  for display         *
****************************************************/

char line1[16] = {};// = ("  Newhaven Displ");
char line2[16] = {};// = ("   International");

char last1[16] = {}; //store last data to only refresh screen when needed
char last2[16] = {};

/****************************************************
*         OLED  Function Commands                   *
****************************************************/

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Command Function:  
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  10-bit Instruction Transmission
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void command(char i)
{  
  unsigned int m;
  digitalWrite(CS, LOW);   // CS = 0;
  digitalWrite(SDI, LOW);  // RS = 0;
  digitalWrite(SCL, LOW);
  digitalWrite(SCL, HIGH); // Clock RS Pin in
  digitalWrite(SCL, LOW);
  delay(1);
  digitalWrite(SDI, LOW);  // RW = 0;
  digitalWrite(SCL, LOW);
  digitalWrite(SCL, HIGH); // Clock RW Pin in
  digitalWrite(SCL, LOW);
  for(m=0;m<8;m++){        // Clock in DB0 - DB7
    if((i&0x80)==0x80)
    digitalWrite(SDI, HIGH);
    else
    digitalWrite(SDI, LOW);
    while(0);
    i=(i<<1);
    digitalWrite(SCL, LOW);
    while(0);
    digitalWrite(SCL, HIGH);
    while(0);
    digitalWrite(SCL, LOW);
    }
  digitalWrite(CS, HIGH);  // CS = 1;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  OLED Start_Data_Transmission Function: 
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Set CS pin LOW to begin transmission; Set RS & RW bits 
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void start_data_transmission()
{  
  digitalWrite(CS, LOW);   // CS = 0;
  delay(1);
  digitalWrite(SDI, HIGH); // RS = 1;
  digitalWrite(SCL, LOW);
  digitalWrite(SCL, HIGH); // Clock RS Pin in
  digitalWrite(SCL, LOW);
  delay(1);
  digitalWrite(SDI, LOW);  // RW = 0;
  digitalWrite(SCL, LOW);
  digitalWrite(SCL, HIGH); // Clock RW Pin in
  digitalWrite(SCL, LOW);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  OLED Data Function: 
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Send 8-bit Transmission; Can be continuous transmission
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void data(char i)
{    
  unsigned int m;
  for(m=0;m<8;m++){        // Clock in DB0 - DB7
    if((i&0x80)==0x80)
    digitalWrite(SDI, HIGH);
    else
    digitalWrite(SDI, LOW);
    while(0);
    i=(i<<1);
    digitalWrite(SCL, LOW);
    while(0);
    digitalWrite(SCL, HIGH);
    while(0);
    digitalWrite(SCL, LOW);
    }
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  OLED End_Data_Transmission Function: 
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Set CS pin HIGH to end transmission.
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void end_data_transmission()
{
  digitalWrite(CS, HIGH);  // CS = 1;
  delay(1);
}

/****************************************************
*     OLED             Display Functions            *
****************************************************/

void clear_screen(){         // clear display 
 command(0x01);
}

void ret_home(){             // Return to home position
 command(0x02); 
}

//
//void disp1(){                 // DISPLAY TEXT
//  clear_screen();
//  ret_home();                 // First Line
//  start_data_transmission();
//  for( int i=0;i<16;i++){
//    data(text1[i]);
//  }
//  end_data_transmission();
///******************************************/    
//  command(0xc0);              // Second Line
//  start_data_transmission();
//  for (int i=0;i<16;i++){
//    data(text2[i]);
//  }
//  end_data_transmission();
///************************************* 
//  command(0x94);              // Third Line
//  start_data_transmission();
//  for( int i=0;i<20;i++){
//    data(text3[i]);
//  }
//  end_data_transmission();  
///************************************  
//  command(0xD4);              // Fourth Line
//  start_data_transmission();
//  for (int i=0;i<20;i++){
//    data(text4[i]);  
//  }*****
//  end_data_transmission();******/   
//}
//
//
//void disp2(){                 // DISPLAY TEXT
//  clear_screen();
//  ret_home();                 // First Line
//  start_data_transmission();
//  for( int i=0;i<16;i++){
//    data(text5[i]);
//  }
//  end_data_transmission();
///******************************************/    
//  command(0xc0);              // Second Line
//  start_data_transmission();
//  for (int i=0;i<16;i++){
//    data(text6[i]);
//  }
//  end_data_transmission();
///*****************************************    
//  command(0x94);              // Third Line
//  start_data_transmission();
//  for( int i=0;i<20;i++){
//    data(text7[i]);
//  }
//  end_data_transmission();   
///****************************************    
//  command(0xD4);              // Fourth Line
//  start_data_transmission();
//  for (int i=0;i<20;i++){
//    data(text8[i]);  
//  }
//  end_data_transmission();  **/  
//}

/****************************************************
*     OLED             Screen redraw logic          *
****************************************************/

int differenceThreshold=1; //less than this amount of differing characters means we do not redraw the screen
void printToScreen(){                 // DISPLAY TEXT

  int differences=0;
  for(int i=0;i<16;i++){
    if(line1[i]!=last1[i]){
      differences++;
    }
    if(line2[i]!=last2[i]){
      differences++;
    }
  }

  if(differences<1) {return;}
  
  clear_screen();
  ret_home();                 // First Line
  start_data_transmission();
  for( int i=0;i<16;i++){
    data(line1[i]);
    last1[i]=line1[i];//store last data
  }
  end_data_transmission();
/******************************************/    
  command(0xc0);              // Second Line
  start_data_transmission();
  for (int i=0;i<16;i++){
    data(line2[i]);
    last2[i]=line2[i];//store last data
  }
  end_data_transmission();
/*****************************************    
  command(0x94);              // Third Line
  start_data_transmission();
  for( int i=0;i<20;i++){
    data(text7[i]);
  }
  end_data_transmission();   
/****************************************    
  command(0xD4);              // Fourth Line
  start_data_transmission();
  for (int i=0;i<20;i++){
    data(text8[i]);  
  }
  end_data_transmission();  **/  
}

/****************************************************
*        OLED  Initialization Routine               *
****************************************************/

void init1(){
  delay(300);

  command(0x28);             //Function set
  delay(2);
  command(0x08);             //Display OFF
  delay(2);
  command(0x01);             //Clear Display
  delay(2);
  command(0x06);             //Entry Mode set
  delay(2);
  command(0x02);             //Return Home
  delay(2);
  command(0x0C);             //Display ON
  delay(2);
}



byte startFrame[4]={5,0,6,5};


void updateScreen() {
  if(mySerial.available()>0) {
    int byteCounter=0;
    int lineCounter=0;

    bool validFrame=false;
    int frameIdx=0;

    byte lastData=0;
    while(mySerial.available() > 0) {
        byte incomingData = mySerial.read(); //read incoming data

        if(incomingData==5) {  frameIdx=0; ;} //ENQ NUL ACK NUL is the start frame, lock onto ENQ
       // if(incomingData==6) {  ;}
       // if(incomingData==0) { ;}

        if ((validFrame==false) && (incomingData==startFrame[frameIdx])) {frameIdx++;  }//start frame part has been matched  

        if (frameIdx>=3) { validFrame=true;} //matched all chars of frame start, therefore this frame is valid

        if(incomingData<32) {continue;} //do not render test characters or space

        if(validFrame==true) {
          if(lineCounter==0){ line1[byteCounter]=(char)incomingData;}
          else { line2[byteCounter]=(char)incomingData;}
          
          byteCounter++;
  
          if(byteCounter>=16) { lineCounter++; byteCounter=0;}
  
          if(lineCounter>1) { break; }
  
  
          
          inputData = inputData + String((char)incomingData);
        }
    }
    if(validFrame==true)
    {
      Serial.println(inputData);
      inputData="";
    
      mySerial.flush();
      printToScreen();
    }
  }
}

/*****************************************************
*     WS2812B  LED update function                   *
*****************************************************/
// I enjoyed a function of a new Keithley PSU at the
// HIVE Makerspace at Georgia Tech. It had a 
// LED-backlit output button. When the PSU's output
// was on, the output button was blue. 

void updateLEDs() {
  leds[1]=CRGB::Red;
  //output blue
  if((line1[13]=='O')&&(line1[14]=='F')&&(line1[15]=='F')) {
    leds[9]=CRGB::Black;
    FastLED.show();
  } else if ((line1[14]=='O')&&(line1[15]=='N')) {
    leds[9]=CRGB::Blue;
    FastLED.show();
  }
  FastLED.show();

  
}

/*****************************************************
*           Setup Function, to run once              *
*****************************************************/

void setup() {

  //OLED SPI pins
  pinMode(SCL,OUTPUT);
  pinMode(SDI,OUTPUT);
  pinMode(CS,OUTPUT);

  //WS2812B LEDs
  pinMode(LEDPIN,OUTPUT);

  //Keys
  pinMode(KEY_OPERATE, INPUT); //NO INPUT PULLUP
  pinMode(KEY_4WSENSE, INPUT); //NO INPUT PULLUP Not a 2304A key.

  pinMode(KEY_ENTER, INPUT_PULLUP);
  pinMode(KEY_RIGHT, INPUT_PULLUP);
  pinMode(KEY_DOWN, INPUT_PULLUP);
  pinMode(KEY_UP, INPUT_PULLUP);
  pinMode(KEY_LEFT, INPUT_PULLUP);
  pinMode(KEY_SET, INPUT_PULLUP);
  pinMode(KEY_DISPLAY, INPUT_PULLUP);
  pinMode(KEY_POWER, INPUT_PULLUP); //Not a 2304A key.
  pinMode(KEY_MENU, INPUT_PULLUP);

  //Debug serial
  Serial.begin(9600);

  //RS-232 to Keithley 2304A
  mySerial.begin(9600,EspSoftwareSerial::SWSERIAL_8N1,RS232_RX,RS232_TX,false);
  Serial.println("START");  // reserve 200 bytes for the inputString:
  inputString.reserve(200);


  FastLED.addLeds<NEOPIXEL, LEDPIN>(leds, 11);
  
  //OLED initialization
  //DDRB = 0xFF; // Enable pins on PORT B as outputs
  init1();
  
  //delay(100);

  
}

/*****************************************************
*           Loop Function, to run repeatedly         *
*****************************************************/

bool operateState=0;

void loop() {

  /*if(digitalRead(39)==1) {
    if(operateState==0) {
      mySerial.write('E');
      operateState=1;
    }
  } else {
    if(operateState==1) {
      mySerial.write('@');
      operateState=0;
    }
  }*/
  
  //disp1();
  //delay(2500);
  //disp2();
  delay(100);
  updateScreen();
  updateLEDs();
}
