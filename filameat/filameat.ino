/*
 * FilaMeat controller.  Written by Paul Chase.
 * 
 * Heavy reliance on the example code from the Freetronics LCD
 * as well as the examples from the arduino-menusystem.
 * 
 *   Pins used:
 *   A0: Buttons, analog input from voltage ladder
 *   A1: Thermistor read
 *   D3: Relay control pin
 *   D4: LCD bit 4
 *   D5: LCD bit 5
 *   D6: LCD bit 6    
 *   D7: LCD bit 7    
 *   D8: LCD RS    
 *   D9: LCD E    
 *   D10: LCD Backlight (high = on, also has pullup high so default is on)
 *   
 *   Initially, only implementing a simple temperature/setpoint display and the ability to change the setpoint.
 */

 #include <LiquidCrystal.h>
 #include <MenuSystem.h>
 #include "thermistor.h"

//Pin defines
#define BUTTON_ADC_PIN           A0  // A0 is the button ADC input
#define RELAY_PIN                3   // D3 controls the relay
#define LCD_BACKLIGHT_PIN        10  // D10 controls LCD backlight

//ADC voltages for the buttons - I had to read these, the default values were way off.
#define RIGHT_10BIT_ADC         815  // right
#define UP_10BIT_ADC            930  // up
#define DOWN_10BIT_ADC          903  // down
#define LEFT_10BIT_ADC          856  // left
#define SELECT_10BIT_ADC        612  // select
#define BUTTONHYSTERESIS         10  // hysteresis for valid button sensing window

//enum values for ReadButtons()
#define BUTTON_NONE               0
#define BUTTON_RIGHT              1
#define BUTTON_UP                 2
#define BUTTON_DOWN               3
#define BUTTON_LEFT               4
#define BUTTON_SELECT             5

//uncomment this line to make the bottom row of the LCD display the thermistor
//and button voltages within readButtons.
#define DEBUG

////////global variables
//buttons
uint8_t buttonJustPressed  = false;         //this will be true after a ReadButtons() call if triggered
uint8_t buttonJustReleased = false;         //this will be true after a ReadButtons() call if triggered
uint8_t buttonWas          = BUTTON_NONE;   //used by ReadButtons() for detection of button events

//lcd
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

//menu
MenuSystem ms;
Menu mm("Press Select");
MenuItem mm_temp("Update Temp");
MenuItem mm_duty("Update Duty Cycle");

//state variables
uint8_t  SetPoint;    //what temp we're trying to stay at
uint8_t  MaxDutyCycle;   //0-100%, how often to turn the heater on at most
uint8_t  CycleTime;   //length of a cycle, in milliseconds
uint16_t CycleStart;  //this is counted in milliseconds, 
bool     RelayOn;     //is the relay on?

void setup(){
  //fire up the LCD first
  lcd.begin(16,2);
  lcd.setCursor(0,1);
  lcd.print("FilaMEAT =^..^= ");
  lcd.setCursor(0,1);
  lcd.print("Paul Chase  v0.0");

  //initialize state
  SetPoint = 0;           //start with heaters off
  MaxDutyCycle = 50;      //seems like a good cycle for starters
  RelayOn = false;        //start off
  CycleTime = 10*1000;    //10 second cycle to start with
  CycleStart = millis();  //this is when the current cycle started
  
  //thermistor input
  pinMode(THERMISTOR_PIN, INPUT);         //ensure thermistor is an input
  digitalWrite(THERMISTOR_PIN, LOW);      //ensure pullup is off on thermistor

  //relay output
  pinMode(RELAY_PIN, OUTPUT);    //relay is an output
  digitalWrite(RELAY_PIN, LOW);  //turn off the relay :-)

  //button adc input
  pinMode(BUTTON_ADC_PIN, INPUT);         //ensure A0 is an input
  digitalWrite(BUTTON_ADC_PIN, LOW);      //ensure pullup is off on A0

  //lcd backlight control
  digitalWrite(LCD_BACKLIGHT_PIN, HIGH);  //backlight control pin is high (on)
  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);     //backlight is an output

  //set up the menu
  mm.add_item(&mm_temp, &on_item1_selected);
  mm.add_item(&mm_duty, &on_item2_selected);
  ms.set_root_menu(&mm);
}

