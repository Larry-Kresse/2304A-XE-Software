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
*      PINOUT: Arduino Uno -> Character OLED        *
****************************************************/

#define CS    5
#define SDI   23
#define SCL   18

//Serial input stuff

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
EspSoftwareSerial::UART  mySerial (16,17,true);
String inputData;

//LEDs
CRGB leds[11];

#define LEDPIN 22

/****************************************************
*                 Text Strings  for display                    *
****************************************************/

char line1[16] = {};// = ("  Newhaven Displ");
char line2[16] = {};// = ("   International");

char last1[16] = {}; //store last data to only refresh screen when needed
char last2[16] = {};
/****************************************************
*               Function Commands                   *
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
//  Start_Data_Transmission Function: 
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
//  Data Function: 
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
//  End_Data_Transmission Function: 
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Set CS pin HIGH to end transmission.
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void end_data_transmission()
{
  digitalWrite(CS, HIGH);  // CS = 1;
  delay(1);
}

/****************************************************
*                  Display Functions                *
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
*              Initialization Routine               *
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

/*****************************************************
*           Setup Function, to run once              *
*****************************************************/

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


void setup() {
  pinMode(SCL,OUTPUT);
  pinMode(SDI,OUTPUT);
  pinMode(CS,OUTPUT);

  pinMode(39, INPUT_PULLUP);

  Serial.begin(9600);
  mySerial.begin(9600,EspSoftwareSerial::SWSERIAL_8N1,16,17,false);
  Serial.println("START");  // reserve 200 bytes for the inputString:
  inputString.reserve(200);


  FastLED.addLeds<NEOPIXEL, LEDPIN>(leds, 11);
  
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
