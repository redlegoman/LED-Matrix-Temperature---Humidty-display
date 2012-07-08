/*
*/
#include "DHT.h"
#include "adafruitHT1632.h"
#define DHTPIN 7     // what pin we're connected to (inside temp sensor)
#define DHTOUTSIDEPIN 6 // (outside pin sensor)

// ---- TEMPERATURE SENSORS:
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
DHT dhtoutside(DHTOUTSIDEPIN, DHTTYPE);

#define DATA 13 // LED Matrix
#define WR   12 // LED Matrix
#define CS   11 // LED Matrix
#define CS2  5  // LED Matrix

// use this line for single matrix
HT1632LEDMatrix matrix = HT1632LEDMatrix(DATA, WR, CS);
// use this line for two matrices!
//HT1632LEDMatrix matrix = HT1632LEDMatrix(DATA, WR, CS, CS2);

#include <stdio.h>
#include <string.h>
#include <DS1302.h>

// VARS ----------------------------------
int incomingByte = 0;   // for incoming serial data
const int ledPin = 5; // pin to which LED should be attached.
int LEDON = 0;
//the time when the sensor outputs a low impulse
long unsigned int lowIn;  
long unsigned int pause = 2000; 
String S3; // the message to scrol, plus the temperature
char test[20]; // used in the rounding of the float to 1 decimaml place
  
long unsigned int  starttime = millis(); // the time we started
int show_in = 0;
int show_out = 0;
int show_outH = 0;
int showtime = 0;
int showday = 1;
int readtemp = 1; // should we read the temperature again?
int scrolldelay = 25; // lower number speeds up scrolling
int LEDwidth = 32; // the width of the matrix;
int Fontwidth = 6 ; //width of fonts plus a space
String theTime = "00:00";
String theDay = "NoSetDay";
long int SwitchTime = 4000; // time between switching modes (outside, inside, time etc.)
String oldmsg = "99:99";
float GInsideTemp = 00.0;
float GOutsideTemp = 00.0;
float GInsideH = 00.0;
float GOutsideH = 00.0;
long int startJson = 0;


/* Set the appropriate digital I/O pin connections */
uint8_t CE_PIN   = 8; // RTC
uint8_t IO_PIN   = 9; // RTC
uint8_t SCLK_PIN = 10;// RTC

/* Create buffers */
char buf[50];
char day[10];
char mon[10];

/* Create a DS1302 object */
DS1302 rtc(CE_PIN, IO_PIN, SCLK_PIN);

void print_time()
{
  /* Get the current time and date from the chip */
  Time t = rtc.time();
  /* Name the day of the week */
  memset(day, 0, sizeof(day));  /* clear day buffer */
  switch (t.day) {
    case 1:
      strcpy(day, "Sunday");
      break;
    case 2:
      strcpy(day, "Monday");
      break;
    case 3:
      strcpy(day, "Tuesday");
      break;
    case 4:
      strcpy(day, "Wednesday");
      break;
    case 5:
      strcpy(day, "Thursday");
      break;
    case 6:
      strcpy(day, "Friday");
      break;
    case 7:
      strcpy(day, "Saturday");
      break;
  }
  switch (t.mon) {
    case 1:
      strcpy(mon, "January");
      break;
    case 2:
      strcpy(mon, "February");
      break;
    case 3:
      strcpy(mon, "March");
      break;
    case 4:
      strcpy(mon, "April");
      break;
    case 5:
      strcpy(mon, "May");
      break;
    case 6:
      strcpy(mon, "June");
      break;
    case 7:
      strcpy(mon, "July");
      break;
    case 8:
      strcpy(mon,"August");
      break;
    case 9:
      strcpy(mon,"September");
      break;
    case 10:
      strcpy(mon,"October");
      break;
    case 11:
      strcpy(mon,"November");
      break;
    case 12:
      strcpy(mon,"December");
      break;  
  }
  char suffix[3] = "th";
  switch (t.date) {
    case 1: 
    case 21:
    case 31:
      strcpy(suffix,"st"); break;
    case 2: 
    case 22:
      strcpy(suffix,"nd"); break;
    case 3: 
    case 23:
      strcpy(suffix,"rd"); break;
  }
  /* Format the time and date and insert into the temporary buffer */
  //snprintf(buf, sizeof(buf), "%s %04d-%02d-%02d %02d:%02d:%02d",
  //         day,
  //         t.yr, t.mon, t.date,
  //         t.hr, t.min, t.sec);
  /* Print the formatted string to serial so we can see the time */
  //Serial.println(buf);
  snprintf(buf,sizeof(buf), "%02d:%02d", t.hr, t.min);
  theTime=buf;
  snprintf(buf,sizeof(buf), "%s %2d%s %s %04d", day, t.date, suffix, mon, t.yr);
  theDay=buf;

}


  
void setup()
{
  Serial.begin(9600);
  //  HT1632.begin( 11, 12, 13 );
  dht.begin();
  dhtoutside.begin();


  matrix.begin(HT1632_COMMON_8NMOS);  
  matrix.fillScreen();
  GOutsideTemp = dhtoutside.readTemperature();
  GOutsideH = dhtoutside.readHumidity();
  GInsideTemp = dht.readTemperature();
  GInsideH = dht.readHumidity();
  delay(500);
  matrix.clearScreen();
  long int startJson = millis();
  
  
}