void loop(){
  #ifdef DEBUG
  uint16_t buttonVoltage = analogRead( BUTTON_ADC_PIN );
  uint16_t thermistorVoltage = analogRead (THERMISTOR_PIN);
  lcd.setCursor( 0, 0 );
  lcd.print("TH:");
  lcd.print( readTemp(), DEC );
  lcd.setCursor( 8, 0 );
  lcd.print("BN:");
  lcd.print( readButtons(), DEC );
  
  lcd.setCursor( 0, 1 );
  lcd.print("TH:");
  lcd.print( thermistorVoltage, DEC );
  lcd.setCursor( 8, 1 );
  lcd.print("BN:");
  lcd.print( buttonVoltage, DEC );
  delay(250); //wait a bit so the voltages don't flicker stupid fast... this means it takes 1/4 of a second to read a button, though.
  #endif

  //Step 1: update the LCD
  displayStatus();

  //Step 2: update the menu
  displayMenu();

  //Step 3: check buttons
  menuHandler(readButtons());
  
  //Step 4: manage heater
}

//draw the top line
void displayStatus(){
  lcd.setCursor(0,0);
  lcd.print("T");
  lcd.print(pad(readTemp(), 3));
  lcd.print("/");
  lcd.print(pad(SetPoint, 3));
  lcd.setCursor(0,9);
  lcd.print(pad(MaxDutyCycle, 2));
  lcd.setCursor(0,13);
  if(RelayOn){
    lcd.print("ON ");
  }else{
    lcd.print("OFF");
  }
}

String pad(uint8_t input, uint8_t len){
  String ret = String(input, DEC);
  if(len >= 2){
    if(input <= 9){
      ret = " "+ret;
    }
  }
  if(len >= 3){
    if(input <= 99){
      ret = " "+ret;
    }
  }
  return ret;
}

//draw the menu - it gets the bottom line of the LCD, unless in debug mode.
void displayMenu(){
}



//Read a button, and return the 
uint8_t readButtons()
{
   uint16_t buttonVoltage;
   uint16_t thermistorVoltage;
   uint8_t button = BUTTON_NONE;   // return no button pressed if the below checks don't write to btn
 
   //read the button ADC pin voltage
   buttonVoltage = analogRead( BUTTON_ADC_PIN );

   //check if the voltage falls within valid voltage windows
   if(    buttonVoltage >= ( RIGHT_10BIT_ADC - BUTTONHYSTERESIS )
       && buttonVoltage <= ( RIGHT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_RIGHT;
   }
   else if(   buttonVoltage >= ( UP_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( UP_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_UP;
   }
   else if(   buttonVoltage >= ( DOWN_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( DOWN_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_DOWN;
   }
   else if(   buttonVoltage >= ( LEFT_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( LEFT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_LEFT;
   }
   else if(   buttonVoltage >= ( SELECT_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( SELECT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_SELECT;
   }
   //handle button flags for just pressed and just released events
   if( ( buttonWas == BUTTON_NONE ) && ( button != BUTTON_NONE ) )
   {
      //the button was just pressed, set buttonJustPressed, this can optionally be used to trigger a once-off action for a button press event
      //it's the duty of the receiver to clear these flags if it wants to detect a new button change event
      buttonJustPressed  = true;
      buttonJustReleased = false;
   }
   if( ( buttonWas != BUTTON_NONE ) && ( button == BUTTON_NONE ) )
   {
      buttonJustPressed  = false;
      buttonJustReleased = true;
   }
 
   //save the latest button value, for change event detection next time round
   buttonWas = button;
 
   return(button);
}
