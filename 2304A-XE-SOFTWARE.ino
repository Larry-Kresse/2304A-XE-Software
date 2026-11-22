#include <FastLED.h>
#include "esp_timer.h" // for esp_timer_get_time()
#include <SoftwareSerial.h>

/*
Keithley 2304A remote display alternative PSU control software

Larry Kresse
Started in 2023, then abandoned

Salvaged in 2026

MIT Licensed
*/

/****************************************************
*      Application state                            *
****************************************************/
bool psu_enabled = false; //is PSU relay commanded on?
bool psu_comms = false;   //is PSU talking to us?
//if PSU enabled==false, the "power" light will be Red
//if PSU enabled==true, but psu_comms is false, 
// the "power" light will be Yellow

//if PSU enabled==true, and psu_comms is true, the
// power light will be green.

//if psu_enabled == false or psu_comms == false,
// the OLED should also be off to maintain lifespan
// (OLEDs can have a lifespan as low as 10000 hrs)

//relay to connect 4-wire (kelvin) sensing wires
//to jacks on front of UI board
//I actually forgot 
bool fourwire_sw_enabled = false; 


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

//not a key, but probably was intended to be controlled
//by one on external module
#define LOCK 13        //An external input intended to
                       //disable keys in case of cats
                       //walking on keypad. Not used.

/****************************************************
*      PINOUT: Output relays                        *
****************************************************/
#define RELAY_4WS 21 //4 wire sensing signal relay
#define RELAY_FAN 4  //optional PC fan to cool 2304A 
#define RELAY_POWER 15 //controls power to 2304A

#define FAN_POWER 128 //PC fan runs at half speed
                      //When PSU is on
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

}

/****************************************************
*        OLED  Initialization Routine               *
****************************************************/

void initOLED(){
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
int64_t last_successful_rx = 0;

int64_t RX_TIMEOUT_US = 500000;

void updateScreen() {

  // if we have received an update from the PSU recently, check if we have had any lately
  // to detect a timeout (and set psu_comms false to warn user)
  if (psu_comms == true)
  {
    int64_t now = esp_timer_get_time(); //get current time
    if (now - last_successful_rx > RX_TIMEOUT_US) {
      psu_comms = false;
    }
  }


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
      //yay, we got data! celebrate by setting psu_comms to true!
      psu_comms = true;
      last_successful_rx = esp_timer_get_time(); //store current time as last successful RX
                                                 //used to detect timeout

      Serial.println(inputData);
      inputData="";
    
      mySerial.flush();
      printToScreen();
    }
  }
}


/*****************************************************
*     Poll-keys function                             *
*****************************************************/
// handle debouncing and trigger actions in response
// to key-up, key-down, key-toggle

// KEY_OPERATE 39 
// KEY_4WSENSE 36 //Not a 2304A key. Controls 4 wire
                       //sense signal relay

// KEY_ENTER 34
// KEY_RIGHT 35
// KEY_DOWN 32
// KEY_UP 33
// KEY_LEFT 25
// KEY_SET 26
// KEY_DISPLAY 27
// KEY_POWER 14   //Not a 2304A key. Controls power
                       //to the 2304A via a relay.
          
// KEY_MENU 12

// for this much time after Key-up, the key will be 
// unpressable
#define KEY_DEBOUNCE_TIME_US 10000

// to get a result, the key must be held for this much time
#define KEY_HOLD_TIME_TO_TRIGGER_US 40000

//keys to be polled
int keys[11] = {KEY_OPERATE, KEY_4WSENSE, KEY_ENTER, KEY_RIGHT, KEY_DOWN, KEY_UP, KEY_LEFT, KEY_SET, KEY_DISPLAY, KEY_POWER, KEY_MENU};

//The 2304A serial command (a single byte) that should be sent when they key is pressed
//? and / are not 2304A commands, and signify something different should be done instead of communicating with the 2304A
char codes[11] = {'E',        '?',          'A',        'F',      'J',      'K',    '0',      'C',       'P',       '/',        'B'};

//KEY-UP is the @ for some reason
char keyup = '@';

//how long has key been held? (in us)
int64_t held[11] = {0};

//this should be updated by initKeyPolling() on startup ideally..
int64_t last_update_time_us[11] = {0};

//have we sent the code to the 2304A yet (or toggled the global var for power or 4wsense)?
bool triggeredAction[11] = {0};

