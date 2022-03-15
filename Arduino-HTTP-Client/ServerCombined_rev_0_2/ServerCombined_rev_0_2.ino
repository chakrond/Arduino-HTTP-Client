//// Reset Function
//void(* resetFunc) (void) = 0;
//const int RESET_PIN = A1;

#include "arduino_secrets.h"

// Json
#include <ArduinoJson.h>

// Buffer size of incoming response
#define BUFSIZ 1024

// Wifi
#include <WiFiNINA.h>
char ssid[] = SECRET_SSID;             //  your network SSID (name) between the " "
char pass[] = SECRET_PASS;      // your network password between the " "
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;      //connection status
WiFiServer server(80);            //server socket
//WiFiClient client = .available();
WiFiClient client;

// Username&Password Login for Host server
char Hostserver[] = HOST;    // ** Server Address
String email = USER;
String password = PASS;
String Gtoken; // store token

// HTTP request Timeout
unsigned long lastPostTime = 0;            // last time you connected to the server, in milliseconds
unsigned long lastPATCHTime = 0;
//unsigned long lastCheckedTime = 0;
const unsigned long postingInterval = 15L * 1000L; // delay between updates, in milliseconds
const unsigned long pacthingInterval = 5L * 1000L; // delay between updates, in milliseconds
//const unsigned long checkingInterval = 20L * 60L * 1000L; // delay between updates, in milliseconds


// SD
//#include <SdFat.h>
//SdFat sd;
#include <SD.h>
#include <SPI.h>
#define SDCARD_CS 5
File dataFile;
File ConRec;
File root;
File sumfile;
//int fatpc1;
//int fatpc2;
float sumSize;
float sumSizeMB;
float sumSizeGB;
// SD Save Data Timeout
unsigned long lastSavedTime = 0;


// LCD
// include the library code:
#include <LiquidCrystal.h>
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 7, en = 6, d4 = 1, d5 = 2, d6 = 3, d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// DHT
#include "DHT.h"
#define DHTPIN 0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//Time
#include "RTClib.h"
RTC_DS1307 rtc;

// Relay Control
const int RELAY_PIN  = A0;  // the Arduino pin, which connects to the IN pin of relay
int       Ontemp     = 30;
int       OnHumd     = 50;
int       counter    =  0;
int       pircounter =  0;

//==============================================================================================================================================================
//  Initialization
//==============================================================================================================================================================

void setup() {

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  // LCD
  lcd.begin(16, 2);


  // Reset
  //  pinMode(RESET_PIN, OUTPUT);
  //  digitalWrite(RESET_PIN, HIGH);


  // SD Card
  // init SD card
  SPI.begin();
  SD.begin(SDCARD_CS);

  if (!SD.begin(SDCARD_CS)) {
    Serial.println("Failed to initialize SD card!");
    //    while (1);
  }

  sumfile = SD.open("/");
  sumallfile(sumfile);

  root = SD.open("/");
  printDirectory(root, 0);
  // Recursive list of all directories
  Serial.println(F("Files found in all dirs:"));
  printDirectory(root, 0);
  Serial.println();
  Serial.println(F("Done"));


  // DHT
  dht.begin();


  // Time
  // Full Timestamp
  rtc.begin();
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    lcd.clear();
    lcd.print(F("RTC not found"));
    lcd.setCursor(0, 1);
    lcd.print(F("Chk Batry/Wire"));
    Serial.flush();
    abort();
  }

  // Relay- Setup
  // initialize digital pin as an output.
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Relay working on LOW

  // Server
  chechWiFiModule();
  connect_WiFi();
  Serial.println("Connected to WiFi");
  printWifiStatus();
  server.begin();

  // Login to Hostserver
  Gtoken = LoginToServer(email, password);
}

//==============================================================================================================================================================
//  Main
//==============================================================================================================================================================

