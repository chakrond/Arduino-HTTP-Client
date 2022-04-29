#include "arduino_secrets.h"

#if ( defined(ARDUINO_SAM_DUE) || defined(__SAM3X8E__) )
// Default pin 10 to SS/CS
#define USE_THIS_SS_PIN       10
#define BOARD_TYPE      "SAM DUE"
#elif ( defined(CORE_TEENSY) )
#error You have to use examples written for Teensy
#endif

#ifndef BOARD_NAME
#define BOARD_NAME    BOARD_TYPE
#endif

#define _WEBSOCKETS_LOGLEVEL_     2
#define WEBSOCKETS_NETWORK_TYPE   NETWORK_WIFININA

//#include <WiFiNINA_Generic.h>

#include <ArduinoJson.h>

#include <WebSocketsClient_Generic.h>
#include <SocketIOclient_Generic.h>

SocketIOclient socketIO;

// -----------------------------------------------------------------------------------------------------
// WiFi Settings
// -----------------------------------------------------------------------------------------------------
char serverIP[] = WEBSOCKETS_SERVER_HOST;
uint16_t  serverPort = WEBSOCKETS_SERVER_PORT;

int status = WL_IDLE_STATUS;

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;         // your network password (use for WPA, or use as key for WEP), length must be 8+

const char* user_agent = "user-agent: mkr1010";
// -----------------------------------------------------------------------------------------------------
// Event Handler
// -----------------------------------------------------------------------------------------------------
String socket_ID;
String socket_username;
String socket_address;

// -----------------------------------------------------------------------------------------------------


void socketIOEvent(const socketIOmessageType_t& type, uint8_t * payload, const size_t& length) {

  // Extract Event
  DynamicJsonDocument doc2(1024);
  String eventName;


  switch (type)
  {
    case sIOtype_DISCONNECT:
      Serial.println("[IOc] Disconnected");

      break;

    case sIOtype_CONNECT:
      Serial.print("[IOc] Connected to url: ");
      Serial.println((char*) payload);

      // join default namespace (no auto join in Socket.IO V3)
      socketIO.send(sIOtype_CONNECT, "/");

      break;

    case sIOtype_EVENT:
      Serial.print("[IOc] Get event: ");
      Serial.println((char*) payload);

      // -----------------------------------------------------------------------------------------------------
      // Get Event name
      // -----------------------------------------------------------------------------------------------------

      // Deserialize the JSON document
      deserializeJson(doc2, payload);

      eventName = doc2[0].as<String>();
      //      Serial.print("__Debug eventName: "); Serial.println(eventName);


      // -----------------------------------------------------------------------------------------------------
      // On joined event
      // -----------------------------------------------------------------------------------------------------

      if ( eventName == "joined" ) {

        JsonObject Array_1;
        Array_1 = doc2[1];
        socket_ID = Array_1["id"].as<String>();
        socket_username = Array_1["username"].as<String>();
        socket_address = Array_1["address"].as<String>();

        Serial.print("socket_ID: "); Serial.println(socket_ID);
        Serial.print("socket_username: "); Serial.println(socket_username);
        Serial.print("socket_address: "); Serial.println(socket_address);
      }

      break;

    //    case sIOtype_ACK:
    //      Serial.print("[IOc] Get ack: ");
    //      Serial.println(length);
    //
    //      //hexdump(payload, length);
    //
    //      break;
    //
    case sIOtype_ERROR:
      Serial.print("[IOc] Get error: ");
      Serial.println(length);

      //hexdump(payload, length);

      break;
    //
    //    case sIOtype_BINARY_EVENT:
    //      Serial.print("[IOc] Get binary: ");
    //      Serial.println(length);
    //
    //      //hexdump(payload, length);
    //
    //      break;
    //
    //    case sIOtype_BINARY_ACK:
    //      Serial.print("[IOc] Get binary ack: ");
    //      Serial.println(length);
    //
    //      //hexdump(payload, length);
    //
    //      break;
    //
    case sIOtype_PING:
      Serial.println("[IOc] Get PING");

      break;

    case sIOtype_PONG:
      Serial.println("[IOc] Get PONG");

      break;

    default:
      break;
  }
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("WebSockets Client IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setup()
{
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  delay(200);

  Serial.print("\nStart WebSocketClientSocketIO_NINA on "); Serial.println(BOARD_NAME);
  Serial.println(WEBSOCKETS_GENERIC_VERSION);

  Serial.println("Used/default SPI pinout:");
  Serial.print("MOSI:");
  Serial.println(MOSI);
  Serial.print("MISO:");
  Serial.println(MISO);
  Serial.print("SCK:");
  Serial.println(SCK);
  Serial.print("SS:");
  Serial.println(SS);

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

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    //delay(10000);
  }

  printWifiStatus();

  // server address, port and URL
  Serial.print("Connecting to WebSockets Server @ IP address: ");
  Serial.print(serverIP);
  Serial.print(", port: ");
  Serial.println(serverPort);

  // setReconnectInterval to 10s, new from v2.5.1 to avoid flooding server. Default is 0.5s
  socketIO.setReconnectInterval(10000);

  socketIO.setExtraHeaders(user_agent);

  // server address, port and URL
  // void begin(IPAddress host, uint16_t port, String url = "/socket.io/?EIO=4", String protocol = "arduino");
  // To use default EIO=4 from v2.5.1
  socketIO.begin(serverIP, serverPort);

  // event handler
  socketIO.onEvent(socketIOEvent);
}


void loop() {

  socketIO.loop();

}
