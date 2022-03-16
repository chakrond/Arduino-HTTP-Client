
// encoding
#include "base64.hpp"

unsigned char string[] = "user:pass";
unsigned char base64[21]; // 20 bytes for output + 1 for null terminator

// encode_base64() places a null terminator automatically, because the output is a string
unsigned int base64_length = encode_base64(string, strlen((char *) string), base64);




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
String strs[20];
String param[20];
String value[20];
int StringCount = 0;
String header;

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
        //        Serial.write(c);
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

    // Extract query params
    int firstSpace = header.indexOf(' ');
    int secondSpace = header.indexOf(' ', firstSpace + 1);
    header = header.substring(header.indexOf('?') + 1, secondSpace);
    Serial.println("header: " + header);
    // Split the string into substrings
    while (header.length() > 0) {
      int index = header.indexOf('&');
      if (index == -1) {
        header = header.substring(0, header.indexOf(' '));
        strs[StringCount++] = header;
        header = "";
        break;
      } else {
        strs[StringCount++] = header.substring(0, index);
        header = header.substring(index + 1);
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

    // Clear the header variable
    header = "";
    StringCount = 0;
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

void SettingsPage() {


}