void loop() {

  // SD Free space check
  while ( sumSizeGB > float(55) ) {
    lcd.clear();
    lcd.print("SD Memory Full");
    lcd.setCursor(0, 1);
    lcd.print("Pls Format");
    delay(4000);
  }


  //==============================================================================================================================================================
  //  LCD Display data from sensor DHT
  //==============================================================================================================================================================

  // Time
  // Full Timestamp
  DateTime time = rtc.now();
  DateTime now  = rtc.now();
  String CurntTime = String(time.timestamp(DateTime::TIMESTAMP_FULL));
  //  Serial.println(CurntTime);
  lcd.clear();
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
  delay(4000);

  // DHT
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

  //  lcd.begin(16, 2);
  lcd.print(F("Humid: "));
  lcd.print(h);
  lcd.print(F(" %"));
  lcd.setCursor(0, 1);
  lcd.print(("Temp : "));
  lcd.print(t);
  lcd.print(F(" C"));
  delay(4000);


  //==============================================================================================================================================================
  //  Save record to SD card
  //==============================================================================================================================================================

  // Filename

  int nyear  = now.year();
  int nmonth = now.month();
  int nday   = now.day();

  char fileName[13];
  sprintf(fileName, "%4d%02d%02d.csv", nyear, nmonth, nday);

  // init the CSV file with headers
  if (!SD.exists(fileName)) {
    dataFile = SD.open(fileName, FILE_WRITE);
    dataFile.println("timestamp,humidity,temperature");
    dataFile.close();
  }

  if (millis() - lastSavedTime > postingInterval) {
    // Save data
    // if the file opened okay, write to it:
    dataFile = SD.open(fileName, FILE_WRITE);
    if (dataFile) {
      lastSavedTime = millis();
      dataFile.print(CurntTime); // timestamp
      dataFile.print(",");
      dataFile.print(h); // humidity
      dataFile.print(",");
      dataFile.println(t); // temperature

      // close the file
      delay(500);
      dataFile.close();
      Serial.println();
      Serial.println("Saving: " + String(fileName) + " Successfully");
      //      fatpc1 = 0; // attempt counter

    } else {
      // if the file didn't open, print an error:
      Serial.println("\nerror opening data file: " + String(fileName));

      // Print error to LCD
      lcd.clear();
     
      lcd.print(F("SD Card Failed"));
      lcd.setCursor(0, 1);
      lcd.print(F("Save Failed"));
      delay(5000);



      //      fatpc1++;
      //      Serial.println("error opening data file, attempt: " + String(fatpc1));
      //      // Reset board
      //      if (fatpc1 > 10) {
      //        lcd.clear();
      //        lcd.print(F("Failed to open"));
      //        lcd.setCursor(0, 1);
      //        lcd.print(F("Board Reset"));
      //        for (int i = 0; i < 12; i++) {
      //          lcd.print(".");
      //          delay(500);
      //        }
      //        Serial.println("Board Reset");
      //        //      digitalWrite(RESET_PIN, HIGH);
      //        //      resetFunc();
      //        NVIC_SystemReset();
      //      }
    }
  }

  //==============================================================================================================================================================
  //  Send data to server
  //==============================================================================================================================================================
  //  nyear  = now.year();
  //  nmonth = now.month();
  //  nday   = now.day();

  String recDate = String(nyear) + "-" + String(nmonth) + "-" + String(nday);

  if (millis() - lastPostTime > postingInterval) {

    postDataToServer(Gtoken, recDate, CurntTime, h, t);
    //    Serial.println("Client stopped!");
    //    //    delay(500);  // delay for server
    //    client.stop();
  }

  if (millis() - lastPATCHTime > pacthingInterval) {

    patchRealtimeData(Gtoken, CurntTime, h, t);
    //    Serial.println("Client stopped!");
    //    //    delay(500);  // delay for server
    //    client.stop();
  }


  //==============================================================================================================================================================
  //  Web Server
  //==============================================================================================================================================================

  client = server.available();

  char clientline[BUFSIZ];
  char name[17];
  int index = 0;

  if (client) {

    // an http request ends with a blank line
    boolean current_line_is_blank = true;

    // reset the input buffer
    index = 0;

    while (client.connected()) {
      if (client.available()) {

        char c = client.read();

        // If it isn't a new line, add the character to the buffer
        if (c != '\n' && c != '\r') {
          clientline[index] = c;
          index++;
          // are we too big for the buffer? start tossing out data
          if (index >= BUFSIZ)
            index = BUFSIZ - 1;

          // continue to read more data!
          continue;
        }

        // got a \n or \r new line, which means the string is done
        clientline[index] = 0;

        // Print it out for debugging
        //        Serial.print("Tag: void loop(), clientline: "); Serial.println(clientline);

        //        Serial.println("Tag: void loop(), last_Print clientline: "); Serial.println(clientline);

        // Look for substring such as a request to get the file
        if (strstr(clientline, "GET /RD") != 0) {
          char *getname;
          getname = clientline + 5; // look after the "GET /**" (5 chars)  *******
          // a little trick, look for the " HTTP/1.1" string and
          // turn the first character of the substring into a 0 to clear it out.
          (strstr(clientline, " HTTP"))[0] = 0;
          Serial.print("Tag: RD, clientline after clear: "); Serial.println(clientline);

          if (strcmp(getname, "RD") == 0) {
            Serial.print("Tag: RD, Statement: True, "); Serial.println(strcmp(clientline + 5, "RD"));
            printWEB();
          }
        }

        // Print it out for debugging
        //        Serial.println("Tag: download_page, clientline: "); Serial.println(clientline);

        // Look for substring such as a request to get the file
        if (strstr(clientline, "GET /DL/") != 0) {
          // this time no space after the /, so a sub-file!
          char *filename;

          filename = clientline + 8; // look after the "GET /DL/" (8 chars)  *******
          //          Serial.println("Tag: DL, at filename: ");
          Serial.println(clientline);
          // a little trick, look for the " HTTP/1.1" string and
          // turn the first character of the substring into a 0 to clear it out.
          (strstr(clientline, " HTTP"))[0] = 0;
          Serial.print("Tag: DL, clientline after clear: "); Serial.println(clientline);

          if (filename[strlen(filename) - 1] == '/') { // Trim a directory filename
            filename[strlen(filename) - 1] = 0;      //  as Open throws error with trailing /
          }

          Serial.print(F("Web request for: ")); Serial.println(filename);  // print the file we want

          File file = SD.open(filename, O_READ);
          if ( file == 0 ) {  // Opening the file with return code of 0 is an error in SDFile.open
            client.println("HTTP/1.1 404 Not Found");
            client.println("Content-Type: text/html");
            client.println();
            client.println("<h2>File Not Found!</h2>");
            client.println("<br><h3>Couldn't open the File!</h3>");
            break;
          }

          Serial.println("Tag: DL, File Opened!");

          client.println("HTTP/1.1 200 OK");
          if (file.isDirectory()) {
            Serial.println("Tag: DL, is a directory");
            //file.close();
            client.println("Content-Type: text/html");
            client.println();
            client.print("<h2>Files in /");
            client.print(filename);
            client.println(":</h2>");
            ListFiles(client, LS_SIZE, file);
            file.close();

          } else { // Any non-directory clicked, server will send file to client for download
            Serial.println("Tag: DL, send file to client for download");
            client.println("Content-Type: application/octet-stream");
            client.println();

            char file_buffer[16];
            int avail;
            while (avail = file.available()) {
              int to_read = min(avail, 16);
              if (to_read != file.read(file_buffer, to_read)) {
                break;
              }
              // uncomment the serial to debug (slow!)
              //Serial.write((char)c);
              client.write(file_buffer, to_read);
            }
            file.close();
          }
        } else {
          Error_page();
        }
        break;
      }
    }
  }
  delay(1);
  client.stop();
}

