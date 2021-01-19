#include <SPI.h>
#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>

// Developed by MAX_FORYS
// 09.07.2020
// Ver. 1.3.0
#include <Adafruit_NeoPixel.h>
#include "GyverButton.h"
#include "RTClib.h"
// Присваиваем имя для аналогового входа A0 с фоторезистором
#define SENSOR A0
// Присваиваем имя для цифрового значения аналогового входа A0
// unsigned int округляет значения и принимает только положительные числа
unsigned int value = 0;

//nRF24L01
const uint64_t pipe = 0xF0F1F2F3F4LL; // индитификатор передачи, "труба"

RF24 radio(5, 3); // CE, CSN
//

GButton Butt1(8);
GButton Butt2(10);
GButton Butt3(9);
RTC_DS3231 rtc;
unsigned long previousMillis = 0;
int ledState = LOW;
int autoBrightness = 0;
int buz = 7;
int ledLevel = 255;
int mode = 1;
int color = 0;
int ts = 0;
int setmode = 0;
#define NUM_LEDS 174 // 5 by segment + 6 in the middle
#define PIXEL_SGMENT 29 //количество светодиодов в цифре
#define LED_TYPE WS2812
#define COLOR_ORDER GRB // Define color order for your strip
#define PIN 6 // Data pin for led comunication
Adafruit_NeoPixel strip_a = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800); // Define LEDs strip Определение полосы светодиодов
Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, 4, NEO_GRB + NEO_KHZ800);

byte digits[10][29] = {
{0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1},
{1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0},
{1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},
{1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1},
{1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1},
{1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1}
};


 // ниже коды цветовых оттенков

long ledColor = 0xFF0000;

long ColorTable[7] {
0xFF0000,
0xFF0000,
0xFFFF00,
0x008000,
0x00FFFF,
0x0000FF,
0x800080};

void setup(){
  //
  radio.begin();
  delay(2);
  radio.setChannel(9); // канал (0-127)
    
      // скорость, RF24_250KBPS, RF24_1MBPS или RF24_2MBPS
      // RF24_250KBPS на nRF24L01 (без +) неработает.
      // меньше скорость, выше чувствительность приемника.
  radio.setDataRate(RF24_1MBPS); 
   
      // мощьность передатчика, RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM,
  radio.setPALevel(RF24_PA_HIGH);   

  radio.openWritingPipe(pipe); // открываем трубу на передачу.
  //
  
  
  strip.setBrightness(255);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Butt1.setDebounce(80);        // настройка антидребезга (по умолчанию 80 мс)
  Butt1.setTimeout(500);        // настройка таймаута на удержание (по умолчанию 500 мс)
  Butt1.setClickTimeout(300);   // настройка таймаута между кликами (по умолчанию 300 мс)
  Butt2.setDebounce(80);        // настройка антидребезга (по умолчанию 80 мс)
  Butt2.setTimeout(500);        // настройка таймаута на удержание (по умолчанию 500 мс)
  Butt2.setClickTimeout(300);   // настройка таймаута между кликами (по умолчанию 300 мс)
  Butt3.setDebounce(80);        // настройка антидребезга (по умолчанию 80 мс)
  Butt3.setTimeout(500);        // настройка таймаута на удержание (по умолчанию 500 мс)
  Butt3.setClickTimeout(300);   // настройка таймаута между кликами (по умолчанию 300 мс)
  Butt1.setType(HIGH_PULL);
  Butt2.setType(HIGH_PULL);
  Butt3.setType(HIGH_PULL);
  Butt1.setDirection(NORM_OPEN);
  Butt2.setDirection(NORM_OPEN);
  Butt3.setDirection(NORM_OPEN);
  Serial.begin(9600);
  strip_a.setBrightness(ledLevel);
  strip_a.begin();
  strip_a.show(); // Initialize all pixels to 'off'
rtc.begin();
pinMode(buz, OUTPUT);
long ledColor = 0xffdd33;
  // Пин A0 с фоторезистором будет входом (англ. «input»)
  pinMode(SENSOR, INPUT);
}

void TimeToArray(){
DateTime now = rtc.now();
    int hour=now.hour();
    int minute=now.minute();
    int second =now.second();
// ниже вывод цифр
displeyLed(second,145,116);
displeyLed(minute,87,58); // вызов функции и передача значени часов и минут. Такой способ сократил затрачиваемую общую память.
displeyLed(hour,29,0);
}


void displeyLed(int Now, int cursor, int cursor1){ // Now-двухзначное число кот-е надо вывести,
  //cursor- последнее значение двух-го числа, cursor1- первое значение двух-го числа
for(int i=1;i<=2;i++){
int digit = Now % 10;
for(int k=0; k <= PIXEL_SGMENT-1;k++){
  if (digits[digit][k]== 1){
strip_a.setPixelColor(cursor, ledColor);

}
else if (digits[digit][k]==0){
strip_a.setPixelColor(cursor, 0x000000);
}
cursor ++;
}cursor=cursor1;
Now /= 10;
}
}



