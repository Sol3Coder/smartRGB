# smartRGB

esp8266作为tcp client

将esp8266的D5 D6 D7分别模拟输出R值，G值，B值控制RGB灯

服务端发送json样例：{"rValue":123,"gValue":80,"bValue":200}

smartLed为实训所用文件，不包含自由调整RGB功能

用法为服务器发送数字，共六组数字，1111代表红色，2255代表黄色，2060代表蓝色，2056代表绿色，2250代表粉色，1211代表关灯
