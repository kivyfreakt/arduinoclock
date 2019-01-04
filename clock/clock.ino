//---------------libs---------------
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

//---------------pins---------------
#define PHOTORESISTOR_PIN 14 // А0 пин фоторезистора
#define BUZZER_PIN  6 // пин пищалки
#define UP_BTN  5 // пин кнопки вверх(голос)
#define DOWN_BTN  4 // пин кнопки вниз(установка будильника)
#define SET_BTN  2 // пин кнопки установить(смена времени)
#define LCD_LED_PIN 3 // пин подсветки дисплея

//---------------constants---------------
const char time[] = __TIME__;
const byte NUM_TIMES = 2; // количество временных отрезков
const byte DEFAULT_TIME[NUM_TIMES] = {0, 0};
const byte MAX_TIME[NUM_TIMES] = {23, 59};

//---------------lcd custom chars---------------
const byte LT[8] = {0x07,0x0f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f};
const byte UB[8] = {0x1f,0x1f,0x1f,0x00,0x00,0x00,0x00,0x00};
const byte RT[8] = {0x1c,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f};
const byte LL[8] = {0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x0f,0x07};
const byte LB[8] = {0x00,0x00,0x00,0x00,0x00,0x1f,0x1f,0x1f};
const byte LR[8] = {0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1e,0x1c};
const byte MB[8] = {0x1f,0x1f,0x1f,0x00,0x00,0x00,0x1f,0x1f};
const byte block[8] = {0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f};

//---------------vars---------------
byte time_hour = atoi(&time[0]);
byte time_min   = atoi(&time[3]);
byte previous_min = time_min;
byte time_sec  = atoi(&time[6]);
bool alarm_flag = false; // флаг для проверки будильника (true - будильник включен, false - будилькик выключен)
int alarm_time[NUM_TIMES] = {0, 0}; // время будильника

unsigned long setClockpreviousMillis;
unsigned long every_second_timer = 0;



