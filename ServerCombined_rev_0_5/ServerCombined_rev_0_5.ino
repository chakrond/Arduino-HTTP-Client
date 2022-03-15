#include "arduino_secrets.h"

// Json
#include <ArduinoJson.h>

// Wifi
#include <SPI.h>
#include <WiFiNINA.h>
char ssid[] = SECRET_SSID;        //  network SSID (name)
char pass[] = SECRET_PASS;        //  network password b
WiFiClient client;
int nRecon = 0; // WiFi recconection count

// Username&Password Login for Host server
char Hostserver[] = HOST;    // ** Server Address
String email = USER;
String password = PASS;
String Gtoken; // store token

// HTTP request Timeout
unsigned long lastPostTime;            // last time you connected to the server, in milliseconds
unsigned long lastPATCHTime;
const unsigned long postingInterval = 15L * 1000L; // delay between updates, in milliseconds
const unsigned long pacthingInterval = 5L * 1000L; // delay between updates, in milliseconds
int nBadReq = 0; // Bad request  count

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

// Time
#include "RTClib.h"
RTC_DS1307 rtc;
// Set time Once**
// year, month, day, hr, min, sec
// rtc.adjust(DateTime(2022, 1, 19, 15, 4, 0));

//==============================================================================================================================================================
//  Initialization
//==============================================================================================================================================================

void setup() {

  //Initialize serial and wait for port to open
  Serial.begin(9600);

  // LCD
  lcd.begin(16, 2);

  // DHT
  dht.begin();

  // Time
  // Full Timestamp
  rtc.begin();
  if (! rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    lcd.clear();
    lcd.print(F("RTC not found"));
    lcd.setCursor(0, 1);
    lcd.print(F("Chk Batry/Wire"));
    while (1);
  }

  // Server
  chechWiFiModule();
  connect_WiFi();
  printWifiStatus();

  // Login to Hostserver
  Gtoken = LoginToServer(email, password);
}

//==============================================================================================================================================================
//  Main
//==============================================================================================================================================================

void loop() {

  //==============================================================================================================================================================
  //  LCD Display data from sensor DHT
  //==============================================================================================================================================================

  // Time
  // Full Timestamp
  DateTime now  = rtc.now();

  lcd.clear();
  lcd.print(F("Date: "));
  lcd.print(now.year(), DEC);
  lcd.print(F("-"));
  if (now.month() < 10) {
    lcd.print(F("0"));
  }
  lcd.print(now.month(), DEC);
  lcd.print(F("-"));
  if (now.day() < 10) {
    lcd.print(F("0"));
  }
  lcd.print(now.day(), DEC);

  lcd.setCursor(0, 1);
  lcd.print(F("Time: "));
  if (now.hour() < 10) {
    lcd.print(F("0"));
  }
  lcd.print(now.hour(), DEC);
  lcd.print(F(":"));
  if (now.minute() < 10) {
    lcd.print(F("0"));
  }
  lcd.print(now.minute(), DEC);
  lcd.print(F(":"));

  if (now.second() < 10) {
    lcd.print(F("0"));
  }
  lcd.print(now.second(), DEC);
  delay(4000);

  // DHT
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // set up the LCD's number of columns and rows:
  lcd.clear();
  lcd.print(F("Humid: "));
  lcd.print(h);
  lcd.print(F(" %"));
  lcd.setCursor(0, 1);
  lcd.print(F("Temp : "));
  lcd.print(t);
  lcd.print(F(" C"));

  //==============================================================================================================================================================
  //  Send data to server
  //==============================================================================================================================================================

  String recDate = String(now.year()) + '-' + String(now.month()) + '-' + String(now.day());
  String CurntTime = String(now.timestamp(DateTime::TIMESTAMP_FULL));

  if (millis() - lastPostTime > postingInterval) {

    postDataToServer(Gtoken, recDate, CurntTime, h, t);
  }

  delay(500);

  if (millis() - lastPATCHTime > pacthingInterval) {

    patchRealtimeData(Gtoken, CurntTime, h, t);
  }

  if (WiFi.status() != WL_CONNECTED) {
    // WiFi
    chechWiFiModule();
    connect_WiFi();
    printWifiStatus();
  }
}

