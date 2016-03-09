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
 *   The system is entirely state-based, and does not use any interrupts.
 *   This means the heater could be on for up to 750ms longer than it should be... which I think is OK.
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

//all debugging is sent over the serial port
//#define DEBUG

#define HYSTERESIS 3        //don't fret if we're within a few degrees

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
MenuItem mm_temp("Change Temp");
MenuItem mm_duty("Change Duty Cycle");

//state variables
uint16_t   SetPoint;     //what temp we're trying to stay at
uint16_t   MaxDutyCycle; //0-100%, how often to turn the heater on at most
uint16_t   CycleTime;    //length of a cycle, in milliseconds
uint16_t   CycleStart;   //this is counted in milliseconds, 
uint16_t   OnStart;      //time when the heater turned on - to prevent overclicky, we have a minimum on time
uint16_t   MinOnTime;    //how long is the minimum time the heater can heat for
bool       RelayOn;      //is the relay on?
bool       ChangeTemp;   //are we changing the temp?
bool       ChangeDuty;   //are we changing the duty cycle?

void setup(){
  //fire up the LCD first
  lcd.begin(16,2);
  lcd.setCursor(0,1);
  lcd.print("FilaMEAT =^..^= ");
  lcd.setCursor(0,1);
  lcd.print("Paul Chase  v0.0");

#ifdef DEBUG
  Serial.begin(9600);
#endif

  //initialize state
  SetPoint = 0;           //start with heaters off
  MaxDutyCycle = 50;      //seems like a good cycle for starters
  RelayOn = false;        //start off
  CycleTime = 10*1000;    //10 second cycle to start with
  ChangeTemp = false;     //in change temp mode, the direction buttons change the temperature
  ChangeDuty = false;     //in change duty mode, the direction buttons change the duty cycle
  CycleStart = millis();  //this is when the current cycle started
  OnStart = 0;            //when the heater started to go on
  MinOnTime = 2000;       //minimum time to keep the heater on
  
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
  mm.add_item(&mm_temp, &on_temp_selected);
  mm.add_item(&mm_duty, &on_duty_selected);
  ms.set_root_menu(&mm);
}

void loop(){
#ifdef DEBUG2
  uint16_t buttonVoltage = analogRead( BUTTON_ADC_PIN );
  uint16_t thermistorVoltage = analogRead (THERMISTOR_PIN);
  Serial.print("Temperature: ");
  Serial.println( readTemp(), DEC );
  
  Serial.print("Thermistor: ");
  Serial.println( thermistorVoltage, DEC );

  Serial.print("SetPoint: ");
  Serial.println( SetPoint, DEC );

  
  
  Serial.print("Button: ");
  Serial.println( readButtons(), DEC );
  
  Serial.print("ButtonRaw: ");
  Serial.println( buttonVoltage, DEC );

  Serial.println();
  delay(250);
#endif

  //Step 1: update the LCD
  displayStatus();

  //Step 2: check buttons
  menuHandler(readButtons());
  
  //Step 3: update the menu
  displayMenu();
  
  //Step 4: manage heater
  manageHeater(); //todo: make this operate on an interrupt... right now it can be delayed.
}

void manageHeater(){
  //read the current temperature
  uint16_t temp = readTemp();
  uint16_t curCycle = millis() - CycleStart;

  if(curCycle > CycleTime){
    CycleStart = millis();
    curCycle = 0;
#ifdef DEBUG
  Serial.println("New Cycle");
#endif
    return;
  }

  Serial.print("curCycle:");
  Serial.println(curCycle, DEC);
  Serial.print("MaxDutyCycle:");
  Serial.println(MaxDutyCycle, DEC);
  Serial.print("CycleTime:");
  Serial.println(CycleTime, DEC);
  

  //We're never on at the start of a duty cycle
  if(curCycle < CycleTime - MaxDutyCycle*(CycleTime/100)){
    RelayOn = false;
    digitalWrite(RELAY_PIN, LOW);  //turn off the relay :-)
    return;
  }
  
  //Are we too hot?
  if(temp > SetPoint){
    RelayOn = false;
    digitalWrite(RELAY_PIN, LOW);  //turn off the relay :-)
    return;
  }

  //Should we be hotter?
  if(temp+HYSTERESIS < SetPoint){
    RelayOn = true;
    digitalWrite(RELAY_PIN, HIGH);  //turn on the relay :-)
  }
}

//draw the top line
void displayStatus(){
  lcd.setCursor(0,0);
  lcd.print("T");
  lcd.print(padl(readTemp(), 3));
  if(ChangeTemp){
    lcd.print("*");
  }else{
    lcd.print("/");
  }
  lcd.print(padr(SetPoint, 3));
  if(ChangeTemp || ChangeDuty){
    lcd.print("*");
  }else{
    lcd.print(" ");
  }
  
  //lcd.setCursor(0,9);
  lcd.print(padl(MaxDutyCycle, 2));
  if(ChangeDuty){
    lcd.print("*");
  }else{
    lcd.print(" ");
  }
  
  //lcd.setCursor(0,13);
  if(RelayOn){
    lcd.print("ON ");
  }else{
    lcd.print("OFF");
  }
}

