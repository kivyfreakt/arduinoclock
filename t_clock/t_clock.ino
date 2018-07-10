// говорящие часы

//---------------libs---------------
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include "SoftwareSerial.h"
#include <mp3TF.h>


//---------------pins---------------
#define UP_BTN  4 // пин кнопки вверх(голос)
#define DOWN_BTN  7 // пин кнопки вниз(установка будильника)
#define SET_BTN  8 // пин кнопки установить(смена времени)
#define PHOTORESISTOR_PIN 14 // А0 пин фоторезистора
#define LCD_LED_PIN  11 // пин подсветки дисплея

//---------------constants---------------
const int BTN_DELAY = 500; //задержка времени в программе на 500 мс, которая происходит после нажатия на кнопку (самый примитивный метод избежания дребезга контактов)
const byte NUM_TIMES = 6; // количество временных отрезков*
//* - (годы, месяцы, дни, часы, минуты, секунды)
const byte NUM_ALARM_TIMES = 2;
const String DAYS_OF_THE_WEEK[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const int DEFAULT_TIME[NUM_TIMES] = {2018, 1, 1, 0, 0, 0};
const int MAX_TIME[NUM_TIMES] = {3000, 12, 31, 23, 59, 59};

//---------------lcd custom chars---------------
byte alarm_char[] = {0x04, 0x0E, 0x0E, 0x0E, 0x1F, 0x00, 0x04, 0x00}; // иконка колокольчика , обозначающая , что будильник работает

//---------------vars---------------
enum td {YEAR, MONTH, DAY, HOUR, MINUTE, SECOND}; // перечисление временных отрезков для удобной работы
bool alarm_flag = false; // флаг для проверки будильника (true - будильник включен, false - будилькик выключен)
int alarm_time[NUM_ALARM_TIMES] = {0, 0}; // время будильника
int bright;

mp3TF mp3 = mp3TF ();
SoftwareSerial SoundSerial(12, 13);
LiquidCrystal_I2C lcd(0x27, 20, 4);
RTC_DS1307 rtc;

//---------------all functions---------------
void say_time(); // озвучка времени
void alarm(); // функция будильника
void set_alarm(); // установка будильника
void set_lcd_led(); // установка уровня яркости подстветки экрана
void mark_time(int marked , int new_time[6]); // отметка времени для уставки
void set_time(); // установка времени
void print_format(int value); // изменение значений даты/времени в более удобную форму
void button_control(); // обработка кнопок
void print_all(); // вывод всей информации на дисплей


void setup() {
  pinMode(UP_BTN, INPUT_PULLUP);
  pinMode(DOWN_BTN, INPUT_PULLUP);
  pinMode(SET_BTN, INPUT_PULLUP);
  pinMode(PHOTORESISTOR_PIN, INPUT);
  pinMode(LCD_LED_PIN, OUTPUT);
  analogWrite(LCD_LED_PIN, 255);
  // инициализация LCD
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, alarm_char);
  // инициализация RTC
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // инициализация MP3
  SoundSerial.begin(9600);
  mp3.init(&SoundSerial); // включаем передачу данных с DFPlayer mini mp3
  delay(2);
  mp3.volumeSet(30);// устанавливаем громкость от 0 до 30
  delay(2);
}

void loop() {
  if (millis() % 1000 == 0) { // если прошла одна секунда
    print_all(); // выводим всю информацию на экран
    delay(1);
  }

  DateTime now = rtc.now();
  if (alarm_flag) { // если будильник включен
    if (now.second() == 0) { // если секунды равны 0
      if (alarm_time[1] == now.minute()) { // если минуты совпадают
        if (alarm_time[0] == now.hour()) { // если совпадают часы
          alarm();
        }
      }
    }
  }

  set_lcd_led(); // изменяем яркость дисплея
  button_control(); // передаем управление кнопкам
}


void print_all() {
  /* получаем значения из различных датчиков
     и выводим их(значения) на экран
  */
  DateTime now = rtc.now(); // получаем текущее время
  lcd.setCursor(0, 0);
  print_format(now.hour()); // часы
  lcd.print(':');
  print_format(now.minute()); // минуты
  lcd.print(" ");
  print_format(now.day()); // дни
  lcd.print('/');
  print_format(now.month()); // месяцы
  lcd.print(" ");
  lcd.print(DAYS_OF_THE_WEEK[now.dayOfTheWeek()]); // дни недели
  if (alarm_flag) { // если включен будильник, то
    lcd.write(0); // выводим знак будильника
  } else {
    lcd.print(" ");
  }
}

void button_control() {
  /* обработка 3 кнопок */
  bool voice_btn = !digitalRead(UP_BTN);
  bool alarm_btn = !digitalRead(DOWN_BTN);
  bool settings_btn = !digitalRead(SET_BTN);

  lcd.setCursor(0, 1);

  if (voice_btn) { // обработка кнопки голоса
    say_time();
    delay(BTN_DELAY);
  }

  if (settings_btn) { // обработка кнопки будильника
    delay(BTN_DELAY);
    if (alarm_flag) { // если будильник включен,
      alarm_flag = false; // то выключить его
    } else {
      set_alarm(); // если нет , то настроить время будильника
    }
  }

  if (settings_btn) { // обработка кнопки изменения времени
    delay(BTN_DELAY);
    set_time();
  }

}

void alarm() {
  /*функция срабатывания будильника*/
  while (digitalRead(SET_BTN) == LOW) { // пока какая-либо кнопка не нажата
    mp3.playTrackPhisical(1);
  }

}