//-- SHOWSTUFF --------------------------------------------------------------------------
void showStuff(String msg){
  if(msg != oldmsg){
    matrix.clearScreen();
    oldmsg = msg;
  }
  matrix.setTextSize(1);    // size 1 == 8 pixels high
  matrix.setTextColor(1);   // 'lit' LEDs
  matrix.setCursor(0,0); // top left
  matrix.print(msg);
  matrix.writeScreen();
  readKey();

  //delay(250);

}
// -- SCROLL STUFF ----------------------------------------------------------------------
  void scrollStuff(String msg) {

    matrix.clearScreen(); 
    // draw some text!
    matrix.setTextSize(1);    // size 1 == 8 pixels high
    matrix.setTextColor(2);   // 'lit' LEDs

    int i = 0;
    int stufflen = msg.length();
    for(i=LEDwidth; i>0-(stufflen*Fontwidth); i--){
      readKey();
      matrix.setCursor(i, 0);   // start at top left, with one pixel of spacing
      matrix.print(msg);
      matrix.writeScreen();
      delay(scrolldelay);
      matrix.clearScreen();
    }
  }

//---- READKEY ----------------------------------
void readKey(){
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    if(incomingByte == 117)  { //"u"
      float to = dhtoutside.readTemperature();
      if (isnan(to)){
        Serial.println("\n COULD NOT READ!");
      }else{
        digitalWrite(ledPin, HIGH);
        Serial.print("OUTT:");
        Serial.print(to,1);
        Serial.println(":");
        LEDON=1;
        lowIn = millis();
      }
    }
    if(incomingByte == 116) { // "t"
      float t = dht.readTemperature();
      if (isnan(t)){
        Serial.println("\n COULD NOT READ!");
      }else{
        digitalWrite(ledPin, HIGH);
        Serial.print("INST:");
        Serial.print(t,1);
        Serial.println(":");
        LEDON=1;
        lowIn = millis();
      }
    }
    if(incomingByte == 104) { // "h"
      float h = dht.readHumidity();
      if (isnan(h)){
        Serial.println("\n COULD NOT READ!");
      }else{
        digitalWrite(ledPin, HIGH);
        Serial.print("INSH:");
        Serial.print(h,1);
        Serial.println(":");
        LEDON=1;
        lowIn = millis();
      }
    }
    if(incomingByte == 105) { // "i"
      float ho = dhtoutside.readHumidity();
      if (isnan(ho)){
        Serial.println("\n COULD NOT READ!");
      }else{
        digitalWrite(ledPin, HIGH);
        Serial.print("OUTH:");
        Serial.print(ho,1);
        Serial.println(":");
        
        LEDON=1;
        lowIn = millis();
      }
    }
    
    
    
  }
}

void outJSON()
{
  Serial.println("{\"humidty\":\"" + String((long)round(GInsideH)) +
			"\", \"celsius\":\"" + String((long)round(GInsideTemp)) +
			"\", \"humidity2\":\"" + String((long)round(GOutsideH)) +
			"\", \"celsius2\":\"" + String((long)round(GOutsideTemp)) +
			"\"}");
}

