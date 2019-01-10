# <p align = "center">Arduino clock without RTC</p>

-  [Description](#description)
    - [Features](#features)
-  [Folders](#folders)
-  [Components](#components)
-  [Schemes](#schemes)

***

## Description
**Simple Arduino clock project without Real Time Clock**

### Features
- Big font for display
- Adaptive backlight
- Alarm
- Hour signal
- Display output:
    - Big Clock
    - Big Date
    - Alarm time

## Code settings
```c++
#define HOUR_SIGNAL 1 // (0 - off, 1 - on)
#define SMART_BACKLIGHT 1 // (0 - off, 1 - on)
#define TIME_SEPARATOR 0 //  time separator type (0 - none, 1 - dot, 2 - colon)
#define GET_TIME 1 // get the time of firmware download from pc (0 - off, 1 - on)
```

## Folders
**firmware** - folder with firmware for Arduino <br>
**schemes** - component wiring circuitry

## Components
- Arduino NANO
- LCD screen 1602 i2c
- Buzzer
- 3 Buttons
- Photoresistor
- Resistor (10 kΩ)

## Schemes
![scheme](https://github.com/musicfreakt/arduinoclock/blob/master/schemes/scheme_bb.jpg)