//all this does is call the esp32's timer to set the initial update time
void initKeyPolling() {
  int64_t timeInit = esp_timer_get_time();
  for (size_t i = 0; i<sizeof(keys)/sizeof(keys[0]); i++) {
    last_update_time_us[i] = timeInit;
  }
}

// for 2304A keys, this handles debounce and sends serial commands to the 2304A
// for non-2304A keys, this changes the state of the system and doesn't send serial.

// expectation is that relay outputs and LEDs are to be updated by another function
void pollKeysAndTriggerActions()
{

  //check if each key is being pressed
  for (size_t i = 4; i<sizeof(keys)/sizeof(keys[0]); i++) {

    //There is a clock time for each key since execution time of 
    //one key's entry can increase if it has to send any serial
    //data

    //esp_timer_get_time() stores a 64-bit int, and will roll over in 
    //292471 years. I'll be dead by then, so I'm not going to account
    //for that edge case
    int64_t now = esp_timer_get_time(); //get current time

    //calculate THIS KEY's elapsed time
    int64_t elapsed_time = now - last_update_time_us[i]; 
    last_update_time_us[i] = now; //store THIS KEY's last update time

    //update held time for negative held
    //if held is negative, it means we are in the post-release debounce
    //This can increase until it hits 0. Held doesn't increase past 0 unless
    //button is actually held down
    if (held[i]<0) { //if we are in the debounce state, we can't even check if buttons is held down
      held[i] += elapsed_time;
      if(held[i]>0) {held[i]=0;} //held cannot exceed 0 unless button is actually held

    } else { //we are not in the debounce state!
      if (digitalRead(keys[i])==0) { //key is pressed
        if(i>3) {
          Serial.print("Key ");         Serial.print(i);
          Serial.println(" is pressed! ");
        }

        held[i] += elapsed_time; //update held if the key is actively being pressed

        if ((held[i]>KEY_HOLD_TIME_TO_TRIGGER_US) && (triggeredAction[i]==0)) { //key has been held long enough to trigger action
          if(i>3) {
            Serial.print("Key ");         Serial.print(i);
            Serial.println(" is triggered! ");  
          }
          if (codes[i] == '?' ) { //4WS pin
            fourwire_sw_enabled = !fourwire_sw_enabled; 
          } else if (codes[i] == '/' ) {//Power pin
            psu_enabled = !psu_enabled;
          } else {
            mySerial.write(codes[i]); //send the button's code to the 2304A
          }
          triggeredAction[i] = 1;     //record that we sent code, so we don't send multiple

        }
      } else { //key is not pressed
        if (held[i]>KEY_HOLD_TIME_TO_TRIGGER_US) { //key has been held long enough to trigger action
          if ((codes[i] != '?' ) && (codes[i] != '/')) { //the special codes don't have a keyup action
            mySerial.write(keyup); //key was pressed long enough to send code, so it must also send keyup char
          } 
          held[i] = (-1)*KEY_DEBOUNCE_TIME_US; //while held is negative, button can't be pressed
                                               //implements KEY_DEBOUNCE_TIME_US debounce
          triggeredAction[i] = 0; //reset action triggered tracker                                          
        }
      }
    }
  } //key polling loop



} // pollKeysAndTriggerActions()

/*****************************************************
*           "Relay" updater function                 *
*****************************************************/
// 4-wire-sensing switch is a signal relay to connect the
// 4-wire kelvin sensing leads to the 2304A. 

// psu_enabled controls the main power relay, that can
// cut power to the 2304A to save energy. 

// Fan is a solid-state switch and can be PWMd to adjust
// PC case fan speed. The PC case fan is intended to be
// Aimed at the 2304A to provide extra cooling.
// The case fan is only on if the PSU is powered on too.

void updateRelayStates() {
  digitalWrite(RELAY_4WS,fourwire_sw_enabled);
  digitalWrite(RELAY_POWER,psu_enabled);
  if (psu_enabled) {
    analogWrite(RELAY_FAN, FAN_POWER);
  } else {
    digitalWrite(RELAY_FAN, 0); //no PWM, fan off
  }
}


/*****************************************************
*     WS2812B RGB LED update function                *
*****************************************************/
// I enjoyed a function of a new Keithley PSU at the
// HIVE Makerspace at Georgia Tech. It had a 
// LED-backlit output button. When the PSU's output
// was on, the output button was blue. 

//default color of backlighting. 
CRGB default_color = 0x00AAAA;

//if action is triggered, use this color
CRGB hit_color = 0x00FFFF;

