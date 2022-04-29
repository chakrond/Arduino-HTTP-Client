#include "arduino_secrets.h"

//#if !defined(ESP8266)
//#error This code is intended to run only on the ESP8266 boards ! Please check your Tools->Board setting.
//#endif
//
//#define DEBUG_WEBSOCKETS_PORT  Serial
//#define _WEBSOCKETS_LOGLEVEL_  4

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ArduinoJson.h>

#include <WebSocketsClient_Generic.h>
#include <SocketIOclient_Generic.h>

ESP8266WiFiMulti WiFiMulti;
SocketIOclient socketIO;

// -----------------------------------------------------------------------------------------------------
// WiFi Settings
// -----------------------------------------------------------------------------------------------------
char serverIP[] = WEBSOCKETS_SERVER_HOST;
uint16_t  serverPort = WEBSOCKETS_SERVER_PORT;

int status = WL_IDLE_STATUS;

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;         // your network password (use for WPA, or use as key for WEP), length must be 8+

// -----------------------------------------------------------------------------------------------------
// Device name
// -----------------------------------------------------------------------------------------------------
const char* user_agent = "user-agent: esp8266-dsTemp-01";

// -----------------------------------------------------------------------------------------------------
// Event Handler
// -----------------------------------------------------------------------------------------------------
String socket_ID;
String socket_username;
String socket_address;

// -----------------------------------------------------------------------------------------------------
// Event Handler
// -----------------------------------------------------------------------------------------------------
// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 0

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

float tempC;
const unsigned long readTimeout_dsTemp = 5L * 1000L;
unsigned long lastReadTime_dsTemp;
const unsigned long intervalRead_dsTemp = 15L * 1000L;

// -----------------------------------------------------------------------------------------------------

void socketIOEvent(const socketIOmessageType_t& type, uint8_t * payload, const size_t& length) {

  // Extract Event
  DynamicJsonDocument doc2(1024);
  String eventName;


  switch (type) {

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

        //        Serial.print("socket_ID: "); Serial.println(socket_ID);
        //        Serial.print("socket_username: "); Serial.println(socket_username);
        //        Serial.print("socket_address: "); Serial.println(socket_address);
      }

      // -----------------------------------------------------------------------------------------------------

      break;

    //    case sIOtype_ACK:
    //      Serial.print("[IOc] Get ack: ");
    //      Serial.println(length);
    //
    //      //hexdump(payload, length);
    //
    //      break;
    //
    //    case sIOtype_ERROR:
    //      Serial.print("[IOc] Get error: ");
    //      Serial.println(length);
    //
    //      //hexdump(payload, length);
    //
    //      break;
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
    //    case sIOtype_PING:
    //      Serial.println("[IOc] Get PING");
    //
    //
    //      break;
    //
    //    case sIOtype_PONG:
    //      Serial.println("[IOc] Get PONG");
    //
    //
    //      break;

    default:
      break;
  }
}

void printWifiStatus() {
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

void setup() {

  Serial.begin(115200);
  delay(200);

  //  Serial1.begin(9600);
  //  delay(200);

  Serial.print("\nStart WebSocketClientSocketIO on "); Serial.println(ARDUINO_BOARD);
  Serial.println(WEBSOCKETS_GENERIC_VERSION);

  WiFiMulti.addAP(ssid, pass);

  Serial.print("Connecting to "); Serial.println(ssid);

  //WiFi.disconnect();
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();

  // Client address
  Serial.print("WebSockets Client started @ IP address: ");
  Serial.println(WiFi.localIP());

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


  // dsTemp

  // Start up the library
  Serial.println("Init dsTemp Sensor...");
  sensors.begin();
}


void loop() {

  socketIO.loop();


  if ( millis() - lastReadTime_dsTemp > intervalRead_dsTemp ) {

    // dsTemp
    unsigned long readTime_dsTemp = millis();

    do {

      lastReadTime_dsTemp = millis();
      
      sensors.requestTemperatures();
      tempC = sensors.getTempCByIndex(0);

      delay(100);

    } while (tempC == DEVICE_DISCONNECTED_C && millis() - readTime_dsTemp < readTimeout_dsTemp);

    // creat JSON message for Socket.IO (event)
    DynamicJsonDocument doc3(1024);
    JsonArray array = doc3.to<JsonArray>();

    // add evnet name
    array.add("dsTemp");

    // add payload (parameters) for the event
    JsonObject param1 = array.createNestedObject();
    param1["id"]      = socket_ID;
    param1["Temp"]    = tempC;

    // JSON to String (serializion)
    String output;
    serializeJson(doc3, output);

    // Send event
    socketIO.sendEVENT(output);

    // Print JSON for debugging
    Serial.println(output);
  }

}
