/****************************************************************************************************************************
  ESP8266_WebSocketClient.ino
  For ESP8266

  Based on and modified from WebSockets libarary https://github.com/Links2004/arduinoWebSockets
  to support other boards such as  SAMD21, SAMD51, Adafruit's nRF52 boards, etc.

  Built by Khoi Hoang https://github.com/khoih-prog/WebSockets_Generic
  Licensed under MIT license

  Originally Created on: 24.05.2015
  Original Author: Markus Sattler
*****************************************************************************************************************************/

#if !defined(ESP8266)
  #error This code is intended to run only on the ESP8266 boards ! Please check your Tools->Board setting.
#endif

#define _WEBSOCKETS_LOGLEVEL_     2

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>

#include <WebSocketsClient_Generic.h>

#include <Hash.h>
#include <Servo.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

#define USE_SSL         true
#define SERVO_PIN 13 //舵机数据信号引脚（D7)
#define SERVO_Px_PIN 2 //舵机数据信号引脚（D3) phoneX
#define SERVO_Py_PIN 0 //舵机数据信号引脚（D4) phoneY
#define SERVO_PIN 13 //舵机数据信号引脚（D7)
#define MOTOR_A_PIN_1 5 //电机1 (D1)
#define MOTOR_A_PIN_2 4//电机1 (D2)
#define MOTOR_A_SPEED_PIN 14//电机1 pwm速度引脚(D5)
#

Servo myServo;  // 定义Servo对象来控制
Servo ServoPX;  // 定义Servo对象来控制phoneX
Servo ServoPY;  // 定义Servo对象来控制phoney

#if USE_SSL
  // Deprecated echo.websocket.org to be replaced

#define WS_SERVER           "192.168.2.18"
  #define WS_PORT             4433
#else
  // To run a local WebSocket Server
//  #define WS_SERVER           "192.168.199.181"
#define WS_SERVER           "192.168.2.18"
//  #define WS_PORT             1234
#endif

bool alreadyConnected = false;
String wsID="";

String messageToSend = "From " ARDUINO_BOARD;

long pingInterval=1000;
void sendTXTMessage()
{
  static uint64_t sendTXTMessage_timeout = 0;

  uint64_t now = millis();

  //KH
#define SEND_INTERVAL         300L

  // sendTXTMessage every SEND_INTERVAL (30) seconds.
  if (now > sendTXTMessage_timeout)
  {
    //webSocket.sendTXT("message here");
    // creat JSON message
    DynamicJsonDocument doc(1024);
    JsonArray array = doc.to<JsonArray>();

    array.add(messageToSend);

    // add payload (parameters) for the event
    JsonObject param1 = array.createNestedObject();
    param1["com"]     = "ping";
     param1["data"]["id"]="carClient";

    // JSON to String (serializion)
    String output;
    serializeJson(doc, output);

    // Send event
//    webSocket.sendTXT(output);

    // Print JSON for debugging
    Serial.println(output);

    sendTXTMessage_timeout = millis() + SEND_INTERVAL;
  }
}

void webSocketEvent(const WStype_t& type, uint8_t * payload, const size_t& length)
{
  switch (type)
  {
    case WStype_DISCONNECTED:
      if (alreadyConnected)
      {
        Serial.println("[WSc] Disconnected!");
        alreadyConnected = false;
      }

      break;

    case WStype_CONNECTED:
    {
      alreadyConnected = true;

      Serial.print("[WSc] Connected to url: ");
//      Serial.println((char *) payload);

      // send message to server when Connected
//      webSocket.sendTXT("Connected");
        webSocket.sendTXT("{\"com\":\"car\"}");
    }
    break;

    case WStype_TEXT:
//      Serial.printf("[WSc] get text: %s\n", payload);
//      test((char *)payload);
      comhandler((char *)payload);
      break;

    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      hexdump(payload, length);

      // send data to server
      webSocket.sendBIN(payload, length);
      break;

    case WStype_PING:
      // pong will be send automatically
//      Serial.printf("[WSc] get ping\n");
      break;

    case WStype_PONG:
      // answer to a ping we send
//      Serial.printf("[WSc] get pong\n");
      break;

    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;

    default:
      break;
  }
}

