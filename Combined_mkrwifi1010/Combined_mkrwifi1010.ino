// LCD
// include the library code:
#include <LiquidCrystal.h>
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 7, en = 6, d4 = 1, d5 = 2, d6 = 3, d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// PIR
int pirPin  = 13;                 // PIR Out pin
int pirStat = 0;                   // PIR status

// DHT
#include "DHT.h"
#define DHTPIN 0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// SD
// include the SD library:
#include <SPI.h>
#include <SD.h>
const int chipSelect = 5;
File dataFile;
File Logfile;
File ConRec;
// SD Card Info
//SdFile root;
//Sd2Card card;
//SdVolume volume;


//Time
#include "RTClib.h"
RTC_DS1307 rtc;


// Relay Control
const int RELAY_PIN = A0;  // the Arduino pin, which connects to the IN pin of relay
const int Ontemp    = 30;
const int OnHumd    = 50;
int       counter   = 0;
int       pircounter = 0;

void setup() {

  // LCD - Setup
  // the LCD's number of columns and rows:
  // lcd.begin(16, 2);

  // PIR Setup
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pirPin, INPUT);


  // SD - Setup
  //  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  //  root.openRoot(volume);
  //  // list all files in the card with date and size
  //  root.ls(LS_R | LS_DATE | LS_SIZE);


  // Relay- Setup
  // initialize digital pin as an output.
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Relay working on LOW


  Serial.begin(9600);
}

