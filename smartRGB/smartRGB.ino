#include <ESP8266WiFi.h>        
#include <WiFiClient.h> 
#include <ArduinoJson.h>

/*****************网络设置项 *****************/
const char *STAssid ="";                //wifi名称
const char *STApassword="";           //wifi密码
const char *host= "";              //外网服务器ip
const int httpPort = ;                    //外网服务器端口
WiFiClient client;                            //声明客户端对象
bool flag=0;                                    //用来做连接到服务器后，仅发送一次问候语

void setup() {
  Serial.begin(115200);
  Serial.println("");
  pinMode(14,OUTPUT);//D5,R
  pinMode(12,OUTPUT);//D6,G
  pinMode(13,OUTPUT);//D7,B
  //digitalWrite(14,0);
  analogWrite(14,0);  
  WiFi.mode(WIFI_STA);                       //设置为STA模式 
  WiFi.begin(STAssid, STApassword);          //配置连接信息，开始连接
  //运行自定义函数，在未连接到路由器的过程中，打印输出......，直到连接成功   
  connectWifi();  
  Serial.println("连接路由器成功");
  //此时已经连接到路由器，可以开始连接远程服务器   
  connectServer();                                 
}

void loop() {   
/****************检测WiFi连接状态****************/ 
  if (WiFi.status() != WL_CONNECTED) { 
    WiFi.disconnect();
    flag = 0;
    WiFi.begin(STAssid, STApassword);
    connectWifi();  
  }
  else{
    if (!client.connected()) {            //如果没有连接到服务器
      flag = 0;
      connectServer();
      return;
    }
    else{                                //否则，就是连接到服务器
      if(flag == 0){
        client.print("connect success"); 
        flag = 1;      
      }
    }
  }
  tcpHandler(readTcp());                 //读取服务器信息，并处理
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

  //服务端json样例：{"rValue":123,"gValue":80,"bValue":200}
 if(data!=""){

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
  client.print(rValue); 
  client.print(gValue); 
  client.print(bValue); 
//模拟输出rgb值
  analogWrite(14,rValue);
  analogWrite(12,gValue);
  analogWrite(13,bValue);

 }
}
//连接路由器
void connectWifi(){
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);    
    Serial.print(".");
  }
}
//连接服务器
void connectServer() {
  Serial.println("开始连接服务器");
  client.connect(host, httpPort);
  delay(1000);
}
