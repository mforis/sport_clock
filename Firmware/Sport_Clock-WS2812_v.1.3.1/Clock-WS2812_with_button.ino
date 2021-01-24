// Developed by MAX_FORYS and SerZheka
// 20.01.2021
// Ver. 1.3.2

// TODO сделать массив цветов через mData
// TODO сделать цифры через битовые поля

#include <SPI.h>
//#include <nRF24L01.h>
#include <printf.h>
//#include <RF24.h>
//#include <RF24_config.h>
#include <GyverButton.h>
#include <RTClib.h>
#include <microLED.h>

#define SENSOR A0           // Фоторезистор
#define RADIOTRANSMITION 0  // 0 - выключить передачу, 1 - включить
#define PIPE 0xF0F1F2F3F4LL // индитификатор передачи, "труба"
#define NUM_LEDS 174        // 5 by segment + 6 in the middle
#define PIXEL_SEGMENT 29    // количество светодиодов в цифре
#define PIN 6               // Data pin for led comunication
#define SMALL_LEDS_PIN 4
#define BUZZ 2
#define AUTOBRIGHTNESS 0
#define DEBUG 1 // 0 - без вывода в консоль, 1 - с выводом
#define LED_TYPE LED_WS2812
#define COLOR_ORDER ORDER_GRB
#define NUM_COLORS 7 // Число цветов в ColorTable