//==============================================================================================================================================================
//  Functions
//==============================================================================================================================================================

void printWEB() {

  // Time RTC
  DateTime time = rtc.now();
  DateTime now  = rtc.now();
  String CurntTime = String(time.timestamp(DateTime::TIMESTAMP_FULL));


  // DHT11 Sensor
  delay(1000);
  float humid = dht.readHumidity();
  float temp  = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if ( isnan(humid) || isnan(temp) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  char clientline[BUFSIZ];
  int index = 0;

  if (client) {

    // Print realtime data
    Serial.println("Sensors Data!");

    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");

    client.print("<p>Timestamp,Humidity(%),Temperature(C)</p>");
    client.print("<p>");
    client.print(CurntTime);
    client.print(",");
    client.print(humid);
    client.print(",");
    client.print(temp);
    client.print("</p>");
  }
  // give the web browser time to receive the data
  delay(1);
  client.stop();
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  Serial.println();
}

void chechWiFiModule() {

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the WiFi firmware");
  }

}

void connect_WiFi() {
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
}

void printDirectory(File dir, int numTabs) {
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

void ListFiles(WiFiClient client, uint8_t flags, File dir) {
  client.println("<ul>");
  while (true) {
    File entry = dir.openNextFile();

    // done if past last used entry
    if (! entry) {
      // no more files
      break;
    }

    // print any indent spaces
    client.print("<li><a href=\"");
    client.print(entry.name());
    if (entry.isDirectory()) {
      client.println("/");
    }
    client.print("\">");

    // print file name with possible blank fill
    client.print(entry.name());
    if (entry.isDirectory()) {
      client.println("/");
    }

    client.print("</a>");
    /*
        // print modify date/time if requested
        if (flags & LS_DATE) {
           dir.printFatDate(p.lastWriteDate);
           client.print(' ');
           dir.printFatTime(p.lastWriteTime);
        }
        // print size if requested
        if (!DIR_IS_SUBDIR(&p) && (flags & LS_SIZE)) {
          client.print(' ');
          client.print(p.fileSize);
        }
    */
    client.println("</li>");
    entry.close();
  }
  client.println("</ul>");
}

void Error_page() { // Directory Not Found

  // everything else is a 404
  client.println("HTTP/1.1 404 Not Found");
  client.println("Content-Type: text/html");
  client.println();
  client.println("<h2>Page Not Found!</h2>");
  delay(1);
  client.stop();

}

void sumallfile(File dir) {
  sumSize = 0;
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    sumSize    = sumSize + float(entry.size() );
    sumSizeMB  = sumSize / pow(1024, 2);
    sumSizeGB  =  sumSize / pow(1024, 3);
    entry.close();
  }
  Serial.println("SD Occupied Size: " + String(sumSizeMB) + " MB");
  Serial.println("SD Occupied Size: " + String(sumSizeGB) + " GB");

}