String padr(uint8_t input, uint8_t len){
  String ret = String(input, DEC);
  if(len >= 2){
    if(input <= 9){
      ret = ret+" ";
    }
  }
  if(len >= 3){
    if(input <= 99){
      ret = ret+" ";
    }
  }
  return ret;
}

String padl(uint8_t input, uint8_t len){
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
  
  // Display the menu
  Menu const* cp_menu = ms.get_current_menu();

  if(!ChangeTemp && !ChangeDuty){
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
    //lcd.print(cp_menu->get_name());
    lcd.print(cp_menu->get_selected()->get_name());
  }
}

void menuHandler(uint8_t button){
  if(!buttonJustPressed){
    return;
  }
  buttonJustPressed = false;
  switch(button){
    case BUTTON_NONE:
      return;
    case BUTTON_LEFT:
      if(ChangeDuty){
        MaxDutyCycle = MaxDutyCycle - 1;
        return;
      }
      if(ChangeTemp){
        SetPoint = SetPoint - 1;
        return;
      }
      ms.back();
      break;
      
    case BUTTON_RIGHT:
      if(ChangeDuty){
        MaxDutyCycle = MaxDutyCycle + 1;
        return;
      }
      if(ChangeTemp){
        SetPoint = SetPoint + 1;
        return;
      }
      ms.select();
      break;

    case BUTTON_DOWN:
      if(ChangeDuty){
        MaxDutyCycle = MaxDutyCycle - 1;
        return;
      }
      if(ChangeTemp){
        SetPoint = SetPoint - 10;
        return;
      }
      ms.next();
      break;
      
    case BUTTON_UP:
      if(ChangeDuty){
        MaxDutyCycle = MaxDutyCycle + 1;
        return;
      }
      if(ChangeTemp){
        SetPoint = SetPoint + 10;
        return;
      }
      ms.prev();
      break;

    case BUTTON_SELECT:
      lcd.clear();
      if(ChangeDuty){
        ChangeDuty = false;
        return;
      }
      if(ChangeTemp){
        ChangeTemp = false;
        return;
      }
      ms.select();
      break;
  }
}

void on_temp_selected(MenuItem* p_menu_item){
  lcd.setCursor(0,1);
  lcd.print("Changing Temp   ");
  delay(500);
  ChangeTemp = true;
  ChangeDuty = false;
}

void on_duty_selected(MenuItem* p_menu_item){
  lcd.setCursor(0,1);
  lcd.print("Changing Duty   ");
  delay(500);
  ChangeTemp = false;
  ChangeDuty = true;
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

short readTemp(){
  short thermistorVoltage = 0;
  short lowTemp = 0;
  short highTemp = 0;
  int i;
  
  //we sample a bunch of readings to get a stable thermistor value
  for(i=0; i<OVERSAMPLENR; i++){
    thermistorVoltage += analogRead (THERMISTOR_PIN);
  }

  #ifdef DEBUG2
  Serial.print("oversample: ");
  Serial.println( OVERSAMPLENR, DEC );
  Serial.print("Tvolts: ");
  Serial.println( thermistorVoltage, DEC );
  #endif
  

  //this is a temperature overflow - greater than the sensor can handle.
  //Receiving program should shut down.
  if(thermistorVoltage < temptable[0][0]){
    return 0;
  }

  //note the -2 - if we read off the other end of the table, we'll return 0
  //That's a thermal underflow, which probably means the thermistor's unplugged.
  for (i=1; i < 60; i++){
    
    if(thermistorVoltage < temptable[i][0]){
      lowTemp = i;
      highTemp = i-1;
      break;
    }
  }
  
  #ifdef DEBUG3
  Serial.print("lt[");
  Serial.print(lowTemp, DEC);
  Serial.print("][1] ");
  Serial.println( temptable[lowTemp][1], DEC );

  Serial.print("ht[");
  Serial.print(highTemp, DEC);
  Serial.print("][1] ");
  Serial.println( temptable[highTemp][1], DEC );

  Serial.print("tvolts: ");
  Serial.println(thermistorVoltage, DEC);

  Serial.print("ht[");
  Serial.print(highTemp, DEC);
  Serial.print("][0] ");
  Serial.println( temptable[highTemp][0], DEC );

  Serial.print("lt[");
  Serial.print(lowTemp, DEC);
  Serial.print("][0] ");
  Serial.println( temptable[lowTemp][0], DEC );
  #endif

  
  
  return temptable[highTemp][1]-(temptable[highTemp][1]-temptable[lowTemp][1])*(thermistorVoltage-temptable[highTemp][0])/(temptable[lowTemp][0]-temptable[highTemp][0]);
}

