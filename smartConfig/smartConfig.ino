#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

WiFiClient client;
const char* ssid = "CONFIG-ESP8266";
const char* password = "00001111";
String STAssid;
String STApassword;
String serverIP ;
String serverPort;

ESP8266WebServer server(80);
bool LED_Flag = false;
String str = 
"<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\"content=\"width=device-width, initial-scale=1.0\"><meta http-equiv=\"X-UA-Compatible\"content=\"ie=edge\"><title>我爱摸鱼-ESP8266配网</title></head><body><form name=\"my\">WiFiName：<input type=\"text\"name=\"s\"placeholder=\"请输入您WiFi的名称\"id=\"id\"><br>WiFiPassword：<input type=\"text\"name=\"p\"placeholder=\"请输入您WiFi的密码\"id=\"pwd\"><br>serverIP：<input type=\"text\"name=\"p\"placeholder=\"请输入您的服务器IP\"id=\"ip\"><br>ServerPort：<input type=\"text\"name=\"p\"placeholder=\"请输入您的服务器端口号\"id=\"port\"><br><input type=\"button\"value=\"连接\"onclick=\"wifi()\"></form><script language=\"javascript\">function wifi(){var ssid=id.value;var password=pwd.value;var serverIP=ip.value;var serverPort=port.value;var xmlhttp=new XMLHttpRequest();xmlhttp.open(\"GET\",\"/HandleVal?ssid=\"+ssid+\"&password=\"+password+\"&serverIP=\"+serverIP+\"&serverPort=\"+serverPort,true);xmlhttp.send()}</script></body></html>";
/*****************************************************
 * 函数名称：handleRoot()
 * 函数说明：客户端请求回调函数
 * 参数说明：无
******************************************************/
void handleRoot() {
  server.send(200, "text/html", str);
}
/*****************************************************
 * 函数名称：HandleVal()
 * 函数说明：对客户端请求返回值处理
 * 参数说明：无
******************************************************/
void HandleVal()
{
    STAssid = server.arg("ssid"); //从JavaScript发送的数据中找ssid的值
    STApassword = server.arg("password"); //从JavaScript发送的数据中找password的值
    serverIP = server.arg("serverIP"); //从JavaScript发送的数据中找ssid的值
    serverPort = server.arg("serverPort"); //从JavaScript发送的数据中找ssid的值
    Serial.println(STAssid); 
    Serial.println(STApassword);
    Serial.println(serverIP); 
    Serial.println(serverPort);
    WiFi.begin(STAssid,STApassword);
    connectServer();  
}
/*****************************************************
 * 函数名称：handleNotFound()
 * 函数说明：响应失败函数
 * 参数说明：无
******************************************************/
void handleNotFound() {
  digitalWrite(LED_BUILTIN, 0);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(LED_BUILTIN, 1);
}
/*****************************************************
 * 函数名称：autoConfig()
 * 函数说明：自动连接WiFi函数
 * 参数说明：无
 * 返回值说明:true：连接成功 false：连接失败
******************************************************/
bool autoConfig()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  Serial.print("AutoConfig Waiting......");
  for (int i = 0; i < 5; i++)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      connectServer();  
      if (client.connected()) 
        {            
            Serial.println("AutoConfig Success");
            Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
            Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
            Serial.printf("serverIP:%s\r\n", serverIP.c_str());
            Serial.printf("serverPort:%s\r\n", serverPort.c_str());
            WiFi.printDiag(Serial);
            return true;
        }
    }
    else
    {
      Serial.print(".");
      LED_Flag = !LED_Flag;
      if(LED_Flag)
          digitalWrite(LED_BUILTIN, HIGH);
      else
          digitalWrite(LED_BUILTIN, LOW); 
      delay(500);
    }
  }
  Serial.println("AutoConfig Faild!" );
  return false;
  //WiFi.printDiag(Serial);
}
/*****************************************************
 * 函数名称：htmlConfig()
 * 函数说明：web配置WiFi函数
 * 参数说明：无
******************************************************/
void htmlConfig()
{

    WiFi.mode(WIFI_AP_STA);//设置模式为AP+STA
    digitalWrite(LED_BUILTIN, LOW);
    WiFi.softAP(ssid, password);
    Serial.println("Start AP mode");
    
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  
    if (MDNS.begin("esp8266")) {
      Serial.println("MDNS responder started");
    }
  
    server.on("/", handleRoot);
    server.on("/HandleVal", HTTP_GET, HandleVal);
    server.onNotFound(handleNotFound);//请求失败回调函数
  
    server.begin();//开启服务器
    Serial.println("HTTP server started");

    Serial.println("Waitting for config ");
    while(1)
    {
        server.handleClient();
        MDNS.update();

            connectServer();  

        if (WiFi.status() == WL_CONNECTED)
        {
            if (client.connected()) 
            {            //如果没有连接到服务器
                Serial.println("HtmlConfig Success");
                Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
                Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
                Serial.printf("serverIP:%s\r\n", serverIP.c_str());
                Serial.printf("serverPort:%s\r\n", serverPort.c_str());
                 break;
            }
               
        }

    }  
}

void connectServer() {
  if(serverIP!=""){
      Serial.println("Try to connecet server");
      Serial.println(serverIP);
      client.connect(serverIP, serverPort.toInt());
      delay(1000);
  }
}

void setup(void) {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.begin(115200);
    bool wifiConfig = autoConfig();
    if(wifiConfig == false)
        htmlConfig();//HTML配网
}

void loop(void) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    client.println(String("test"));
}