void say_time() {
  /*озвучивание текущего времени*/
  DateTime now = rtc.now();

  mp3.playTrackPhisical(now.hour() + 60);
  delay(1150);
  mp3.playTrackPhisical(now.minute() - 1);
}

void print_format(int value) {
  /* в библиотеке RTClib часы,минуты,секунды... ,состоящие из
     одного знака, выводятся без ведущего нуля.
    эта функция сравнивает значение, полученное с часов,
    и если оно меньше 10 , то приписывает ведущий нолик
  */
  if (value <= 9) { // если принимаемое значение меньше или равно 9, то
    lcd.print("0"); // приписываем ведущий нолик
  }
  lcd.print(value); // выводим значение
}

void set_time() {
  /* изменение текущего времени на другое
     управление происходит  кнопками
  */
  int new_time[6]; // массив с новыми временными промежутками
  int marked = 0; // временной отрезок, который сейчас изменяется

  for (int i = 0; i < NUM_TIMES ; i++) {
    new_time[i] = DEFAULT_TIME[i];
  }

  while (marked < NUM_TIMES) {// пока отмеченное время меньше количества временных отрезков , обрабатываем кнопки
    bool up_btn = !digitalRead(UP_BTN);
    bool down_btn = !digitalRead(DOWN_BTN);
    bool set_btn = !digitalRead(SET_BTN);

    if (up_btn) { // обработка кнопки вверх
      if (new_time[marked] < MAX_TIME[marked]) { // если устанавливаемый временной отрезок реален, то
        new_time[marked] = new_time[marked] + 1; // увеличиваем его значение на 1
      } else {
        new_time[marked] = DEFAULT_TIME[marked];
      }
      delay(BTN_DELAY);
    }
    if (down_btn) { // обработка кнопки вниз
      if (new_time[marked] > DEFAULT_TIME[marked]) { // если устанавливаемый временной отрезок реален, то
        new_time[marked] = new_time[marked] - 1; // уменьшаем его значение на 1
      } else {
        new_time[marked] = MAX_TIME[marked];
      }
      delay(BTN_DELAY);
    }
    if (set_btn) { // обработка кнопки изменить
      marked++; // при нажатии меняем временной отрезок
      delay(BTN_DELAY);
    }
    mark_time(marked, new_time);
    delay(100);
  }

  rtc.adjust(DateTime(new_time[YEAR], new_time[MONTH], new_time[DAY], new_time[HOUR], new_time[MINUTE], new_time[SECOND]));// устанавливаем время
}

void set_alarm() {
  /* установка будильника на выбранное время
     управление происходит кнопками
  */
  alarm_flag = true;
  int marked = 0; // временной отрезок, который сейчас изменяется

  while (marked < NUM_ALARM_TIMES) { // пока отмеченное время меньше количества временных отрезков , обрабатываем кнопки
    bool up_btn = !digitalRead(UP_BTN);
    bool down_btn = !digitalRead(DOWN_BTN);
    bool set_btn = !digitalRead(SET_BTN);
    
    if (up_btn) { // обработка кнопки вверх
      if (alarm_time[marked] < MAX_TIME[marked + 3]) { // если устанавливаемый временной отрезок реален, то
        alarm_time[marked] = alarm_time[marked] + 1; // увеличиваем его значение на 1
      } else {
        alarm_time[marked] = DEFAULT_TIME[marked + 3];
      }
      delay(BTN_DELAY);
    }
    if (down_btn) { // обработка кнопки вниз
      if (alarm_time[marked] > DEFAULT_TIME[marked + 3]) { // если устанавливаемый временной отрезок реален, то
        alarm_time[marked] = alarm_time[marked] - 1; // уменьшаем его значение на 1
      }      else {
        alarm_time[marked] = MAX_TIME[marked + 3];
      }
      delay(BTN_DELAY);
    }
    if (set_btn) { // обработка кнопки изменить
      marked++; // при нажатии меняем временной отрезок
      delay(BTN_DELAY);
    }
    lcd.clear();
    switch (marked) {
      case 0:
        lcd.print("set hour");
        break;
      case 1:
        lcd.print("set minute");
        break;
    }
    lcd.setCursor(0, 1);
    lcd.print(alarm_time[marked]);
    delay(100);
  }
}

void mark_time(int marked , int arr[]) {
  /* отображение на экране временного промежутка, который был выбран,
     и его значение
  */
  lcd.clear();
  switch (marked) {
    case YEAR:
      lcd.print("set year");
      break;
    case MONTH:
      lcd.print("set month");
      break;
    case DAY:
      lcd.print("set day");
      break;
    case HOUR:
      lcd.print("set hour");
      break;
    case MINUTE:
      lcd.print("set minute");
      break;
    case SECOND:
      lcd.print("set second");
      break;
  }
  lcd.setCursor(0, 1);
  lcd.print(arr[marked]);
}

void set_lcd_led() {
  /* установка яркости подсветки дисплея,
     исходя из значения, полученного с фоторезистора
  */
  bright = map(analogRead(PHOTORESISTOR_PIN), 320, 1024, 0, 5); // устанавливаем яркость дисплея
  if (bright < 1) { // если яркость меньше чем 1, то
    bright = 1; // устанавливаем значение на 1
  }
  analogWrite(LCD_LED_PIN, bright * 51); // устанавливаем яркость дисплея
}