const byte digits[10][29] = {
    {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
    {1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1}};

const long ColorTable[NUM_COLORS]{
    0xFF0000,
    0xFFFF00,
    0x008000,
    0x00FFFF,
    0x0000FF,
    0x800080
    };

//RF24 radio(5, 3);
GButton Butt1(8);
GButton Butt2(10);
GButton Butt3(9);
RTC_DS3231 rtc;
unsigned long previousMillis = 0;
int ledState = LOW;
int ledLevel = 51;
int mode = 1;    // Текущий режим настройки часов
int color = 0;   // Текущая яркость часов
int setmode = 0; // Режим работы (0 - соревновательный режим, 1 - режим часов)
long ledColor = 0xFF0000;
microLED<NUM_LEDS, PIN, MLED_NO_CLOCK, LED_TYPE, COLOR_ORDER, CLI_AVER> mainLEDs;
microLED<4, SMALL_LEDS_PIN, MLED_NO_CLOCK, LED_TYPE, COLOR_ORDER, CLI_AVER> smallLEDs;

void setup()
{
/*#if (RADIOTRANSMITION)
    radio.begin();
    delay(2);
    radio.setChannel(9);
    // скорость, RF24_250KBPS, RF24_1MBPS или RF24_2MBPS (RF24_250KBPS на nRF24L01 (без +) неработает).
    // меньше скорость, выше чувствительность приемника.
    radio.setDataRate(RF24_1MBPS);
    // мощность передатчика, RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBM,
    radio.setPALevel(RF24_PA_HIGH); // TODO есть еще RF24_PA_MAX, знаешь?
    radio.openWritingPipe(PIPE);
#endif*/

    smallLEDs.setBrightness(255);
    smallLEDs.clear();
    smallLEDs.show();

    mainLEDs.setBrightness(ledLevel);
    mainLEDs.clear();
    mainLEDs.show();

    Butt1.setDebounce(80);      // настройка антидребезга
    Butt1.setClickTimeout(300); // настройка таймаута между кликами
    Butt2.setDebounce(80);
    Butt2.setClickTimeout(300);
    Butt3.setDebounce(80);
    Butt3.setClickTimeout(300);

#if (DEBUG)
    Serial.begin(9600);
#endif
    rtc.begin();
    pinMode(BUZZ, OUTPUT);
#if (AUTOBRIGHTNESS)
    pinMode(SENSOR, INPUT);
#endif
}

void TimeToArray()
{
    DateTime now = rtc.now();
    displayLed(now.second(), 145, 116);
    displayLed(now.minute(), 87, 58); // вызов функции и передача значени часов и минут. Такой способ сократил затрачиваемую общую память.
    displayLed(now.hour(), 29, 0);
}

// Now - двухзначное число которое надо вывести,
// cursor - последнее значение двух-го числа
// cursor1 - первое значение двух-го числа
void displayLed(int Now, int cursor, int cursor1)
{
    for (int i = 0; i < 2; i++)
    {
        int digit = Now % 10;
        for (int k = 0; k < PIXEL_SEGMENT; k++)
        {
            if (digits[digit][k])
                mainLEDs.leds[cursor] = mHEX(ledColor);
            else
                mainLEDs.leds[cursor] = mRGB(0, 0, 0);
            cursor++;
        }
        cursor = cursor1;
        Now /= 10;
    }
}

// пищалка и моргалка
void WarningStartDelay()
{
    DateTime now = rtc.now();
    unsigned long currentMillis = millis();

    if (mode == 4)
    {
        for (int i = 0; i < 4; i++)
        {
            smallLEDs.leds[i] = mRGB(0, 0, 255);
        }
        smallLEDs.show();
        return;
    }

    if (now.second() == 0)
    {
        tone(BUZZ, 3000, 1000);
        //digitalWrite(BUZZ, HIGH);
        for (int i = 0; i < 4; i++)
            smallLEDs.leds[i] = mRGB(0, 255, 0);
        ledState = LOW;
    }

    // TODO Заменил на следующий if, если перестанет работать пищалка с 1 по 56 секунду - поменять
     /*if ((now.second() > 0) && (now.second() <= 56))
     {
         for (int i = 0; i < 4; i++)
             // strip.setPixelColor(i, strip.Color(255, 0, 0));
             smallLEDs.set(i, mRGB(255, 0, 0));
         // strip.show();
         smallLEDs.show();
         digitalWrite(BUZZ, LOW);
     }*/

    else if (now.second() == 1)
    {
        for (int i = 0; i < 4; i++)
            smallLEDs.leds[i] = mRGB(255, 0, 0);
        noTone(BUZZ);
        //digitalWrite(BUZZ, LOW);
    }
    else if ((currentMillis - previousMillis > 499) && (now.second() > 56))
    {
        previousMillis = currentMillis;
        // if the LED is off turn it on and vice-versa:
        if (ledState == LOW)
        {
            for (int i = 0; i < 4; i++)
                smallLEDs.leds[i] = mRGB(0, 0, 0);
            //noTone(BUZZ);
            digitalWrite(BUZZ, LOW);
            ledState = HIGH;
        }
        else
        {
            for (int i = 0; i < 4; i++)
                smallLEDs.leds[i] = mRGB(255, 255, 0);
            //tone(BUZZ, 3000, 250);
            digitalWrite(BUZZ, HIGH);
            ledState = LOW;
        }
#if (DEBUG)
        Serial.print("Текущее состояние светодиода: ");
        Serial.println(ledState);
        Serial.print("Текущая секунда: ");
        Serial.println(now.second());
#endif
    }
    smallLEDs.show();
}

/*void nRF24L01()
{
    DateTime now = rtc.now();
    int second = now.second();
    radio.write(&second, sizeof(second));
}*/

// работа с кнопками
void ButtonControl()
{
    Butt1.tick();
    Butt2.tick();
    Butt3.tick();

    if (Butt1.isClick())
    {
        if (mode >= 4)
            mode = 1;
        else
            mode++;
#if (DEBUG)
        Serial.print("Click ");
        Serial.println(mode);
#endif
        return;
    }

    DateTime now = rtc.now();
    int hour = now.hour();
    int minute = now.minute();

    switch (mode)
    {
    case 1:
#if (DEBUG)
        Serial.println("Настройка яркости");
#endif
        // если кнопка была удержана (это для инкремента яркости)
        if (Butt2.isClick())
        {
            ledLevel += 51;
            if (ledLevel > 255)
                ledLevel = 0;
            mainLEDs.setBrightness(ledLevel);
        }
        // если кнопка была удержана (это для декремента яркости)
        if (Butt3.isClick())
        {
            ledLevel -= 51;
            if (ledLevel < 0)
                ledLevel = 255;
            mainLEDs.setBrightness(ledLevel);
        }
#if (DEBUG)
        Serial.print("Текущая яркость: ");
        Serial.println(ledLevel);
#endif
        break;

    case 2:
#if (DEBUG)
        Serial.println("Настройка цвета");
#endif
        if (Butt2.isClick())
            if (color >= NUM_COLORS - 1)
                color = 0;
            else
                color++;
        ledColor = ColorTable[color];
        break;

    case 3:
#if (DEBUG)
        Serial.println("Настройка режима");
#endif
        if (Butt2.isClick())
        {
            if (setmode)
            {
                setmode = 0;
#if (DEBUG)
                Serial.println("Стартовый режим");
#endif
            }
            else
            {
                setmode = 1;
#if (DEBUG)
                Serial.println("Режим часов");
#endif
            }
        }
        break;

    case 4:
        if (Butt2.isClick())
        {
#if (DEBUG)
            Serial.println("Настройка часов");
#endif
            if (hour == 23)
                hour = 0;
            else
                hour += 1;
        }
        else if (Butt3.isClick())
        {
#if (DEBUG)
            Serial.println("Настройка минут");
#endif
            if (minute == 59)
                minute = 0;
            else
                minute += 1;
        }
        else if (Butt2.isStep())
        {
#if (DEBUG)
            Serial.println("Сброс");
#endif
            minute = 0;
            hour = 0;
        }
        rtc.adjust(DateTime(0, 0, 0, hour, minute, 0));
        break;

    default:
#if (DEBUG)
        Serial.println("Ошибка настройки часов");
#endif
        break;
    }
}

void brightnessAuto()
{
    ledLevel = analogRead(SENSOR) / 4; // Полученные значения на аналоговом входе A0 делим на 4
    mainLEDs.setBrightness(ledLevel);
#if (DEBUG)
    Serial.print("Яркость после автонастройки ");
    Serial.println(ledLevel);
#endif
}

void loop()
{
/*#if (RADIOTRANSMITION)
    nRF24L01();
#endif*/
    ButtonControl();
#if (AUTOBRIGHTNESS)
    brightnessAuto(); // TODO доделать автояркость
#endif
    if (!setmode)
        WarningStartDelay();
    else
    {
        for (int i = 0; i < 4; i++)
            smallLEDs.leds[i] = (mode == 4 ? mRGB(0, 0, 255) : mRGB(0, 0, 0));
        smallLEDs.show();
    }
    TimeToArray();
    mainLEDs.show();
    delay(1);
}