void loop() {

  // Counter
  counter = counter + 1;

  // PIR
  pirStat = digitalRead(pirPin);
  //    while (pirStat == HIGH) {            // if motion detected
  //      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  //      Serial.println("Motion Detected");
  //      // Print a message to the LCD.
  //      lcd.clear();
  //      lcd.begin(16, 2);
  //      lcd.print("Motion Detected!");
  //      pirStat = digitalRead(pirPin);
  //      delay(1000);
  //      if ((pirStat == LOW)) {
  //      digitalWrite(LED_BUILTIN, LOW); // turn LED OFF if we have no motion
  //    break;
  //      }
  //      }
  pirStat = digitalRead(pirPin);
  if (pirStat == HIGH) {            // if motion detected
    counter     = 0;
    pircounter  = 0;
    Serial.println("Motion Detected");
    // Print a message to the LCD.
    lcd.display();
    lcd.clear();
    lcd.begin(16, 2);
    lcd.print("Motion Detected!");
    delay(1000);
  }

  if (pircounter>25){
    lcd.noDisplay();
    Serial.println("Turn off Display");
  }
  

  // Time
  // Full Timestamp
  rtc.begin();
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  DateTime time = rtc.now();
  DateTime now  = rtc.now();
  String CurntTime = String(time.timestamp(DateTime::TIMESTAMP_FULL));
  //  Serial.println(CurntTime);
  lcd.begin(16, 2);
  lcd.print("Date: ");
  lcd.print(now.year(), DEC);
  lcd.print('-');
  lcd.print(now.month(), DEC);
  lcd.print('-');
  lcd.print(now.day(), DEC);

  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  if (now.hour() < 10) {
    lcd.print("0");
  }
  lcd.print(now.hour(), DEC);
  lcd.print(':');
  if (now.minute() < 10) {
    lcd.print("0");
  }
  lcd.print(now.minute(), DEC);
  lcd.print(':');

  if (now.second() < 10) {
    lcd.print("0");
  }
  lcd.print(now.second(), DEC);
  delay(2000);


  // DHT
  dht.begin();

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // set up the LCD's number of columns and rows:
  lcd.clear();

  lcd.begin(16, 2);
  lcd.print(F("Humid: "));
  lcd.print(h);
  lcd.print(F(" %"));
  lcd.setCursor(0, 1);
  lcd.print(("Temp : "));
  lcd.print(t);
  lcd.print(F(" C"));
  delay(2000);

  // SD Writing Data

  SPI.begin();
  SD.begin(chipSelect);

  // init SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("Failed to initialize SD card!");
    lcd.clear(); lcd.print("Failed to initialize SD card!");
    while (1);
  }

  // Filename

  int nyear  = now.year();
  int nmonth = now.month();
  int nday   = now.day();

  char fileName[13];
  sprintf(fileName, "%4d%02d%02d.csv", nyear, nmonth, nday);
  Serial.println("Saving: " + String(fileName) + " Successfully");


  // init the CSV file with headers
  if (!SD.exists(fileName)) {
    dataFile = SD.open(fileName, FILE_WRITE);
    dataFile.println("timestamp,humidity,temperature");
    dataFile.close();
  }

  // Save data
  // if the file opened okay, write to it:
  dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) {
    dataFile.print(CurntTime); // timestamp
    dataFile.print(",");
    dataFile.print(h); // humidity
    dataFile.print(",");
    dataFile.println(t); // temperature

    // close the file
    dataFile.close();

  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening data file");
  }


  // Continuous Record
  String fname2 = "ConRec.csv";
  int str_len2  = fname2.length() + 1;
  char char_array2[str_len2];
  fname2.toCharArray(char_array2, str_len2);
  

  // init the CSV file with headers
  if (!SD.exists(fname2)) {
    ConRec = SD.open(fname2, FILE_WRITE);
    ConRec.println("timestamp,humidity,temperature");
    ConRec.close();
  }

  // Save data
  // if the file opened okay, write to it:
  ConRec = SD.open(fname2, FILE_WRITE);
  if (ConRec) {
    ConRec.print(CurntTime); // timestamp
    ConRec.print(",");
    ConRec.print(h); // humidity
    ConRec.print(",");
    ConRec.println(t); // temperature

    // close the file
    ConRec.close();

  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening data file");
  }

  // Relay Control
  String fname1 = "Rlog.csv";
  int str_len = fname1.length() + 1;
  char char_array[str_len];
  fname1.toCharArray(char_array, str_len);

  if (pirStat == LOW) {
    pircounter = pircounter + 1;

    // Save data
    // if the file opened okay, write to it:
    Logfile = SD.open(fname1, FILE_WRITE);
    if (Logfile && counter == 0) {
      Logfile.print(CurntTime); // timestamp
      Logfile.print(",");
      Logfile.println("OFF");
      Logfile.close();
      Serial.println("Saving: " + String(fname1) + " Successfully" );
    }

  }

  if (now.hour() > 18 || h > 75) {
    digitalWrite(RELAY_PIN, HIGH);
    counter     = 0;
    pircounter  = 0;

    // Save data
    // if the file opened okay, write to it:
    Logfile = SD.open(fname1, FILE_WRITE);
    if (Logfile && counter == 0) {
      Logfile.print(CurntTime); // timestamp
      Logfile.print(",");
      Logfile.println("OFF");
      Logfile.close();
      Serial.println("Saving: " + String(fname1) + " Successfully" );
    }

  }

  if (counter > 200 && pircounter > 200) {
    digitalWrite(RELAY_PIN, HIGH);
    counter     = 0;
    pircounter  = 0;

    // Save data
    // if the file opened okay, write to it:
    Logfile = SD.open(fname1, FILE_WRITE);
    if (Logfile && counter == 0) {
      Logfile.print(CurntTime); // timestamp
      Logfile.print(",");
      Logfile.println("OFF");
      Logfile.close();
      Serial.println("Saving: " + String(fname1) + " Successfully" );
    }

  }


  if ( h > OnHumd && t > Ontemp && pirStat == HIGH && now.hour() >= 10 && now.hour() <= 18 ) {
    digitalWrite(RELAY_PIN, LOW); 

    // init the CSV file with headers
    if (!SD.exists(fname1)) {
      Logfile = SD.open(fname1, FILE_WRITE);
      Logfile.println("timestamp,stat");
      Logfile.close();
    }

    // Save data
    // if the file opened okay, write to it:
    Logfile = SD.open(fname1, FILE_WRITE);
    if (Logfile && counter == 1) {
      Logfile.print(CurntTime); // timestamp
      Logfile.print(",");
      Logfile.println("ON");
      Logfile.close();
      Serial.println("Saving: " + String(fname1) + " Successfully" );
    }

  }
  
}

void printData() {

  
}
