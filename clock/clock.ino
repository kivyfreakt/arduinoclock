
//---------------libs---------------
#include <Time.h>
#include <TimeLib.h>
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
const byte NUM_TIMES = 5;
const byte ANUM_TIMES = 2;
const byte DEFAULT_TIME[NUM_TIMES] = {2019, 0, 0, 0, 0};
const byte MAX_TIME[NUM_TIMES] = {3000, 12, 31, 23, 59};
const byte NUM_SCREENS = 3;
//---------------lcd custom chars---------------

// Eight programmable character definitions
byte custom[8][8] = {
{ B11111,B11111,B11111,B00000,B00000,B00000,B00000,B00000 },
{ B11100,B11110,B11111,B11111,B11111,B11111,B11111,B11111 },
{ B11111,B11111,B11111,B11111,B11111,B11111,B01111,B00111 },
{ B00000,B00000,B00000,B00000,B00000,B11111,B11111,B11111 },
{ B11111,B11111,B11111,B11111,B11111,B11111,B11110,B11100 },
{ B11111,B11111,B11111,B00000,B00000,B00000,B11111,B11111 },
{ B11111,B00000,B00000,B00000,B00000,B11111,B11111,B11111 },
{ B00111,B01111,B11111,B11111,B11111,B11111,B11111,B11111 }
};

// Characters, each with top and bottom half strings
// \nnn string encoding is octal, so:
// \010 = 8 decimal (8th programmable character)
// \024 = 20 decimal (space)
// \377 = 255 decimal (black square)

const char *bigChars[][2] = {
{"\024\024\024", "\024\024\024"}, // Space
{"\377", "\007"}, // !
{"\005\005", "\024\024"}, // "
{"\004\377\004\377\004", "\001\377\001\377\001"}, // #
{"\010\377\006", "\007\377\005"}, // $
{"\001\024\004\001", "\004\001\024\004"}, // %
{"\010\006\002\024", "\003\007\002\004"}, // &
{"\005", "\024"}, // '
{"\010\001", "\003\004"}, // (
{"\001\002", "\004\005"}, // )
{"\001\004\004\001", "\004\001\001\004"}, // *
{"\004\377\004", "\001\377\001"}, // +
{"\024", "\005"}, // ,
{"\004\004\004", "\024\024\024"}, // -
{"\024", "\004"}, // .
{"\024\024\004\001", "\004\001\024\024"}, // /
{"\010\001\002", "\003\004\005"}, // 0
{"\024\002\024", "\024\377\024"}, // 1
{"\006\006\002", "\003\007\007"}, // 2
{"\006\006\002", "\007\007\005"}, // 3
{"\003\004\002", "\024\024\377"}, // 4
{"\377\006\006", "\007\007\005"}, // 5
{"\010\006\006", "\003\007\005"}, // 6
{"\001\001\002", "\024\010\024"}, // 7
{"\010\006\002", "\003\007\005"}, // 8
{"\010\006\002", "\024\024\377"}, // 9
{"\004", "\001"}, // :
{"\004", "\005"}, // ;
{"\024\004\001", "\001\001\004"}, // <
{"\004\004\004", "\001\001\001"}, // =
{"\001\004\024", "\004\001\001"}, // >
{"\001\006\002", "\024\007\024"}, // ?
{"\010\006\002", "\003\004\004"}, // @
{"\010\006\002", "\377\024\377"}, // A
{"\377\006\005", "\377\007\002"}, // B
{"\010\001\001", "\003\004\004"}, // C
{"\377\001\002", "\377\004\005"}, // D
{"\377\006\006", "\377\007\007"}, // E
{"\377\006\006", "\377\024\024"}, // F
{"\010\001\001", "\003\004\002"}, // G
{"\377\004\377", "\377\024\377"}, // H
{"\001\377\001", "\004\377\004"}, // I
{"\024\024\377", "\004\004\005"}, // J
{"\377\004\005", "\377\024\002"}, // K
{"\377\024\024", "\377\004\004"}, // L
{"\010\003\005\002", "\377\024\024\377"}, // M
{"\010\002\024\377", "\377\024\003\005"}, // N
{"\010\001\002", "\003\004\005"}, // 0/0
{"\377\006\002", "\377\024\024"}, // P
{"\010\001\002\024", "\003\004\377\004"}, // Q
{"\377\006\002", "\377\024\002"}, // R
{"\010\006\006", "\007\007\005"}, // S
{"\001\377\001", "\024\377\024"}, // T
{"\377\024\377", "\003\004\005"}, // U
{"\003\024\024\005", "\024\002\010\024"}, // V
{"\377\024\024\377", "\003\010\002\005"}, // W
{"\003\004\005", "\010\024\002"}, // X
{"\003\004\005", "\024\377\024"}, // Y
{"\001\006\005", "\010\007\004"}, // Z
{"\377\001", "\377\004"}, // [
{"\001\004\024\024", "\024\024\001\004"}, // Backslash
{"\001\377", "\004\377"}, // ]
{"\010\002", "\024\024"}, // ^
{"\024\024\024", "\004\004\004"}, // _
};


