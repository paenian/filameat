/*
 
  This example code is in the public domain.
 
  ---------------------------------------------------------------------
 
  This program demonstrates button detection, LCD text/number printing,
  and LCD backlight control on the Freetronics LCD & Keypad Shield, connected to an Arduino board.
 
 
  Pins used by LCD & Keypad Shield:
 
    A0: Buttons, analog input from voltage ladder
    A1: Thermistor read
    D3: Relay control pin
    D4: LCD bit 4
    D5: LCD bit 5
    D6: LCD bit 6
    D7: LCD bit 7
    D8: LCD RS
    D9: LCD E
    D10: LCD Backlight (high = on, also has pullup high so default is on)
 
  ADC voltages for the 5 buttons on analog input pin A0:
 
    RIGHT:  0.00V :   0 @ 8bit ;   0 @ 10 bit
    UP:     0.71V :  36 @ 8bit ; 145 @ 10 bit
    DOWN:   1.61V :  82 @ 8bit ; 329 @ 10 bit
    LEFT:   2.47V : 126 @ 8bit ; 505 @ 10 bit
    SELECT: 3.62V : 185 @ 8bit ; 741 @ 10 bit
*/
/*--------------------------------------------------------------------------------------
  Includes
--------------------------------------------------------------------------------------*/
#include <LiquidCrystal.h>   // include LCD library
/*--------------------------------------------------------------------------------------
  Defines
--------------------------------------------------------------------------------------*/
// Pins in use
#define BUTTON_ADC_PIN           A0  // A0 is the button ADC input
#define THERMISTOR_PIN           A1  // A1 is the thermistor read pin
#define RELAY_PIN                3   // D3 controls the relay
#define LCD_BACKLIGHT_PIN         10  // D10 controls LCD backlight
// ADC readings expected for the 5 buttons on the ADC input
#define RIGHT_10BIT_ADC         815  // right
#define UP_10BIT_ADC            930  // up
#define DOWN_10BIT_ADC          903  // down
#define LEFT_10BIT_ADC          856  // left
#define SELECT_10BIT_ADC        612  // select
#define BUTTONHYSTERESIS         10  // hysteresis for valid button sensing window
//return values for ReadButtons()
#define BUTTON_NONE               0  // 
#define BUTTON_RIGHT              1  // 
#define BUTTON_UP                 2  // 
#define BUTTON_DOWN               3  // 
#define BUTTON_LEFT               4  // 
#define BUTTON_SELECT             5  // 
//some example macros with friendly labels for LCD backlight/pin control, tested and can be swapped into the example code as you like
#define LCD_BACKLIGHT_OFF()     digitalWrite( LCD_BACKLIGHT_PIN, LOW )
#define LCD_BACKLIGHT_ON()      digitalWrite( LCD_BACKLIGHT_PIN, HIGH )
#define LCD_BACKLIGHT(state)    { if( state ){digitalWrite( LCD_BACKLIGHT_PIN, HIGH );}else{digitalWrite( LCD_BACKLIGHT_PIN, LOW );} }
/*--------------------------------------------------------------------------------------
  Variables
--------------------------------------------------------------------------------------*/
byte buttonJustPressed  = false;         //this will be true after a ReadButtons() call if triggered
byte buttonJustReleased = false;         //this will be true after a ReadButtons() call if triggered
byte buttonWas          = BUTTON_NONE;   //used by ReadButtons() for detection of button events
/*--------------------------------------------------------------------------------------
  Init the LCD library with the LCD pins to be used
--------------------------------------------------------------------------------------*/
LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );   //Pins for the freetronics 16x2 LCD shield. LCD: ( RS, E, LCD-D4, LCD-D5, LCD-D6, LCD-D7 )
/*--------------------------------------------------------------------------------------
  setup()
  Called by the Arduino framework once, before the main loop begins
--------------------------------------------------------------------------------------*/
void setup()
{
   //thermistor input
   pinMode( THERMISTOR_PIN, INPUT );         //ensure thermistor is an input
   digitalWrite( THERMISTOR_PIN, LOW );      //ensure pullup is off on thermistor

   //relay output
   pinMode( RELAY_PIN, OUTPUT );    //relay is an output
   digitalWrite( RELAY_PIN, LOW );  //turn off the relay :-)
     
   //button adc input
   pinMode( BUTTON_ADC_PIN, INPUT );         //ensure A0 is an input
   digitalWrite( BUTTON_ADC_PIN, LOW );      //ensure pullup is off on A0
   
   //lcd backlight control
   digitalWrite( LCD_BACKLIGHT_PIN, HIGH );  //backlight control pin is high (on)
   pinMode( LCD_BACKLIGHT_PIN, OUTPUT );     //backlight is an output
   
   //set up the LCD number of columns and rows: 
   lcd.begin( 16, 2 );
   //Print some initial text to the LCD.
   lcd.setCursor( 0, 0 );   //top left
   //          1234567890123456
   lcd.print( "LinkSprite  16x2" );
   //
   lcd.setCursor( 0, 1 );   //bottom left
   //          1234567890123456
   lcd.print( "Btn:" );
}

   byte oldstamp = 0;
   bool relayToggle = false;

