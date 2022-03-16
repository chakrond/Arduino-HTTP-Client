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
  // Create a client connection
  client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        //read char by char HTTP request
        if (readString.length() < 100) {
          //store characters to string
          readString += c;
        }

        //if HTTP request has ended
        if (c == '\n') {
          Serial.println(readString); //print to serial monitor for debuging


          client.println("HTTP/1.1 200 OK"); //send new page
          client.println("Content-Type: text/html");
          client.println();
          client.println("<html><head><link rel=\"icon\" href=\"data:,\">");
          client.println("<title>Input Form</title>");
          client.println("</head><body>");
          client.println("<form action=\"/paramSettings\">");
          client.println("<label for=\"fname\">First Name</label>");
          client.println("<input type=\"text\" id=\"fname\" name=\"firstname\" placeholder=\"Your name..\">");
          client.println("<label for=\"lname\">Last Name</label>");
          client.println("<input type=\"text\" id=\"lname\" name=\"lastname\" placeholder=\"Your last name..\">");
          client.println("<label for=\"country\">Country</label>");
          client.println("<select id=\"country\" name=\"country\">");
          client.println("<option value=\"australia\">Australia</option>");
          client.println("<option value=\"canada\">Canada</option>");
          client.println("<option value=\"usa\">USA</option>");
          client.println("</select>");
          client.println("<input type=\"submit\" value=\"Submit\">");
          client.println("</form>");
          client.println("</body></html>");
          client.println("<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"http://arduino.cc/en/favicon.png\" />");



          //          client.println("<<html><head>");
          //          client.println("<title>Input Form</title>");
          //          client.println("</head><body>");
          //          client.println("<form action= \"/get\">");
          //          client.println("input1: <input type=\"text\" name=\"input1\">");
          //          client.println("<input type=\"submit\" value=\"Submit\">");
          //          client.println("</form><br>");
          //          client.println("</body> </html>");

          delay(1);
          //stopping client
          client.stop();


          //controls the Arduino if you press the buttons
          //          if (readString.indexOf("?firstname=") > 0) {
          //            Serial.print("IndexOf ?firstname: ");
          //            Serial.println(readString.indexOf("?firstname"));
          //          }


          readString = readString.substring(readString.indexOf('?') + 1);
          // Split the string into substrings
          while (readString.length() > 0) {
            int index = readString.indexOf('&');
            if (index == -1) {
              readString = readString.substring(0, readString.indexOf(' '));
              strs[StringCount++] = readString;
              readString = "";
              break;
            } else {
              strs[StringCount++] = readString.substring(0, index);
              readString = readString.substring(index + 1);
            }
          }

          // Show the resulting substrings
          for (int i = 0; i < StringCount; i++) {
            Serial.print(i);
            Serial.print(": \"");
            Serial.print(strs[i]);
            Serial.println("\"");
          }

          // Extract pair values
          for (int i = 0; i < StringCount; i++) {
            int index = strs[i].indexOf('=');
            if (index == -1) {
              param[i] = strs[i];
              break;
            } else {
              param[i] = strs[i].substring(0, index);
              value[i] = strs[i].substring(index + 1);
            }
          }


          // Show the resulting substrings
          for (int i = 0; i < StringCount; i++) {
            Serial.print(i);
            Serial.print(": \"");
            Serial.print("param = ");
            Serial.print(param[i]);
            Serial.print(", value = ");
            Serial.print(value[i]);
            Serial.println("\"");
          }


          //clearing string for next read
          readString = "";
          StringCount = 0;

        }
      }
    }
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