//---------------vars---------------
byte previous_min = minute();

enum tm{YEAR, MONTH, DAY, HOUR, MINUTE};
enum scr{TIME_SCREEN, DATE_SCREEN, ALARM_SCREEN};
byte screen = TIME_SCREEN;
byte previous_screen = screen;

bool update_flag = true;
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
  for (int i = 0; i < 8; i++){
    lcd.createChar(i+1, custom[i]);
  }
  lcd.backlight();
  get_time();
  writeBigString("101", 0, 0);
  tone(BUZZER_PIN, 2349 , 100);
  delay(1000);
  writeBigString("     ", 0, 0);
}


void loop(){
  if (alarm_flag && second() == 0 && alarm_time[1] == minute() && alarm_time[0] == hour()) { // если время совпадает
    alarm();
    alarm_flag = false;
  }

  if (second() == 0 && minute() == 0) {
    tone(BUZZER_PIN, 1000);
    delay(200);
    noTone(BUZZER_PIN);
    tone(BUZZER_PIN, 1000);
    delay(200);
    noTone(BUZZER_PIN);
  }

  if(minute() != previous_min){
    screen = TIME_SCREEN;
    update_flag = true;
    previous_min = minute();
  }

  if(update_flag or screen != previous_screen){
    update_flag = false;
    previous_screen = screen;
    show_screen();
  }
  set_lcd_led();
  button_listen();
}

void show_screen(){
  switch (screen) {
    case TIME_SCREEN:
      time_screen();
    break;
    case DATE_SCREEN:
      date_screen();
    break;
    case ALARM_SCREEN:
      alarm_screen();
    break;
  }
}

void time_screen() {
  lcd.clear();
  char hour_str[2], min_str[2];
  sprintf(hour_str, "%02d", hour());
  sprintf(min_str, "%02d", minute());
  writeBigString(hour_str, 1, 0);
  writeBigString(min_str, 9, 0);
}

void date_screen(){
  lcd.clear();
  char day_str[2];
  sprintf(day_str, "%02d", day());
  writeBigString(day_str, 0, 0);
  lcd.setCursor(7, 0);
  lcd.print(monthStr(month()));
}

void alarm_screen(){
  lcd.clear();
  if (alarm_flag){
    char hour_str[2], min_str[2];
    sprintf(hour_str, "%02d", alarm_time[0]);
    sprintf(min_str, "%02d", alarm_time[1]);
    writeBigString(hour_str, 1, 0);
    writeBigString(min_str, 9, 0);
  }else{
    writeBigString("OFF", 4, 0);
  }
}


void button_listen() {
  /* обработка кнопок */
  bool screen_btn = !digitalRead(UP_BTN);
  bool alarm_btn = !digitalRead(DOWN_BTN);
  bool settings_btn = !digitalRead(SET_BTN);

  if (screen_btn) {
    delay(500);
    if (screen < NUM_SCREENS - 1){
      screen++;
    } else {
      screen = TIME_SCREEN;
    }
  }

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
  int new_time[NUM_TIMES]; // массив с новыми временными промежутками
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

  setTime(new_time[3], new_time[4], 0, new_time[2], new_time[1], new_time[0]);
  previous_min = minute();
  lcd.clear();
  update_flag = true;
}

