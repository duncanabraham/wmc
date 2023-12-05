ESP8266
```
 9  RST   o          o  TX   8 
10  A0    o          o  RX   7
11  D0    o          o  D1   6
12  D5    o          o  D2   5
13  D6    o          o  D3   4
14  D7    o          o  D4   3
15  D8    o          o  GND  2
16  3v3   o          o  5v   1
```


AS5600  (A)
```
1  VCC   o          o  DIR  7
2  OUT   o          o  SCL  6
3  GND   o          o  SDA  5
                    o  GPO  4
```

AHT21 
1  VIN
2  GND
3  SCL
4  SDA


BTS7960  (B)
```
Control Pins:
1  RPWM  o  o  LPWM  8
2  R_EN  o  o  L_EN  7
3  R_IS  o  o  L_IS  6
4  VCC   o  o  GND   5

Power and Motor:
o  B-
o  B+
o  M+
o  M-
```

Connections:
```
ESP8266     AS5600     BTS7960      AHT21
3v3         VCC (1)                 VIN (1)
GND         GND (3)    GND (5)      GND (2)
D1 (GPIO5)             SCL (6)      SCL (3)
D2 (GPIO4)             SDA (5)      SDA (4)
D5 (GPIO14)            RPWM (1)
D6 (GPIO12)            LPWM (8)
D7 (GPIO13)            R_EN (2)
D8 (GPIO15)            L_EN (7)
5v                     VCC (4)
```