//пищалка и моргалка
void WarningStartDelay()
{
DateTime now = rtc.now();  
  
  unsigned long currentMillis = millis();

   if (mode==4){
            for (int i = 0; i < 4; i++) 
  { 
    strip.setPixelColor(i, strip.Color(0, 0, 255));
    strip.show();
  }} else {
  if ((currentMillis - previousMillis > 399) && (now.second()>=57)) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) 
      {
        for (int i = 0; i < 4; i++) 
  { 
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  } 
  strip.show();
     noTone(buz);
      ledState = HIGH;
    } else {
            for (int i = 0; i < 4; i++) 
  { 
    strip.setPixelColor(i, strip.Color(255, 255, 0));
  } 
  // Передаем цвета ленте. 
  strip.show();  
  digitalWrite(buz, HIGH);
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    Serial.println(ledState);
    Serial.println(now.second());
  }
  
  
  if (now.second()==0) 
    {
            for (int i = 0; i < 4; i++) 
  { 
    strip.setPixelColor(i, strip.Color(0, 255, 0));
  } 
  // Передаем цвета ленте. 
  strip.show();  
  digitalWrite(buz, HIGH);
    }

   if ((now.second()>0) && (now.second()<=56)){
            for (int i = 0; i < 4; i++) 
  { 
    strip.setPixelColor(i, strip.Color(255, 0, 0));
  } 
  // Передаем цвета ленте. 
  strip.show();  
  digitalWrite(buz, LOW);
  ledState = LOW;
  }}}

void nRF24L01()   
{
  DateTime now = rtc.now();
    int second =now.second();

  radio.write(&second, sizeof(second));
  
} 

//работа с кнопками
void ButtonControl() {
 Butt1.tick();
 Butt2.tick();
 Butt3.tick(); 
 if (Butt1.isClick()){ 
    mode++;
 Serial.println("Click");
 Serial.println(mode);
 }
  if (mode==1){
    Serial.println("Настройка яркости");
  if (Butt2.isClick()) {                                 // если кнопка была удержана (это для инкремента)
    ledLevel+=51;                                       // увеличивать яркость
    Serial.println(ledLevel);                              // для примера выведем в порт
    if (ledLevel>255){
      ledLevel=0;
    }
  strip_a.setBrightness(ledLevel);
  }
   if (Butt3.isClick()) {                                 // если кнопка была удержана (это для инкремента)
    ledLevel-=51;                                       // увеличивать яркость
    Serial.println(ledLevel);                              // для примера выведем в порт
    if (ledLevel<0){
      ledLevel=255;
    }
  strip_a.setBrightness(ledLevel);
  }
 }
 
 if (mode==2){
  Serial.println("Настройка цвета");
  if (Butt2.isClick()) {
    // если кнопка была удержана (это для инкремента)
    color++;                                            // увеличивать яркость
}                              // для примера выведем в порт

ledColor = ColorTable[color];

}
 if (mode==4){
DateTime now = rtc.now(); 
    int hour=now.hour();
    int minute=now.minute();
    int second =now.second();
      if (Butt2.isClick()){
        Serial.println("Настройка часов");
        if (hour== 23){hour=0;}
          else {hour += 1;};
        }
        if (Butt3.isClick()){
          Serial.println("Настройка минут");
          if (minute== 59){minute=0;}
          else {minute += 1;};
          };
    if (Butt2.isStep()){
          Serial.println("Сброс");
          minute=0;
          hour=0;
          };
    rtc.adjust(DateTime(0,0,0,hour,minute,0)); 
    
 }
  if (mode==3){
    Serial.println("Настройка режима");
    if (Butt2.isClick()){
        Serial.println("Часовой режим");
        setmode++;
        if (setmode>1){setmode=0;
        Serial.println("Стартовый режим");
        }
        }
  }
 if (mode>4){
  mode=1;
 }
 if (color>6){
  color=1;
 }
}

void brightnessAuto() {
  // Считываем значение с фоторезистора на аналоговом входе A0
  value = analogRead(SENSOR);
  
  // Полученные значения на аналоговом входе A0 делим на 4
  ledLevel = value / 4;
strip_a.setBrightness(ledLevel);  
Serial.println (ledLevel);
}

void loop() {
//nRF24L01();
ButtonControl();
//brightnessAuto();
if (setmode==0){
WarningStartDelay();
}
TimeToArray();// Get leds array with required configuration Получить массив светодиодов с требуемой конфигурацией
strip_a.show(); // Display leds array
  }