String LoginToServer (String email, String pass) {

  // Timeout
  unsigned long limitedTime = 15L * 1000L;

  // JSON Object creatation
  const int capacity = JSON_OBJECT_SIZE(4);
  StaticJsonDocument<capacity> doc;
  doc["email"] = email;
  doc["password"] = pass;
  //  Serial.println("Json Length: " + String(measureJson(doc)) );

  Serial.println("\nStarting connection to server...");

  if (client.connect(Hostserver, 80)) {
    Serial.println("connected to server");

    // HTTP Method
    client.println("POST /users/login HTTP/1.1");

    // HTTP headers
    client.print("Host: ");
    client.println(Hostserver);
    client.println("User-Agent: Arduino/mrkwifi");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(measureJson(doc));
    client.println("Content-Type: application/json");

    // **Terminate headers with a blank line**
    client.println();

    // **Send JSON document in body**
    serializeJson(doc, client);

  }

  char clientline[BUFSIZ];
  int index = 0;
  boolean HTTPcheck = false;

  unsigned long connectedTime = millis();
  // Check HTTP status
  char stat[32] = {0};
  client.readBytesUntil('\r', stat, sizeof(stat));
  // It should be "HTTP/1.0 200 OK" or "HTTP/1.1 200 OK"
  if (strcmp(stat + 9, "200 OK") == 0) {
    Serial.println(F("Response: Okay"));
    HTTPcheck = true;
  }
  if (strcmp(stat + 9, "200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(stat);
    Serial.println(F("Login failed"));
    client.stop();
    Serial.println();
    Serial.println(F("Re-Login..."));
    delay(5000);
    Gtoken = LoginToServer(email, password);
  }

  if (HTTPcheck) {

    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (client.find(endOfHeaders)) {
      Serial.println(F("Skip to the end of Header..."));
    }

    // Get Token
    char clientline[BUFSIZ];
    int index = 0;
    char findToken[] = "\"token\":\"";
    if (client.find(findToken)) {
      Serial.println(F("Found token..."));

      while (client.available()) {
        char c = client.read();
        //        Serial.write(c);
        clientline[index] = c;
        index++;
      }

      //    String getToken = String(clientline);
      int idx2 = String(clientline).indexOf("\"}");
      String Token = String(clientline).substring(0, idx2);

      if (millis() - connectedTime > limitedTime) {
        Serial.println();
        Serial.println("Client timed out, disconnecting from server.");
        client.stop();
      }

      Serial.println();
      Serial.println("token: " + Token);
      client.stop();
      return Token;

    } else {
      Serial.println("Login Failed");
      client.stop();
    }
  }
}


void postDataToServer(String token, String recDate, String recTime, float Humidity, float Temperature) {

  // Timeout
  boolean isTimeout = false;
  unsigned long limitedTime = 15L * 1000L;


  // JSON Object creatation
  const int capacity = JSON_ARRAY_SIZE(2) + 4 * JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacity> doc;
  doc["recDate"] = recDate;

  // Add the array
  JsonArray dataArray = doc.createNestedArray("dataArray");
  JsonObject data = dataArray.createNestedObject();
  data["recTime"] = recTime;
  data["Humidity"] = Humidity;
  data["Temperature"] = Temperature;

  Serial.println("\nStarting connection to server...");

  if (client.connect(Hostserver, 80)) {
    unsigned long connectedTime = millis();
    Serial.println("connected to server");
    Serial.println("POST data to server...");

    // HTTP Method
    client.println("POST /data HTTP/1.1");

    // HTTP headers
    client.print("Host: ");
    client.println(Hostserver);
    client.print("Authorization: Bearer ");
    client.println(token);
    client.println("User-Agent: Arduino/mrkwifi");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(measureJson(doc));
    client.println("Content-Type: application/json");
    //    client.println("Accept: */*");
    //    client.println("Accept-Encoding: gzip, deflate, br");

    // **Terminate headers with a blank line**
    client.println();

    // **Send JSON document in body**
    serializeJson(doc, client);
    Serial.println("Data Posted!, store data on database");

    if (millis() - connectedTime > limitedTime) {
      Serial.println();
      Serial.println("Client timed out!, disconnecting from server.");
      client.stop();
      isTimeout = true;
    }
  }

  // Checking Head Response
  if (millis() < (45L * 1000L)) {
    delay(5000);
    Serial.println("1st times - Checking head response...");

    // Check HTTP status
    char stat[32] = {0};
    client.readBytesUntil('\r', stat, sizeof(stat));

    if (strcmp(stat + 9, "201 Created") == 0) {
      Serial.println(F("Response: Okay"));
    }
    if (strcmp(stat + 9, "201 Created") != 0) {
      Serial.print(F("Unexpected response: "));
      Serial.println(stat);
      client.stop();
    }
  }

  client.stop();

  if (!isTimeout) {
    // note the time that the connection was made:
    lastPostTime = millis();
  }
}

void patchRealtimeData(String token, String recTime, float Humidity, float Temperature) {

  // Timeout
  boolean isTimeout = false;
  unsigned long limitedTime = 15L * 1000L;


  // JSON Object creatation
  const int capacity = 3 * JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacity> doc;
  doc["recTime"] = recTime;
  doc["Humidity"] = Humidity;
  doc["Temperature"] = Temperature;

  Serial.println("\nStarting connection to server...");

  if (client.connect(Hostserver, 80)) {
    unsigned long connectedTime = millis();

    Serial.println("connected to server");
    Serial.println("PATCH data to server...");

    // HTTP Method
    client.println("PATCH /data/real HTTP/1.1");

    // HTTP headers
    client.print("Host: ");
    client.println(Hostserver);
    client.print("Authorization: Bearer ");
    client.println(token);
    client.println("User-Agent: Arduino/mrkwifi");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(measureJson(doc));
    client.println("Content-Type: application/json");
    //    client.println("Accept: */*");
    //    client.println("Accept-Encoding: gzip, deflate, br");

    // **Terminate headers with a blank line**
    client.println();

    // **Send JSON document in body**
    serializeJson(doc, client);
    Serial.println("Data Patched!, update real-time data on database");

    if (millis() - connectedTime > limitedTime) {
      Serial.println();
      Serial.println("Client Timed Out, disconnecting from server.");
      client.stop();
      isTimeout = true;
    }
  }

  // Checking Head Response
  if (millis() < (60L * 1000L)) {
    delay(5000);
    Serial.println("1st times - Checking head response...");

    // Check HTTP status
    char stat[32] = {0};
    client.readBytesUntil('\r', stat, sizeof(stat));

    if (strcmp(stat + 9, "201 Created") == 0) {
      Serial.println(F("Response: Okay"));
    }
    if (strcmp(stat + 9, "201 Created") != 0) {
      Serial.print(F("Unexpected response: "));
      Serial.println(stat);
      client.stop();
    }
  }

  client.stop();

  if (!isTimeout) {
    // note the time that the connection was made:
    lastPATCHTime = millis();
  }
}
