/*
  WiFi Web Server

  A simple web server that shows the value of the analog input pins.

  This example is written for a network using WPA encryption. For
  WEP or WPA, change the WiFi.begin() call accordingly.

  Circuit:
   Analog inputs attached to pins A0 through A5 (optional)

  created 13 July 2010
  by dlf (Metodo2 srl)
  modified 31 May 2012
  by Tom Igoe

*/

#include <SPI.h>
#include <WiFiNINA.h>


#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                 // your network key index number (needed only for WEP)

int status = WL_IDLE_STATUS;

WiFiServer server(80);
WiFiClient client;
String readString;
String strs[20];
String param[20];
String value[20];
int StringCount = 0;
String header;

// encoding
#include "base64.hpp"
unsigned char string[] = "user:pass";
unsigned char base64[21]; // 20 bytes for output + 1 for null terminator

// encode_base64() places a null terminator automatically, because the output is a string
unsigned int base64_length = encode_base64(string, strlen((char *) string), base64);

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();
}


void loop() {

  WiFiClient client = server.available(); // Listen for incoming clients
  if (client) {
    String currentLine = ""; 
    
    while (client.connected()) {
      
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        
        if (c == '\n') {

          if (currentLine.length() == 0) {

            //"Authorization: Basic ".length() = 21
            int index = header.indexOf("Authorization: Basic ") + 21;
            if ((index >= 21) && (header.substring(index, header.indexOf('\n', index) - 1) == (char *) base64)) { //"dXNlcjpwYXNz"

              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();
              client.println("<!DOCTYPE html>\n");
              client.println("<html>\n");
              client.println("<body>\n");
              client.println("<center>\n");
              client.println("<h1 style=\"color:blue;\">ESP32 webserver</h1>\n");
              client.println("<h2 style=\"color:green;\">Hello World Web Sever</h2>\n");
              client.println("<h2 style=\"color:blue;\">Password protected Web server</h2>\n");
              client.println("</center>\n");
              client.println("</body>\n");
              client.println("</html>");

              break;
            }
            // Wrong user or password, so HTTP request fails...
            else {
              client.println("HTTP/1.1 401 Unauthorized");
              client.println("WWW-Authenticate: Basic realm=\"Secure\"");
              client.println("Content-Type: text/html");
              client.println();
              client.println("<html>Authentication failed</html>");
              break;
            }
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') { // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
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
}