void mark_time(int marked , int arr[]) {
  /* отображение на экране временного промежутка, который был выбран,
     и его значение
  */

  switch (marked) {
    case YEAR:
      lcd.setCursor(0, 0);
      lcd.print("set year");
      break;
    case MONTH:
      lcd.setCursor(0, 0);
      lcd.print("set month");
      break;
    case DAY:
      lcd.setCursor(0, 0);
      lcd.print("set day");
      break;
    case HOUR:
      lcd.setCursor(0, 0);
      lcd.print("set hour");
      break;
    case MINUTE:
      lcd.setCursor(0, 0);
      lcd.print("set minute");
      break;
  }
  lcd.setCursor(0, 1);
  lcd.print(arr[marked]);
  lcd.print("   ");
}




void set_alarm() {
  /* установка будильника на выбранное время
     управление происходит кнопками
  */
  alarm_flag = true;
  update_flag = true;
  byte marked = 0; // временной отрезок, который сейчас изменяется

  lcd.clear();
  while (marked < ANUM_TIMES) { // пока отмеченное время меньше количества временных отрезков , обрабатываем кнопки
    bool up_btn = !digitalRead(UP_BTN);
    bool down_btn = !digitalRead(DOWN_BTN);
    bool set_btn = !digitalRead(SET_BTN);

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
    lcd.print(alarm_time[marked]);
    lcd.print("   ");
    delay(100);

    if (up_btn) { // обработка кнопки вверх
      if (alarm_time[marked] < MAX_TIME[marked + 3]) { // если устанавливаемый временной отрезок реален, то
        alarm_time[marked] = alarm_time[marked] + 1; // увеличиваем его значение на 1
      } else {
        alarm_time[marked] = DEFAULT_TIME[marked + 3];
      }
      delay(500);
    }
    if (down_btn) { // обработка кнопки вниз
      if (alarm_time[marked] > DEFAULT_TIME[marked + 3]) { // если устанавливаемый временной отрезок реален, то
        alarm_time[marked] = alarm_time[marked] - 1; // уменьшаем его значение на 1
      }      else {
        alarm_time[marked] = MAX_TIME[marked + 3];
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
  tone(BUZZER_PIN, 1000);
  delay(200);
  noTone(BUZZER_PIN);
  tone(BUZZER_PIN, 1000);
  delay(200);
  noTone(BUZZER_PIN);
}


void get_time(){
  const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  char Month[4];
  short Hour, Min, Sec, Day, Year, month_index;
  sscanf(__TIME__, "%d:%d:%d", &Hour, &Min, &Sec);
  sscanf(__DATE__, "%s %d %d", Month, &Day, &Year);
  for (month_index = 0; month_index < 12; month_index++) {
    if (strcmp(Month, monthName[month_index]) == 0) break;
  }
  setTime(Hour, Min, Sec, Day, month_index, Year);
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


int writeBigChar(char ch, int x, int y) {
  const char *(*blocks)[2] = NULL; // Pointer to an array of two strings (character pointers)
  if (ch < ' ' || ch > '_') // If outside our table range, do nothing
  return 0;
  blocks = &bigChars[ch-' ']; // Look up the definition
  for (int half = 0; half <=1; half++) {
    int t = x; // Write out top or bottom string, byte at a time
    for (const char *cp = (*blocks)[half]; *cp; cp++) {
      lcd.setCursor(t, y+half);
      lcd.write(*cp);
      t = (t+1) % 40; // Circular scroll buffer of 40 characters, loop back at 40
    }
    lcd.setCursor(t, y+half);
    lcd.write(' '); // Make space between letters, in case overwriting
  }
  return strlen((*blocks)[0]); // Return char width
}


void writeBigString(char *str, int x, int y) {
  char c;
  while ((c = *str++))
  x += writeBigChar(c, x, y);
}