void setup()
{
  pinMode(UP_BTN, INPUT_PULLUP);
  pinMode(DOWN_BTN, INPUT_PULLUP);
  pinMode(SET_BTN, INPUT_PULLUP);
  pinMode(PHOTORESISTOR_PIN, INPUT);
  pinMode(LCD_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  analogWrite(LCD_LED_PIN, 255);
  lcd.init();
  lcd.createChar(0, LT);
  lcd.createChar(1, UB);
  lcd.createChar(2, RT);
  lcd.createChar(3, LL);
  lcd.createChar(4, LB);
  lcd.createChar(5, LR);
  lcd.createChar(6, MB);
  lcd.createChar(7, block);
  lcd.backlight();
  print_time();
}


void loop()
{

  if ( (millis() - every_second_timer) > 1000 ) {
    update_time();
    every_second_timer = millis();
  }


  if (alarm_flag && time_sec == 0 && alarm_time[1] == time_min && alarm_time[0] == time_hour) { // если время совпадает
    alarm();
  }

  if (time_sec == 0 && time_min == 0) {
    tone(BUZZER_PIN, 1000);
    delay(200);
    noTone(BUZZER_PIN);
    tone(BUZZER_PIN, 1000);
    delay(200);
    noTone(BUZZER_PIN);
  }

  if(time_min != previous_min){
    print_time();
  }

  set_lcd_led();
  button_listen();
}


void update_time()
{
  unsigned long setClockcurrentMillis = millis();
  unsigned long setClockelapsedMillis = (setClockcurrentMillis - setClockpreviousMillis);
  while ( setClockelapsedMillis > 999 )
  {
    time_sec++;
    if ( time_sec > 59 )  {
      time_min++;
      time_sec = 0;
    }
    if ( time_min > 59 )  {
      time_hour++;
      time_min = 0;
    }
    if ( time_hour > 23 ) {
      time_hour = 0;
    }
    setClockelapsedMillis -= 1000;
  }
  setClockpreviousMillis = setClockcurrentMillis - setClockelapsedMillis;
}


void print_time() {
  previous_min = time_min;
  lcd.clear();
  printDigits(time_hour / 10, 1);
  printDigits(time_hour % 10, 4);
  printDigits(time_min / 10, 9);
  printDigits(time_min % 10, 12);
}


void button_listen() {
  /* обработка кнопок */
  bool screen_btn = !digitalRead(UP_BTN);
  bool alarm_btn = !digitalRead(DOWN_BTN);
  bool settings_btn = !digitalRead(SET_BTN);


  if (alarm_btn) { // обработка кнопки будильника
    delay(500);
    if (alarm_flag) { // если будильник включен,
      alarm_flag = false; // то выключить его
    } else {
      set_alarm(); // если нет , то настроить время будильника
    }
  }

  if (settings_btn) { // обработка кнопки изменения времени
    delay(500);
    set_time();
  }

}

void set_time() {
  /* изменение текущего времени на другое
     управление происходит  кнопками
  */
  lcd.clear();
  int new_time[2]; // массив с новыми временными промежутками
  byte marked = 0; // временной отрезок, который сейчас изменяется

  for (byte i = 0; i < NUM_TIMES ; i++) {
    new_time[i] = DEFAULT_TIME[i];
  }

  while (marked < NUM_TIMES) {// пока отмеченное время меньше количества временных отрезков , обрабатываем кнопки
    bool up_btn = !digitalRead(UP_BTN);
    bool down_btn = !digitalRead(DOWN_BTN);
    bool set_btn = !digitalRead(SET_BTN);

    mark_time(marked, new_time);
    delay(100);

    if (up_btn) { // обработка кнопки вверх
      if (new_time[marked] < MAX_TIME[marked]) { // если устанавливаемый временной отрезок реален, то
        new_time[marked] = new_time[marked] + 1; // увеличиваем его значение на 1
      } else {
        new_time[marked] = DEFAULT_TIME[marked];
      }
      delay(500);
    }
    if (down_btn) { // обработка кнопки вниз
      if (new_time[marked] > DEFAULT_TIME[marked]) { // если устанавливаемый временной отрезок реален, то
        new_time[marked] = new_time[marked] - 1; // уменьшаем его значение на 1
      } else {
        new_time[marked] = MAX_TIME[marked];
      }
      delay(500);
    }
    if (set_btn) { // обработка кнопки изменить
      marked++; // при нажатии меняем временной отрезок
      delay(500);
    }

  }
  time_hour = new_time[0];
  time_min  = new_time[1];
  previous_min = time_min;
  time_sec = 0;
  lcd.clear();
}

void set_alarm() {
  /* установка будильника на выбранное время
     управление происходит кнопками
  */
  alarm_flag = true;
  byte marked = 0; // временной отрезок, который сейчас изменяется

  while (marked < NUM_TIMES) { // пока отмеченное время меньше количества временных отрезков , обрабатываем кнопки
    bool up_btn = !digitalRead(UP_BTN);
    bool down_btn = !digitalRead(DOWN_BTN);
    bool set_btn = !digitalRead(SET_BTN);

    mark_time(marked, alarm_time);
    delay(100);

    if (up_btn) { // обработка кнопки вверх
      if (alarm_time[marked] < MAX_TIME[marked]) { // если устанавливаемый временной отрезок реален, то
        alarm_time[marked] = alarm_time[marked] + 1; // увеличиваем его значение на 1
      } else {
        alarm_time[marked] = DEFAULT_TIME[marked];
      }
      delay(500);
    }
    if (down_btn) { // обработка кнопки вниз
      if (alarm_time[marked] > DEFAULT_TIME[marked]) { // если устанавливаемый временной отрезок реален, то
        alarm_time[marked] = alarm_time[marked] - 1; // уменьшаем его значение на 1
      }      else {
        alarm_time[marked] = MAX_TIME[marked];
      }
      delay(500);
    }
    if (set_btn) { // обработка кнопки изменить
      marked++; // при нажатии меняем временной отрезок
      delay(500);
    }

  }
  lcd.clear();
}

void alarm() {
  /*функция срабатывания будильника*/
  alarm_flag = false;
  tone(BUZZER_PIN, 1000);
  delay(200);
  noTone(BUZZER_PIN);
  tone(BUZZER_PIN, 1000);
  delay(200);
  noTone(BUZZER_PIN);
}


void mark_time(int marked , int arr[]) {
  /* отображение на экране временного промежутка, который был выбран,
     и его значение
  */

  switch (marked) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("set hour");
      break;
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("set minute");
      break;
  }
  lcd.setCursor(0, 1);
  lcd.print(arr[marked]);
  lcd.print("   ");
}