//if button is in debounce 
CRGB debounce_color = 0x000000;

size_t num_leds = 11;
                // 0               1         2          3           4        5        6        7          8           9          10
//int keys[11] = {KEY_OPERATE, KEY_4WSENSE, KEY_ENTER, KEY_RIGHT, KEY_DOWN, KEY_UP, KEY_LEFT, KEY_SET, KEY_DISPLAY, KEY_POWER, KEY_MENU};

//access the LED's index value of this array to find the relevant key index
//So you can lookup if a key is in debounce by the key's LED
size_t led_index_2_key_index[] = {9,10,1,7,8,6,5,4,3,0,2};

// Takes the LED's index, and uses the state of the corresponding key
// to color LED, with different colors for debounced, triggered, and
// default backlight state
void setLedByIndexAndKeyState(size_t led_index) {
  //this index is used for held, triggeredAction, 
  size_t key_index = led_index_2_key_index[led_index];

  if(held[key_index]<0) {
    leds[led_index] = debounce_color; //key is in debounce
  } else {
    if (triggeredAction[key_index]) {
      leds[led_index] = hit_color; 
    } else {
      leds[led_index] = default_color;
    }
  }

}

// This is to be used if the PSU is not interactable
// It is not interactable if it is powered off or if
// it has not connected yet
// This allows LEDs to communicate to user that the PSU
// isn't usable yet.
void clearAllLEDsExceptPower() {
    for (size_t i = 1; i<num_leds; i++)
    {
      leds[i]=CRGB::Black;  
    }
}

// Sets the color of the key backlight LEDs based on the
// system state
void updateLEDs() {
  //Power LED 0
  if(psu_enabled) {
    if(psu_comms) { //green if communicating
      leds[0]=CRGB::Green;
    } else { //yellow if on but not communicating yet
      leds[0]=CRGB::Yellow;
      clearAllLEDsExceptPower(); //we can't control 2304A until it connects
                                 //this communicates that to user
      FastLED.show();
      return;                                  
    }
  } else { //red if powered down via power relay, all LEDs but power should be off!
    leds[0]=CRGB::Red;
    clearAllLEDsExceptPower(); //we can't control 2304A until it is powered on!

    FastLED.show();
    return;
  }

  //Menu LED 1
  setLedByIndexAndKeyState(1);

  //4WS LED 2
  if(fourwire_sw_enabled) {
    leds[2] = CRGB::Green;
  } else {leds[2] = default_color; }

  //Set LED 3
  setLedByIndexAndKeyState(3);

  //Display LED 4
  setLedByIndexAndKeyState(4);

  //Left LED 5
  setLedByIndexAndKeyState(5);

  //Up LED 6
  setLedByIndexAndKeyState(6);

  //Down LED 7
  setLedByIndexAndKeyState(7);

  //Right LED 8
  setLedByIndexAndKeyState(8);

  //Output LED
  if((line1[13]=='O')&&(line1[14]=='F')&&(line1[15]=='F')) {
    leds[9]=CRGB::Black;

  } else if ((line1[14]=='O')&&(line1[15]=='N')) {
    leds[9]=CRGB::Blue; //Keithley Output Blue
  }

  //Enter LED 10
  setLedByIndexAndKeyState(10);

  //actually outputs data to LEDs
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

  //output relays
  pinMode(RELAY_4WS,OUTPUT); //4 wire sensing signal relay
  pinMode(RELAY_FAN,OUTPUT); //optional PC fan to cool 2304A 
  pinMode(RELAY_POWER,OUTPUT);//controls power to 2304A

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

  //RS-232 Serial to Keithley 2304A
  mySerial.begin(9600,EspSoftwareSerial::SWSERIAL_8N1,RS232_RX,RS232_TX,false);
  Serial.println("START");  // reserve 200 bytes for the inputString:
  inputString.reserve(400);

  //Add LEDs
  FastLED.addLeds<NEOPIXEL, LEDPIN>(leds, 11);
  
  //OLED initialization
  initOLED();

  initKeyPolling(); //updates the key timers for the first time

  delay(100);

  pinMode(KEY_POWER, INPUT_PULLUP); //Not a 2304A key.

}

/*****************************************************
*           Loop Function, to run repeatedly         *
*****************************************************/

void loop() {
  //delay(100);
  //mySerial.write('H');


  pollKeysAndTriggerActions();
  updateRelayStates();
  updateLEDs();
  
  mySerial.write('@');
  delay(50);
  updateScreen();


}
