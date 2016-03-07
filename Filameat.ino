
/*--------------------------------------------------------------------------------------
  Includes
--------------------------------------------------------------------------------------*/
#include <LiquidCrystal.h>   // include LCD library
#include "arduino-menusystem/MenuSystem.h"	//menu library


/*--------------------------------------------------------------------------------------
  Pins In Use
--------------------------------------------------------------------------------------*/
//LCDButtons.h PINS IN USE
#define BUTTON_ADC_PIN           A0  // A0 is the button ADC input
#define THERMISTOR_PIN           A1  // A1 is the thermistor read pin
#define LCD_BACKLIGHT_PIN         10  // D10 controls LCD backlight

//My pins
#define RELAY_PIN                3   // D3 controls the relay


/*--------------------------------------------------------------------------------------
  Global Variables
--------------------------------------------------------------------------------------*/
byte oldstamp			= 0;			 //Used in the relay timing - we don't want too much hysteresis.  Should be deprecated soon.
bool relayState			= false;		 //True if the relay is on; False otherwise.


/*--------------------------------------------------------------------------------------
  Init the LCD library with the LCD pins to be used
--------------------------------------------------------------------------------------*/
LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );   //Pins for the freetronics 16x2 LCD shield. LCD: ( RS, E, LCD-D4, LCD-D5, LCD-D6, LCD-D7 )
LCDButtons buttons(lcd);

/*--------------------------------------------------------------------------------------
  setup()
--------------------------------------------------------------------------------------*/
void setup()
{
   //thermistor input
   pinMode( THERMISTOR_PIN, INPUT );         //ensure thermistor is an input
   digitalWrite( THERMISTOR_PIN, LOW );      //ensure pullup is off on thermistor

   //relay output
   pinMode( RELAY_PIN, OUTPUT );    //relay is an output
   digitalWrite( RELAY_PIN, LOW );  //turn off the relay :-)
   
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

/*--------------------------------------------------------------------------------------
  loop()
  Arduino main loop
--------------------------------------------------------------------------------------*/
void loop()
{
   byte button;
   byte timestamp;

 
   //get the latest button pressed, also the buttonJustPressed, buttonJustReleased flags
   button = buttons.ReadButtons();
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
        relayState = !relayState;
        if(relayState == true){
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



