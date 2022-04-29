// Please enter your WiFi sensitive data in the arduino_secrets.h file
#include "arduino_secrets.h"

#include <ArduinoOTA.h> // only for InternalStorage
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>

#define SKETCH_VERSION "1.0"
boolean isUpdateCheck = true;

const char MY_SSID[] = SECRET_SSID; // Loaded from arduino_secrets.h
const char MY_PASS[] = SECRET_PASS; // Loaded from arduino_secrets.h

WiFiClient    client;
int status = WL_IDLE_STATUS;

void handleSketchDownload() {

  const unsigned long CHECK_INTERVAL = 6000;  // Time interval between update checks (ms)

  // Time interval check
  static unsigned long previousMillis;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis < CHECK_INTERVAL)
    return;
  previousMillis = currentMillis;

  HttpClient httpClient(client, SERVER_HOST, SERVER_PORT);  // HTTP

  char buff[32];
  snprintf(buff, sizeof(buff), SERVER_PATH);

  Serial.print("Check for update file ");
  Serial.println(buff);

  // Make the GET request
  httpClient.beginRequest();
  httpClient.get(buff);
  httpClient.sendHeader("x-mkr1010-version", SKETCH_VERSION);
  httpClient.endRequest();

  int statusCode = httpClient.responseStatusCode();
  Serial.print("Update status code: ");
  Serial.println(statusCode);

  if (statusCode == 304) {
    httpClient.stop();
    Serial.println("Sketech is up to date!");

    // close update check
    isUpdateCheck = false;

    return;
  }

  if (statusCode != 200) {
    httpClient.stop();
    Serial.println("response header != 200");
    return;
  }

  long length = httpClient.contentLength();
  if (length == HttpClient::kNoContentLengthHeader) {
    httpClient.stop();
    Serial.println("Server didn't provide Content-length header. Can't continue with update.");
    return;
  }
  Serial.print("Server returned update file of size ");
  Serial.print(length);
  Serial.println(" bytes");

  if (!InternalStorage.open(length)) {
    httpClient.stop();
    Serial.println("There is not enough space to store the update. Can't continue with update.");
    return;
  }
  byte b;
  while (length > 0) {
    if (!httpClient.readBytes(&b, 1)) // reading a byte with timeout
      break;
    InternalStorage.write(b);
    length--;
  }
  InternalStorage.close();
  httpClient.stop();
  if (length > 0) {
    Serial.print("Timeout downloading update file at ");
    Serial.print(length);
    Serial.println(" bytes. Can't continue with update.");
    return;
  }

  Serial.println("Sketch update apply and reset.");
  Serial.flush();
  InternalStorage.apply(); // this doesn't return
}

void setup() {

  Serial.begin(9600);
  delay(200);

  Serial.print("Sketch version ");
  Serial.println(SKETCH_VERSION);

  Serial.println("Initialize WiFi");
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(MY_SSID);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(MY_SSID, MY_PASS);
  }
  Serial.println("WiFi connected");
}

void loop() {

  // check for updates
  if (isUpdateCheck) {
    handleSketchDownload();
  }

  Serial.print("Sketch version ");
  Serial.println(SKETCH_VERSION);
  delay(3000);

}