//==============================================================================================================================================================
//  Functions
//==============================================================================================================================================================

void printWifiStatus() {
  // print the SSID of the network
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // print board's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print(F("signal strength (RSSI):"));
  Serial.print(rssi);
  Serial.println(F(" dBm"));

  Serial.print(F("To see this page in action, open a browser to http://"));
  Serial.println(ip);
  Serial.println();
}

void chechWiFiModule() {

  // check for the WiFi module
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println(F("Communication with WiFi module failed!"));
    // don't continue
    while (1);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println(F("Please upgrade the WiFi firmware"));
  }
}

void connect_WiFi() {

  WiFi.disconnect(); // clear connection to any wifi
  WiFi.setTimeout(15 * 1000);

  // attempt to connect to WiFi network
  while (WiFi.status() != WL_CONNECTED) {
    nRecon++;
    Serial.print(F("Attempting to connect to SSID: "));
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network
    WiFi.begin(ssid, pass);

    // wait 10 seconds for connection
    delay(10000);
  }
  Serial.println(F("Connected to WiFi"));

  if (nRecon > 5) {
    lcd.clear();
    lcd.print(F("WiFi Failed"));
    lcd.setCursor(0, 1);
    lcd.print(F("Board Reset..."));
    delay(4000);
    NVIC_SystemReset();
  }
}

String LoginToServer (String email, String pass) {

  // Timeout
  unsigned long limitedTime = 15L * 1000L;

  // JSON Object creatation
  const int capacity = JSON_OBJECT_SIZE(4);
  StaticJsonDocument<capacity> doc;
  doc["email"] = email;
  doc["password"] = pass;

  client.stop(); // clear the connection socket
  Serial.println(F("\nStarting connection to server..."));
  if (client.connect(Hostserver, 80)) {
    Serial.println(F("connected to server"));
    Serial.println(F("Login to server..."));

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

  if (!client.connected()) {
    Serial.println(F("\nConnection Failed"));
    Serial.println(F("\nReconnecting WiFi..."));
    // WiFi
    chechWiFiModule();
    connect_WiFi();
    printWifiStatus();

    // Login to Hostserver
    Gtoken = LoginToServer(email, password);
  }


  // Check HTTP status
  boolean HTTPcheck = false;
  unsigned long connectedTime = millis();
  char stat[32] = {}; // Initialise Array

  delay(5000); // delay for response **need
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
    Serial.println(F("\nRe-Login..."));
    delay(5000);
    Gtoken = LoginToServer(email, password);
  }

  if (HTTPcheck) {

    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (client.find(endOfHeaders)) {
      //      Serial.println(F("Skip to the end of Header..."));
    }

    // Get Token
    //  BUFSIZE = 1024;
    char clientline[300];
    int index = 0;
    char findToken[] = "\"token\":\"";
    if (client.find(findToken)) {
      //      Serial.println(F("Found token..."));

      while (client.available()) {
        char c = client.read();
        //        Serial.write(c);
        clientline[index] = c;
        index++;

        if (millis() - connectedTime > limitedTime) {
          Serial.println(F("\nClient timed out, disconnecting from server."));
          client.stop();
          break;
        }
      }

      // String getToken = String(clientline);
      int idx2 = String(clientline).indexOf("\"}");
      String Token = String(clientline).substring(0, idx2);

      Serial.print(F("\ntoken: "));
      Serial.println(Token);
      client.stop();
      return Token;

    } else {
      Serial.println(F("Login Failed"));
      client.stop();
      Serial.println(F("\nRe-Login..."));
      delay(5000);
      Gtoken = LoginToServer(email, password);
    }
  }
}