/* Loop and print the time every second */
void loop()
{
  if(show_in == 1){
    if(readtemp == 1){ // only read the temperature if the delay has passed
      float temperature = dht.readTemperature();
      GInsideTemp = temperature;
      readtemp = 0; // stop reading the temperature
      S3 = "inside ";
      S3 += floatToString(test,temperature,1,3); // round the float to 1 decimal place

      char X = 64;
      S3 += X;
    }
    scrollStuff(S3);
  }
  if(show_out == 1){
    if(readtemp == 1){
      float temperature = dhtoutside.readTemperature();
      GOutsideTemp = temperature;
      readtemp = 0;
      S3 = "outside ";
      S3 += floatToString(test,temperature,1,3);

      char X = 64;
      S3 += X;
    }
    scrollStuff(S3);
  }
    if(show_outH == 1){
      if(readtemp == 1){
        float humidity = dhtoutside.readHumidity();
        GOutsideH = humidity;
        float humidity2 = dht.readHumidity();
        GInsideH = humidity2;

        readtemp = 0;
        S3 = "Humidity ";
        S3 += floatToString(test,humidity,1,3);
        S3 += "%";
      }
      scrollStuff(S3); 
  }
  
  if(showtime == 1){
    print_time();
    S3 = "00:00";
    showStuff(theTime);
    //delay (2000);
  }
  
  if(showday == 1){
    print_time();
    scrollStuff(theDay);
  }
  

  
  if(millis() - starttime > SwitchTime){
    matrix.clearScreen();
    readtemp=1;
    if(show_in == 1){
      show_in = 0;
      show_out = 1;
    }else
    if(show_out == 1){
      show_out = 0;
      show_outH = 1;
    }else
    if(show_outH == 1){
      show_outH = 0;
      showday = 1;
    }else
    if(showtime == 1){
      showtime = 0;
      show_in = 1;
    }else
    if(showday == 1){
      showday = 0;
      showtime = 1;
    }
     
    starttime = millis();
  }

//  Serial.println(" .");
    
  if (LEDON == 1){
    if(millis() - lowIn > pause){
      digitalWrite(ledPin, LOW); // turn off the LED
      LEDON=0;
    }else{
      //Serial.println(millis() - lowIn);
      delay(0);
    }
  }
  
  
  
  long int pauseJson = 5000;
  if(millis() - startJson > pauseJson){
    //Serial.println(millis());
    outJSON();  
    startJson = millis();
  }
    
}
  
  









 //-------------------------------------------------------------------------------------------------- 
  char * floatToString(char * outstr, double val, byte precision, byte widthp){
  char temp[16];
  byte i;

  // compute the rounding factor and fractional multiplier
  double roundingFactor = 0.5;
  unsigned long mult = 1;
  for (i = 0; i < precision; i++)
  {
    roundingFactor /= 10.0;
    mult *= 10;
  }
  
  temp[0]='\0';
  outstr[0]='\0';

  if(val < 0.0){
    strcpy(outstr,"-\0");
    val = -val;
  }

  val += roundingFactor;

  strcat(outstr, itoa(int(val),temp,10));  //prints the int part
  if( precision > 0) {
    strcat(outstr, ".\0"); // print the decimal point
    unsigned long frac;
    unsigned long mult = 1;
    byte padding = precision -1;
    while(precision--)
      mult *=10;

    if(val >= 0)
      frac = (val - int(val)) * mult;
    else
      frac = (int(val)- val ) * mult;
    unsigned long frac1 = frac;

    while(frac1 /= 10)
      padding--;

    while(padding--)
      strcat(outstr,"0\0");

    strcat(outstr,itoa(frac,temp,10));
  }

  // generate space padding
  if ((widthp != 0)&&(widthp >= strlen(outstr))){
    byte J=0;
    J = widthp - strlen(outstr);
    
    for (i=0; i< J; i++) {
      temp[i] = ' ';
    }

    temp[i++] = '\0';
    strcat(temp,outstr);
    strcpy(outstr,temp);
  }
  
  return outstr;
} 


  
  
 