void setup()
{
  // Serial.begin(921600);
  Serial.begin(115200);
  pinMode(SERVO_PIN,OUTPUT);
  pinMode(SERVO_Px_PIN,OUTPUT);
  pinMode(SERVO_Py_PIN,OUTPUT);
  pinMode(MOTOR_A_PIN_1, OUTPUT);
  pinMode(MOTOR_A_PIN_2, OUTPUT);
  pinMode(MOTOR_A_SPEED_PIN, OUTPUT);
  analogWrite(MOTOR_A_SPEED_PIN,0);//电机控制初始化
  
  myServo.attach(SERVO_PIN);
  myServo.write(45);//写入舵机默认角度

  ServoPX.attach(SERVO_Px_PIN);
  ServoPX.write(0);//初始化手机舵机X轴
  
  ServoPY.attach(SERVO_Py_PIN);
  ServoPY.write(0);//初始化手机舵机Y轴

  while (!Serial);

  delay(200);

  Serial.print("\nStart ESP8266_WebSocketClient on ");
  Serial.println(ARDUINO_BOARD);
  Serial.println(WEBSOCKETS_GENERIC_VERSION);

  //Serial.setDebugOutput(true);

  //WiFiMulti.addAP("LanceMag", "jtxq6bsb");
  WiFiMulti.addAP("ChinaNet-E6EJLR", "6QECLSPK");

  //WiFi.disconnect();
  while (WiFiMulti.run() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }

  Serial.println();

  // Client address
  Serial.print("WebSockets Client started @ IP address: ");
  Serial.println(WiFi.localIP());

  // server address, port and URL
  Serial.print("Connecting to WebSockets Server @ ");
  Serial.println(WS_SERVER);

  // server address, port and URL
#if USE_SSL
  webSocket.beginSSL(WS_SERVER, WS_PORT,"/ws");
#else
  webSocket.begin(WS_SERVER, WS_PORT, "/");
#endif


  // event handler
  webSocket.onEvent(webSocketEvent);

  // use HTTP Basic Authorization this is optional remove if not needed
  //webSocket.setAuthorization("user", "Password");

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);

  // start heartbeat (optional)
  // ping server every 15000 ms
  // expect pong from server within 3000 ms
  // consider connection disconnected if pong is not received 2 times
  webSocket.enableHeartbeat(15000, 3000, 2);

  // server address, port and URL
  Serial.print("Connected to WebSockets Server @ IP address: ");
  Serial.println(WS_SERVER);

  //localServoTest();//本地检查舵机是否正常
}

void loop()
{
  webSocket.loop();
  sendJsonPing();

//  sendTXTMessage();
  
}
void sendJsonPing()
{
  if (wsID=="")
  {
//    Serial.println("不发ping");
  }
  else
  {
    if (pingInterval>0)
    {
      pingInterval=pingInterval-1;
    }
    else
    {
      pingInterval=1000;
      webSocket.sendTXT("{\"com\":\"ping\",\"data\":{\"id\":\""+wsID+"\"}}");
    }
  }
}
void comhandler(const char* message)
{
  String mmg=String(message);
   DynamicJsonDocument doc(mmg.length()*2);
   deserializeJson(doc,mmg);
//   Serial.println(mmg);
   String com = doc["com"];
   if (com=="conn_success")
   {
    Serial.println("连接成功");
    wsID=(String)doc["data"]["id"];
   }
   if (com=="to")
   {
    int up=doc["data"]["up"].as<int>();
    int back=doc["data"]["back"].as<int>();
    int left=doc["data"]["left"].as<int>();
   int right=doc["data"]["right"].as<int>();
   int phonex=doc["data"]["phonex"].as<int>();
   int phoney=doc["data"]["phoney"].as<int>();
   Serial.print("up:");
   Serial.print(up);
   Serial.print(" back:");
   Serial.print(back);
   Serial.print(" left:");
   Serial.print(left);
   Serial.print(" right:");
   Serial.print(right);
   Serial.print(" phonex:");
   Serial.print(phonex);
   Serial.print(" phoney:");
   Serial.print(phoney);
   Serial.println();
   if (up>0)
   {//前进
     digitalWrite(MOTOR_A_PIN_1, LOW);
     digitalWrite(MOTOR_A_PIN_2, HIGH);
     analogWrite(MOTOR_A_SPEED_PIN,250);//
   }
   else
   {
    if (back>0)
    {
       digitalWrite(MOTOR_A_PIN_1, HIGH);
       digitalWrite(MOTOR_A_PIN_2, LOW);
       analogWrite(MOTOR_A_SPEED_PIN,back*200);//
    }
    else
    {
      analogWrite(MOTOR_A_SPEED_PIN,0);//
    }
   }
   //Serial.print(left);
   if (left>0)
   {
     myServo.write(90);//写入舵机默认角度
   }
   else
   {
    if (right>0)
    {
      myServo.write(0);//写入舵机默认角度
    }
    else
    {
      myServo.write(30);//写入舵机默认角度
    }
   }
   //写入手机支架舵机角度
   ServoPX.write(phonex);
   ServoPY.write(phoney);
    
   }
   
   
}
void test(const char* message)
{
  String mmg=String(message);
  DynamicJsonDocument doc(1024);
   deserializeJson(doc,mmg);
   String comm=doc["com"].as<String>();
   int numberInt = doc["val"].as<int>();
   Serial.print(comm);
   Serial.print(numberInt);
// Serial.println("test function");

 myServo.write(numberInt); 
}
void localServoTest()
{
    int pos;

  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}