void custom0(int x) {
  lcd.setCursor(x, 0);
  lcd.write((byte)0);
  lcd.write((byte)1);
  lcd.write((byte)2);
  lcd.setCursor(x, 1);
  lcd.write((byte)3);
  lcd.write((byte)4);
  lcd.write((byte)5);
}
void custom1(int x) {
  lcd.setCursor(x, 0);
  lcd.print(" ");
  lcd.write((byte)2);
  lcd.print(" ");
  lcd.setCursor(x, 1);
  lcd.print(" ");
  lcd.write((byte)7);
  lcd.print(" ");
}
void custom2(int x) {
  lcd.setCursor(x, 0);
  lcd.write((byte)6);
  lcd.write((byte)6);
  lcd.write((byte)2);
  lcd.setCursor(x, 1);
  lcd.write((byte)3);
  lcd.write((byte)4);
  lcd.write((byte)4);
}
void custom3(int x) {
  lcd.setCursor(x, 0);
  lcd.write((byte)6);
  lcd.write((byte)6);
  lcd.write((byte)2);
  lcd.setCursor(x, 1);
  lcd.write((byte)4);
  lcd.write((byte)4);
  lcd.write((byte)5);
}
void custom4(int x) {
  lcd.setCursor(x, 0);
  lcd.write((byte)3);
  lcd.write((byte)4);
  lcd.write((byte)7);
  lcd.setCursor(x, 1);
  lcd.print(" ");
  lcd.print(" ");
  lcd.write((byte)7);
}
void custom5(int x) {
  lcd.setCursor(x, 0);
  lcd.write((byte)3);
  lcd.write((byte)6);
  lcd.write((byte)6);
  lcd.setCursor(x, 1);
  lcd.write((byte)4);
  lcd.write((byte)4);
  lcd.write((byte)5);
}
void custom6(int x) {
  lcd.setCursor(x, 0);
  lcd.write((byte)0);
  lcd.write((byte)6);
  lcd.write((byte)6);
  lcd.setCursor(x, 1);
  lcd.write((byte)3);
  lcd.write((byte)4);
  lcd.write((byte)5);
}
void custom7(int x) {
  lcd.setCursor(x, 0);
  lcd.write((byte)1);
  lcd.write((byte)1);
  lcd.write((byte)2);
  lcd.setCursor(x, 1);
  lcd.print(" ");
  lcd.print(" ");
  lcd.write(7);
}
void custom8(int x) {
  lcd.setCursor(x, 0);
  lcd.write((byte)0);
  lcd.write((byte)6);
  lcd.write((byte)2);
  lcd.setCursor(x, 1);
  lcd.write((byte)3);
  lcd.write((byte)4);
  lcd.write((byte)5);
}
void custom9(int x) {
  lcd.setCursor(x, 0);
  lcd.write((byte)0);
  lcd.write((byte)6);
  lcd.write((byte)2);
  lcd.setCursor(x, 1);
  lcd.print(" ");
  lcd.print(" ");
  lcd.write((byte)7);
}
void printDigits(int digits, int x) {
  // utility function for digital clock display: prints preceding colon and leading 0
  switch (digits) {
    case 0:
      custom0(x);
      break;
    case 1:
      custom1(x);
      break;
    case 2:
      custom2(x);
      break;
    case 3:
      custom3(x);
      break;
    case 4:
      custom4(x);
      break;
    case 5:
      custom5(x);
      break;
    case 6:
      custom6(x);
      break;
    case 7:
      custom7(x);
      break;
    case 8:
      custom8(x);
      break;
    case 9:
      custom9(x);
      break;
  }
}


void set_lcd_led() {
  /* установка яркости подсветки дисплея,
     исходя из значения, полученного с фоторезистора
  */
  int bright = map(analogRead(PHOTORESISTOR_PIN), 320, 1024, 0, 5); // переводим полученные значения с фоторезистора в диапазон от 0 до 5
  if (bright < 1) { // если яркость меньше чем 1, то
    bright = 1; // устанавливаем значение на 1
  }
  analogWrite(LCD_LED_PIN, bright * 51); // устанавливаем яркость дисплея
}
