# smartRGB


# 概述
在esp8266内烧写简单html，以完成web配网

利用eeprom断电保存特性完成自动重连

esp8266作为tcp client

将esp8266的D5 D6 D7分别模拟输出R值，G值，B值控制RGB灯

服务端发送json样例：{"rValue":123,"gValue":80,"bValue":200}

smartLed为实训所用文件，不包含自由调整RGB功能

用法为服务器发送数字，共六组数字，1111代表红色，2255代表黄色，2060代表蓝色，2056代表绿色，2250代表粉色，1211代表关灯

# 开发环境

arduino IDE，需在附加开发板管理器网址中输入下述网址以添加esp8266支持

http://arduino.esp8266.com/stable/package_esp8266com_index.json

添加支持后在管理库选项中搜索esp8266安装即可


# 程序描述
上电后开启STA+AP模式，进入配网状态

配网状态为一个while循环，首先读取eeprom值尝试重连，然后暂停一段时间，最后进行web配网，如网络连接成功，退出循环

由于没有按键无法设计外部中断，故进行web配网时，如果8266在尝试重连，web配网会失效，需多点几次连接

web配网流程为：将一段html保存在一个String变量里，开启server模式，利用get请求获取web发来的网络参数(可利用工具将html压缩为一行，同时需注意将 " 替换为 \" )

若网络连接成功，板载led会熄灭，可观察板载led判断配网是否成功

配网完成后进入loop循环，每一次循环会读取tcp server是否发送信息，根据server发来的信息调整rgb值




