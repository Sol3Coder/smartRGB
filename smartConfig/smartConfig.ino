#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <EEPROM.h>


//定义一个写和读的通用方法
#define EEPROM_write(address,p) { int i = 0;byte *pp = (byte*)&(p);for(;i<sizeof(p);i++) EEPROM.write(address+i,pp[i]);EEPROM.end();}
#define EEPROM_read(address,p) { int i = 0;byte *pp = (byte*)&(p);for(;i<sizeof(p);i++) pp[i]=EEPROM.read(address+i);}
//注意事项 写的结尾一定要有EEPROM.end();或EEPROM.commit();目的就是提交保存操作否则不保存成功
char serverInfo[128];
WiFiClient client;
const char* ssid = "CONFIG-ESP8266";
const char* password = "00001111";
String STAssid;
String STApassword;
String serverIP ;
String serverPort;

ESP8266WebServer server(80);
bool htmlFlag = false;
bool LED_Flag = false;
String str = 
"<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\"content=\"width=device-width, initial-scale=1.0\"><meta http-equiv=\"X-UA-Compatible\"content=\"ie=edge\"><title>我爱摸鱼-ESP8266配网</title></head><body><form name=\"my\">WiFiName：<input type=\"text\"name=\"s\"placeholder=\"请输入您WiFi的名称\"id=\"id\"><br>WiFiPassword：<input type=\"text\"name=\"p\"placeholder=\"请输入您WiFi的密码\"id=\"pwd\"><br>serverIP：<input type=\"text\"name=\"p\"placeholder=\"请输入您的服务器IP\"id=\"ip\"><br>ServerPort：<input type=\"text\"name=\"p\"placeholder=\"请输入您的服务器端口号\"id=\"port\"><br><input type=\"button\"value=\"连接\"onclick=\"wifi()\"></form><script language=\"javascript\">function wifi(){var ssid=id.value;var password=pwd.value;var serverIP=ip.value;var serverPort=port.value;var xmlhttp=new XMLHttpRequest();xmlhttp.open(\"GET\",\"/HandleVal?ssid=\"+ssid+\"&password=\"+password+\"&serverIP=\"+serverIP+\"&serverPort=\"+serverPort,true);xmlhttp.send();alert(\"WiFiName:\"+ssid+\"WiFiPassword:\"+password+\"serverIP:\"+serverIP+\"ServerPort:\"+serverPort)}</script></body></html>";
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
    Serial.println("get html"); 

    Serial.println(STAssid); 
    Serial.println(STApassword);
    Serial.println(serverIP); 
    Serial.println(serverPort);

    sprintf(serverInfo,"{\"STAssid\":\"%s\",\"STApassword\":\"%s\",\"serverIP\":\"%s\",\"serverPort\":%s}",STAssid.c_str(),STApassword.c_str(),serverIP.c_str(),serverPort.c_str());
    saveConfig();

    htmlFlag = true;
//    WiFi.begin(STAssid,STApassword);
//    connectServer();  
//    ESP.reset();
}

void saveConfig() {

  EEPROM.begin(256);//申请空间必须大于结构体长度，4的倍数
//  sprintf(serverInfo,"{\"serverIP\":\"%s\",\"serverPort\":%s}",serverIP.c_str(),serverPort.c_str());
  Serial.println("start eeprom write");
  Serial.println(sizeof(serverInfo));
  EEPROM_write(0, serverInfo);//写地址128+1 结构体2
  EEPROM.commit();
}

void readConfig()
{

  Serial.println("start read config");
  EEPROM.begin(256);//申请空间
  char readback[128];//申请变量
  EEPROM_read(0, readback);//读数据
//  Serial.print("config info:");
//  Serial.println(readback);//打印数据
//  Serial.println("read eeprom over!");

    /*-------json解析-------*/
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, readback);
  JsonObject obj = doc.as<JsonObject>();
  String ssid = doc["STAssid"]; 
  String password = doc["STApassword"]; 
  String IP = doc["serverIP"]; 
  String Port = doc["serverPort"]; 
  STAssid = ssid;
  STApassword = password;
  serverIP = IP;
  serverPort = Port;
  Serial.println("latest config:");
  Serial.print(STAssid);
  Serial.print("---");
  Serial.println(STApassword);
  Serial.print(serverIP);
  Serial.print("---");
  Serial.println(serverPort);
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

