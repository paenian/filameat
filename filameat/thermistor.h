#define OVERSAMPLENR 16
#define THERMISTOR_PIN           A1  // A1 is the thermistor read pin

//this is using a 100K thermistor w/a 4.7kohm pullup.
const short temptable[][2] PROGMEM = {
  {23 * OVERSAMPLENR, 300},
  {25 * OVERSAMPLENR, 295},
  {27 * OVERSAMPLENR, 290},
  {28 * OVERSAMPLENR, 285},
  {31 * OVERSAMPLENR, 280},
  {33 * OVERSAMPLENR, 275},
  {35 * OVERSAMPLENR, 270},
  {38 * OVERSAMPLENR, 265},
  {41 * OVERSAMPLENR, 260},
  {44 * OVERSAMPLENR, 255},
  {48 * OVERSAMPLENR, 250},
  {52 * OVERSAMPLENR, 245},
  {56 * OVERSAMPLENR, 240},
  {61 * OVERSAMPLENR, 235},
  {66 * OVERSAMPLENR, 230},
  {71 * OVERSAMPLENR, 225},
  {78 * OVERSAMPLENR, 220},
  {84 * OVERSAMPLENR, 215},
  {92 * OVERSAMPLENR, 210},
  {100 * OVERSAMPLENR, 205},
  {109 * OVERSAMPLENR, 200},
  {120 * OVERSAMPLENR, 195},
  {131 * OVERSAMPLENR, 190},
  {143 * OVERSAMPLENR, 185},
  {156 * OVERSAMPLENR, 180},
  {171 * OVERSAMPLENR, 175},
  {187 * OVERSAMPLENR, 170},
  {205 * OVERSAMPLENR, 165},
  {224 * OVERSAMPLENR, 160},
  {245 * OVERSAMPLENR, 155},
  {268 * OVERSAMPLENR, 150},
  {293 * OVERSAMPLENR, 145},
  {320 * OVERSAMPLENR, 140},
  {348 * OVERSAMPLENR, 135},
  {379 * OVERSAMPLENR, 130},
  {411 * OVERSAMPLENR, 125},
  {445 * OVERSAMPLENR, 120},
  {480 * OVERSAMPLENR, 115},
  {516 * OVERSAMPLENR, 110},
  {553 * OVERSAMPLENR, 105},
  {591 * OVERSAMPLENR, 100},
  {628 * OVERSAMPLENR, 95},
  {665 * OVERSAMPLENR, 90},
  {702 * OVERSAMPLENR, 85},
  {737 * OVERSAMPLENR, 80},
  {770 * OVERSAMPLENR, 75},
  {801 * OVERSAMPLENR, 70},
  {830 * OVERSAMPLENR, 65},
  {857 * OVERSAMPLENR, 60},
  {881 * OVERSAMPLENR, 55},
  {903 * OVERSAMPLENR, 50},
  {922 * OVERSAMPLENR, 45},
  {939 * OVERSAMPLENR, 40},
  {954 * OVERSAMPLENR, 35},
  {966 * OVERSAMPLENR, 30},
  {977 * OVERSAMPLENR, 25},
  {985 * OVERSAMPLENR, 20},
  {993 * OVERSAMPLENR, 15},
  {999 * OVERSAMPLENR, 10},
  {1004 * OVERSAMPLENR, 5},
  {1008 * OVERSAMPLENR, 0} //safety
};

uint16_t readTemp(){
  uint16_t thermistorVoltage = 0;
  uint16_t lowTemp = 0;
  uint16_t highTemp = 0;
  uint16_t i;
  
  //we sample a bunch of readings to get a stable thermistor value
  for(i=0; i<OVERSAMPLENR; i++){
    thermistorVoltage = analogRead (THERMISTOR_PIN);
  }

  //this is a temperature overflow - greater than the sensor can handle.
  //Receiving program should shut down.
  if(thermistorVoltage < temptable[i][0]){
    return 0;
  }

  //note the -2 - if we read off the other end of the table, we'll return 0
  //That's a thermal underflow, which probably means the thermistor's unplugged.
  for (i=0; i < (sizeof(temptable)/(sizeof(short)*2)) - 2; i++){
    if(thermistorVoltage > temptable[i][0]){
      highTemp = i;
      lowTemp = i+1;
    }
  }
  
  return temptable[lowTemp][1]+(temptable[highTemp][1]-temptable[lowTemp][1])*(thermistorVoltage-temptable[highTemp][0])/(temptable[lowTemp][0]-temptable[highTemp][0]);
}