void postDataToServer(String token, String recDate, String recTime, float Humidity, float Temperature) {

  // JSON Object creatation POST method
  const int capacityPost = JSON_ARRAY_SIZE(2) + 4 * JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacityPost> docPost;
  // Add the array
  JsonArray dataArray = docPost.createNestedArray("dataArray");
  JsonObject data = dataArray.createNestedObject();
  docPost["recDate"] = recDate;
  data["recTime"] = recTime;
  data["Humidity"] = Humidity;
  data["Temperature"] = Temperature;

  client.stop(); // clear the connection socket
  Serial.println(F("\nStarting connection to server..."));
  if (client.connect(Hostserver, 80)) {

    Serial.println(F("connected to server"));
    Serial.println(F("POST data to server..."));

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
    client.println(measureJson(docPost));
    client.println("Content-Type: application/json");

    // **Terminate headers with a blank line**
    client.println();

    // **Send JSON document in body**
    serializeJson(docPost, client);
//    serializeJson(docPost, Serial);
    Serial.println(F("Data Posted!, store data on database"));
  }

  // Checking Head Response
  delay(5000); // delay for response **need
  Serial.println(F("Checking head response..."));

  // Check HTTP status
  char stat[32] = {}; // Initialise Array
  client.readBytesUntil('\r', stat, sizeof(stat));

  if (strcmp(stat + 9, "201 Created") == 0) {
    Serial.println(F("Response: Okay"));
    nBadReq = 0; // Reset bad request count
  }
  if (strcmp(stat + 9, "201 Created") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(stat);
    client.stop();
    nBadReq++;
    if (nBadReq > 5) {
      lcd.clear();
      lcd.print(F("Bad Request"));
      lcd.setCursor(0, 1);
      lcd.print(F("Board Reset..."));
      delay(4000);
      NVIC_SystemReset();
    }
  }

  if (!client.connected()) {
    Serial.println(F("\nConnection Failed"));
    Serial.println(F("\nReconnecting WiFi..."));
    // WiFi
    chechWiFiModule();
    connect_WiFi();
    printWifiStatus();

    //    // Login to Hostserver
    //    Gtoken = LoginToServer(email, password);
  }

  client.stop();

  // note the time that the connection was made
  lastPostTime = millis();

}

void patchRealtimeData(String token, String recTime, float Humidity, float Temperature) {

  // JSON Object creatation PATCH method
  const int capacityPatch = 3 * JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacityPatch> docPatch;
  docPatch["recTime"] = recTime;
  docPatch["Humidity"] = Humidity;
  docPatch["Temperature"] = Temperature;

  Serial.println(F("\nStarting connection to server..."));

  if (client.connect(Hostserver, 80)) {
    //    unsigned long connectedTime = millis();

    Serial.println(F("connected to server"));
    Serial.println(F("PATCH data to server..."));

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
    client.println(measureJson(docPatch));
    client.println("Content-Type: application/json");
    //    client.println("Accept: */*");
    //    client.println("Accept-Encoding: gzip, deflate, br");

    // **Terminate headers with a blank line**
    client.println();

    // **Send JSON document in body**
    serializeJson(docPatch, client);
    Serial.println(F("Data Patched!, update real-time data on database"));
  }

  // Checking Head Response
  delay(5000); // delay for response **need
  Serial.println(F("Checking head response..."));

  // Check HTTP status
  char stat[32] = {}; // Initialise Array
  client.readBytesUntil('\r', stat, sizeof(stat));

  if (strcmp(stat + 9, "201 Created") == 0) {
    Serial.println(F("Response: Okay"));
    nBadReq = 0; // Reset bad request count
  }
  if (strcmp(stat + 9, "201 Created") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(stat);
    client.stop();
    nBadReq++;
    if (nBadReq > 5) {
      lcd.clear();
      lcd.print(F("Bad Request"));
      lcd.setCursor(0, 1);
      lcd.print(F("Board Reset..."));
      delay(4000);
      NVIC_SystemReset();
    }
  }

  if (!client.connected()) {
    Serial.println(F("\nConnection Failed"));
    Serial.println(F("\nReconnecting WiFi..."));
    // WiFi
    chechWiFiModule();
    connect_WiFi();
    printWifiStatus();

    //    // Login to Hostserver
    //    Gtoken = LoginToServer(email, password);
  }

  client.stop();

  // note the time that the connection was made
  lastPATCHTime = millis();

}