/*--------------------------------------------------------------------------------------
  loop()
  Arduino main loop
--------------------------------------------------------------------------------------*/
void loop()
{
   byte button;
   byte timestamp;

 
   //get the latest button pressed, also the buttonJustPressed, buttonJustReleased flags
   button = ReadButtons();
   //blank the demo text line if a new button is pressed or released, ready for a new label to be written
   if( buttonJustPressed || buttonJustReleased )
   {
     lcd.setCursor( 4, 1 );
     lcd.print( "            " );
   }
   //show text label for the button pressed
   switch( button )
   {
      case BUTTON_NONE:
      {
         break;
      }
      case BUTTON_RIGHT:
      {
         lcd.setCursor( 4, 1 );
         lcd.print( "RIGHT" );
         delay( 300 );
         break;
      }
      case BUTTON_UP:
      {
         lcd.setCursor( 4, 1 );
         lcd.print( "UP" );
         delay( 300 );
         break;
      }
      case BUTTON_DOWN:
      {
         lcd.setCursor( 4, 1 );
         lcd.print( "DOWN" );
         delay( 300 );
         break;
      }
      case BUTTON_LEFT:
      {
        lcd.setCursor( 4, 1 );
        lcd.print( "LEFT" );
        delay( 300 );
        break;
     }
     case BUTTON_SELECT:
     {
        lcd.setCursor( 4, 1 );
        lcd.print( "SELECT-FLASH" );    
 
        //SELECT is a special case, it pulses the LCD backlight off and on for demo
        digitalWrite( LCD_BACKLIGHT_PIN, LOW );
        delay( 150 );
        digitalWrite( LCD_BACKLIGHT_PIN, HIGH );   //leave the backlight on at exit
        delay( 150 );
 
        break;
      }
      default:
     {
        break;
     }
   }
   // print the number of seconds since reset (two digits only)
   timestamp = ( (millis() / 1000) % 100 );   //"% 100" is the remainder of a divide-by-100, which keeps the value as 0-99 even as the result goes over 100
   lcd.setCursor( 14, 1 );
   if( timestamp <= 9 )
      lcd.print( " " );   //quick trick to right-justify this 2 digit value when it's a single digit
   lcd.print( timestamp, DEC );

   lcd.setCursor( 5, 1 );
   lcd.print( oldstamp, DEC );

   if(timestamp != oldstamp){
      if(timestamp % 10 == 0){
        relayToggle = !relayToggle;
        if(relayToggle == true){
          lcd.setCursor( 13, 1 );
          lcd.print("I");
          digitalWrite( RELAY_PIN, HIGH);
        }else{
          lcd.setCursor( 13, 1 );
          lcd.print("O");
          digitalWrite( RELAY_PIN, LOW);
        }
      }
   }
   
   oldstamp = timestamp;
   
        
/*  
   //debug/test display of the adc reading for the button input voltage pin.
   lcd.setCursor(12, 0);
   lcd.print( "    " );          //quick hack to blank over default left-justification from lcd.print()
   lcd.setCursor(12, 0);         //note the value will be flickering/faint on the LCD
   lcd.print( analogRead( BUTTON_ADC_PIN ) );
*/
   //clear the buttonJustPressed or buttonJustReleased flags, they've already done their job now.
   if( buttonJustPressed )
      buttonJustPressed = false;
   if( buttonJustReleased )
      buttonJustReleased = false;
}
/*--------------------------------------------------------------------------------------
  ReadButtons()
  Detect the button pressed and return the value
  Uses global values buttonWas, buttonJustPressed, buttonJustReleased.
--------------------------------------------------------------------------------------*/
byte ReadButtons()
{
   unsigned int buttonVoltage;
   unsigned int thermistorVoltage;
   byte button = BUTTON_NONE;   // return no button pressed if the below checks don't write to btn
 
   //read the button ADC pin voltage
   buttonVoltage = analogRead( BUTTON_ADC_PIN );
   thermistorVoltage = analogRead (THERMISTOR_PIN);
   lcd.setCursor( 9, 1 );
   lcd.print( thermistorVoltage, DEC );
   lcd.print( " " ); lcd.print( " " ); 
   //delay( 300 );
   //sense if the voltage falls within valid voltage windows
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
 
   return( button );
}