bool autoConfig()
{
//  WiFi.mode(WIFI_AP_STA);
  readConfig();
  WiFi.begin(STAssid,STApassword);
  Serial.println("Start Auto Config");
  for (int i = 0; i < 10; i++)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      if (client.connected()) 
        {            
            Serial.println("AutoConfig Success");
            Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
            Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
            Serial.printf("serverIP:%s\r\n", serverIP.c_str());
            Serial.printf("serverPort:%s\r\n", serverPort.c_str());
            WiFi.printDiag(Serial);
            WiFi.mode(WIFI_STA);
            return true;
        }
      else 
      {
            connectServer();  
      }
    }
    else
    {
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
//    WiFi.mode(WIFI_AP_STA);//设置模式为AP+STA
    digitalWrite(LED_BUILTIN, LOW);
    WiFi.softAP(ssid, password);
    Serial.println("Start Html Config");
    
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
    Serial.println("Waitting for html config");
    while(1)
    {

        server.handleClient();
        MDNS.update();
        if(htmlFlag)
        {
          if(autoConfig())
          {
            break;
          }
        }

        /*
        readConfig();
        WiFi.begin(STAssid,STApassword);
        Serial.println(STAssid);
        Serial.println(STApassword);
        Serial.println("try to connect wifi");
        Serial.print(".");
        delay(1000);
        if (WiFi.status() == WL_CONNECTED)
        {
          Serial.println("WIFI connect success");
          connectServer(); 
            if (client.connected()) 
            {            
                Serial.println("html config success");
                Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
                Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
                Serial.printf("serverIP:%s\r\n", serverIP.c_str());
                Serial.printf("serverPort:%s\r\n", serverPort.c_str());
                WiFi.printDiag(Serial);
                digitalWrite(LED_BUILTIN, HIGH); 
                WiFi.mode(WIFI_STA);
                break;
            }
        }

        */

    }  
}

void smartConfig(){
      WiFi.mode(WIFI_AP_STA);//设置模式为AP+STA
//      delay(1000);
      Serial.print("Stations connected =");
      Serial.println(WiFi.softAPgetStationNum());
      if(!WiFi.softAPgetStationNum())
      {
        autoConfig();
      }
      else{
        htmlConfig();
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

//读取服务器信息，返回String类型
String readTcp(){
  String data = "";          
  while (client.available() > 0){
    data += char(client.read());
    delay(2);    
  } 
  return data;
}

//处理服务器信息
void tcpHandler(String data){

 if(data!="")
 {
    Serial.print("receive data:");
    Serial.println(data);
    if(data=="1111"){
      analogWrite(14,255);  
      analogWrite(12,0); 
      analogWrite(13,0); 
      client.print("red");     
    }
    else if(data=="2255"){
      analogWrite(14,255); 
      analogWrite(12,184); 
      analogWrite(13,0);       
      client.print("yellow");      
    }
     else if(data=="2060"){
      analogWrite(14,0); 
      analogWrite(12,0); 
      analogWrite(13,255); 
      client.print("blue");      
    }
     else if(data=="2056"){
      analogWrite(14,0); 
      analogWrite(12,255); 
      analogWrite(13,0); 
      client.print("green");      
    }
     else if(data=="2250"){
      analogWrite(14,250); 
      analogWrite(12,40); 
      analogWrite(13,181); 
      client.print("pink");      
    }
     else if(data=="1211"){
      analogWrite(14,0); 
      analogWrite(12,0); 
      analogWrite(13,0); 
      client.print("off");      
    }
    else{
          /*-------json解析-------*/
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, data);
      JsonObject obj = doc.as<JsonObject>();
      int rValue = doc["rValue"]; // 100
      int bValue = doc["bValue"]; // 100
      int gValue = doc["gValue"]; // 100
      /*------串口打印rgb值----*/
      Serial.println(data);
      Serial.println(rValue);
      Serial.println(gValue);
      Serial.println(bValue);
    //将解析得rgb值发送回服务器
    
      client.print("rValue:"); 
      client.print(rValue); 
      client.print("---");
    
      client.print("---");
      client.print("gValue:"); 
      client.print(gValue); 
      client.print("---");
    
      client.print("---");
      client.print("bValue:"); 
      client.print(bValue); 
    
    //模拟输出rgb值
      analogWrite(14,rValue);
      analogWrite(12,gValue);
      analogWrite(13,bValue);

    }
  }  
}


void setup(void) {
   
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.begin(115200);
    smartConfig();
    pinMode(14,OUTPUT);//D5,R
    pinMode(12,OUTPUT);//D6,G
    pinMode(13,OUTPUT);//D7,B
    analogWrite(14,0); 
    analogWrite(12,0); 
    analogWrite(13,0); 
}

void loop(void) {

//  smartConfig();
    if (WiFi.status() != WL_CONNECTED) { 
      WiFi.disconnect();
      Serial.println("wifi failure");
      smartConfig();
    } 
    else if(!client.connected()){
      Serial.println("socket failure");
      WiFi.disconnect();
      smartConfig();
    }
    else{
      //digitalWrite(LED_BUILTIN, HIGH); 
      tcpHandler(readTcp()); 
    }
}
