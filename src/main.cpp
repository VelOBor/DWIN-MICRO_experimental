#include <Arduino.h>

/*
Иконки и их адреса в DWIN:
Буровой насос - 1002, его замочек 1006
Шаровый кран - 1003, его замочек 1007
Манипулятор - 1004
Нижний захват - 1005
Скоростиметр - 1000
Высотометр - 1001
Алярма - 1008

Тормоз лебёдки - 2001 (это КНОПКА или ИНДИКАТОР?)
Резет высоты - 2002

Запрос на  DWIN по умолчанию: 5A A5 04 83 10 01 01
  где 5A и A5 заголовок, 04 количество байт далее, 83 запрос отправить данные из адреса Vp 10 01 Vp и 01 количество запрашиваемых слов (слово 2 байта)
  по факту запрос выглядит так: 
  Serial1.write(0x5a, 0xa5, 0x04, 0x83, 0x10, 0x01, 0x01);
  или так:
  Serial1.write(0x5a);  //заголовок
  Serial1.write(0xa5);  //заголовок
  Serial1.write(0x04);  //количество следующих байт
  Serial1.write(0x83);  //команда на чтение данных
  Serial1.write(0x10);  //адрес
  Serial1.write(0x01);  //адрес
  Serial1.write(0x01);  //количество запрашиваемых слов

Любой из представленных выше вариантов может дать ошибку при компиляции если отправляется 0x00
Чтобы обойти это можно ставить префикс (byte), как Serial1.write((byte)0x00);
*/

//==========ОПРЕДЕЛЕНИЯ ПИНОВ==========
int left_pot_pin = A0;  //левый потенциометр, заменится переменной считающей скорость движения блока
int right_pot_pin = A1; //правый потенциометр, заменится переменной считающей положение (высоту) блока

int hall_1_in = 14; //вход первого датчика холла (пин 2 наны в экспериментальном режиме)
int hall_2_in = 15; //вход второго датчика холла (пин 3 наны в экспериментальном режиме)

int r_led_pin = 9;  //красный светодиод, по факту будет сброс высоты блока после удержания кнопки резет на экране
int y_led_pin = 8;  //жёлтый светодиод, по факту будет реле включения алярмы
int g_led_pin = 7;  //зелёный светодиод, по факту будет реле разблокировки тормозов барабана

int but_1_pin = 5;  //кнопка 1, по факту датчик состояния бурового насоса (ВКЛ-ВЫКЛ)
int but_2_pin = 4;  //кнопка 2, по факту датчик состояния шарового крана (ОТКР-ЗАКР)
int but_3_pin = 3;  //кнопка 3, по факту датчик состояния манипулятора (ОТКР-ЗАКР)
int but_4_pin = 2;  //кнопка 4, по факту датчик состояния нижнего захвата (ОТКР-ЗАКР)

//==========ОПРЕДЕЛЕНИЯ ПЕРЕМЕННЫХ СОСТОЯНИЯ==========
int leftpotval = 0;
int rightpotval = 0;
int but1state = 0;
int but2state = 0;
int but3state = 0;
int but4state = 0;

int hall_1_state = 0;
int hall_1_ontime = 0;
int hall_1_offtime = 0;
int hall_2_state = 0;
int hall_2_ontime = 0;
int hall_2_offtime = 0;
int freq = 0;
int period = 0;
int duty = 0;

//==========ОПРЕДЕЛЕНИЯ ПЕРЕМЕННЫХ==========
int resetbuttonstate = 0; //кнопка сброса высоты на экране DWIN
unsigned char Buffer[9];  //массив для получения данных с экрана DWIN, возможно придётся изменить размер когда запрашивать больше чем одно значение
int delaytime = 2;  //время паузы между отправлениями для более стабильной работы
//int delaymillis = 16; //время задержки для таймера
unsigned int reset_height_millis = 10000; //время удержания кнопки резет прежде чем сбросится высота
unsigned long startmillis = 0;    //время при запуске таймера
unsigned long currentmillis = 0;  //текущее время таймера

//put function declarations here:
//int myFunction(int, int);


void setup() {
  Serial.begin(115200);   //для дебага на ПК (USB Serial)
  Serial1.begin(115200);  //для обмена данными с экраном DWIN (пины TX/RX)
  startmillis = millis(); //инициализация таймера

//==========ИНИЦИАЛИЗАЦИЯ ПИНОВ==========
  pinMode(left_pot_pin, INPUT);
  pinMode(right_pot_pin, INPUT);

  pinMode(hall_1_in, INPUT);
  pinMode(hall_2_in, INPUT);

  pinMode(but_1_pin, INPUT_PULLUP);
  pinMode(but_2_pin, INPUT_PULLUP);
  pinMode(but_3_pin, INPUT_PULLUP);
  pinMode(but_4_pin, INPUT_PULLUP);

  pinMode(r_led_pin, OUTPUT);
  pinMode(y_led_pin, OUTPUT);
  pinMode(g_led_pin, OUTPUT);

}



void loop() {

//==========ЧТЕНИЕ ПИНОВ==========
leftpotval = analogRead(left_pot_pin);
rightpotval = analogRead(right_pot_pin);

hall_1_state = digitalRead(hall_1_in);
hall_2_state = digitalRead(hall_2_in);

but1state = !digitalRead(but_1_pin);
but2state = !digitalRead(but_2_pin);
but3state = !digitalRead(but_3_pin);
but4state = !digitalRead(but_4_pin);

//==========ВКЛ-ВЫКЛ ДИОД ДЛЯ ПРОВЕРКИ, ПО ФАКТУ СБРОС ВЫСОТЫ==========
if (resetbuttonstate == 1) {digitalWrite(r_led_pin, HIGH);}  //флажок кнопки резет получаем с экрана, смотри код ниже
if (resetbuttonstate == 0) {digitalWrite(r_led_pin, LOW);}

//==========ВКЛ-ВЫКЛ ДИОДЫ ДЛЯ ПРОВЕРКИ ВХОДОВ С ДАТЧИКОВ ХОЛЛА (СИМУЛЯЦИЯ)==========
if (hall_1_state == 1) {digitalWrite(r_led_pin, HIGH);}
if (hall_1_state == 0) {digitalWrite(r_led_pin, LOW);}

if (hall_2_state == 1) {digitalWrite(y_led_pin, HIGH);}
if (hall_2_state == 0) {digitalWrite(y_led_pin, LOW);}

//==========РАСЧЁТ ЧАСТОТЫ ДАТЧИКОВ ХОЛЛА 1 (СИМУЛЯЦИЯ)==========
hall_1_ontime = pulseIn(hall_1_in, HIGH);
hall_1_offtime = pulseIn(hall_1_in, LOW);
period = hall_1_ontime+hall_1_offtime;
freq =  1000000.0/period;
duty = (hall_1_ontime/period)*100;

//==========ВКЛ-ВЫКЛ ДИОДЫ ДЛЯ ПРОВЕРКИ КНОПОК==========
/*
if (but1state == 1) {digitalWrite(r_led_pin, HIGH);}
if (but1state == 0) {digitalWrite(r_led_pin, LOW);}

if (but2state == 1) {digitalWrite(y_led_pin, HIGH);}
if (but2state == 0) {digitalWrite(y_led_pin, LOW);}

if (but3state == 1) {digitalWrite(g_led_pin, HIGH);}
if (but3state == 0) {digitalWrite(g_led_pin, LOW);}
*/
if (but4state == 1) {digitalWrite(r_led_pin, LOW), digitalWrite(y_led_pin, LOW), digitalWrite(g_led_pin, LOW);} //на всякий случай если залипнет диод, чтобы можно было выключить програмно



//==========ПРОВЕРКА УДЕРЖАНИЯ КНОПКИ СБРОСА ВЫСОТЫ==========
currentmillis = millis(); //получение текущего времени с начала запуска МК
  
if ((currentmillis - startmillis >= reset_height_millis) && (resetbuttonstate == 1))  //проверка прошедшего периода времени И состояния кнопки резет
  {digitalWrite(r_led_pin, HIGH);}  //включение диода, по факту обнуление высоты
    
    /*\\\\НЕ НУЖНО////
    //0x2002, сброс кнопки на экране в исходное состояние 
  Serial1.write((byte)0x5a); // header
  Serial1.write((byte)0xa5); // header
  Serial1.write((byte)0x05); // number of bytes being send
  Serial1.write((byte)0x82); // send/set VP  
  Serial1.write((byte)0x20); // address
  Serial1.write((byte)0x02); // address
  Serial1.write((byte)0x00); // value
  Serial1.write((byte)0x00); // value
delay(delaytime);  
    startmillis = currentmillis;  //restart the timer
  resetbuttonstate = 0;
////НЕ НУЖНО\\\\\*/  

//==========ОТПРАВКА ДАННЫХ НА ЭКРАН DWIN==========

//0x1000, left pot value, simulated block speed
  Serial1.write((byte)0x5a); // header
  Serial1.write((byte)0xa5); // header
  Serial1.write((byte)0x05); // number of bytes being send
  Serial1.write((byte)0x82); // send/set VP  
  Serial1.write((byte)0x10); // address
  Serial1.write((byte)0x00); // address
  Serial1.write((byte)0x00); // value
  Serial1.write(map(leftpotval, 0, 1023, 0, 200)); // value
delay(delaytime);
  //0x1001, right pot value, simulated block height indicator
  Serial1.write((byte)0x5a); // header
  Serial1.write((byte)0xa5); // header
  Serial1.write((byte)0x05); // number of bytes being send
  Serial1.write((byte)0x82); // send/set VP  
  Serial1.write((byte)0x10); // address
  Serial1.write((byte)0x01); // address
  Serial1.write((byte)0x00); // value
  Serial1.write(map(rightpotval, 0, 1023, 0, 250)); // value
delay(delaytime);
  //0x1002, button 1, simulated pumps state indicator
  Serial1.write((byte)0x5a); // header
  Serial1.write((byte)0xa5); // header
  Serial1.write((byte)0x05); // number of bytes being send
  Serial1.write((byte)0x82); // send/set VP  
  Serial1.write((byte)0x10); // address
  Serial1.write((byte)0x02); // address
  Serial1.write((byte)0x00); // value
  Serial1.write(but1state); // value
delay(delaytime);
  //0x1003, button 2, simulated ball valve state indicator
  Serial1.write((byte)0x5a); // header
  Serial1.write((byte)0xa5); // header
  Serial1.write((byte)0x05); // number of bytes being send
  Serial1.write((byte)0x82); // send/set VP  
  Serial1.write((byte)0x10); // address
  Serial1.write((byte)0x03); // address
  Serial1.write((byte)0x00); // value
  Serial1.write(but2state); // value
delay(delaytime);
  //0x1004, button 3, simulated pipe clamp state indicator
  Serial1.write((byte)0x5a); // header
  Serial1.write((byte)0xa5); // header
  Serial1.write((byte)0x05); // number of bytes being send
  Serial1.write((byte)0x82); // send/set VP  
  Serial1.write((byte)0x10); // address
  Serial1.write((byte)0x04); // address
  Serial1.write((byte)0x00); // value
  Serial1.write(but3state); // value
delay(delaytime);
  //0x1005, button 4, simulated lower clamp indicator
  Serial1.write((byte)0x5a); // header
  Serial1.write((byte)0xa5); // header
  Serial1.write((byte)0x05); // number of bytes being send
  Serial1.write((byte)0x82); // send/set VP  
  Serial1.write((byte)0x10); // address
  Serial1.write((byte)0x05); // address
  Serial1.write((byte)0x00); // value
  Serial1.write(but4state); // value
delay(delaytime);

//отправление запроса на получение данных с экрана DWIN
  Serial1.write((byte)0x5a);  //header
  Serial1.write((byte)0xa5);  //header
  Serial1.write((byte)0x04);  //number of bytes in packet
  Serial1.write((byte)0x83);  //command to read/write
  Serial1.write((byte)0x20);  //address
  Serial1.write((byte)0x02);  //address
  Serial1.write((byte)0x01);  //number of words to return


if(Serial1.available()) //чтение
  {
    for(int i=0;i<=8;i++) //сохранение полученных данных в буфер
    {
    Buffer[i]= Serial1.read();
    }
    
    if(Buffer[0]==0x5A) //парсинг заголовка пакета
      {
        switch(Buffer[4]) //если в 4ой ячейке буфера...
        {
          case 0x20:  //...находится искомое число, являющееся адресом искомой переменной (из запроса выше)...
            //Serial.print(" TEST RETURN: "); Serial.print(Buffer[8]);  //выводим на дебаг порт
            if (Buffer[8] == 01) {resetbuttonstate = 1;}  //ставим флажок на удержание кнопки резет
            if (Buffer[8] == 00) {resetbuttonstate = 0; startmillis = currentmillis;} //и начинаем отсчёт прежде чем обнулить высоту
            break;
        }
      }
  }
delay(delaytime);

//дебаг на ПК
//Serial.print(" Start-ms: "); Serial.print(startmillis); Serial.print("  Curr-ms: "); Serial.print(currentmillis);
Serial.print(" h1Ton: "); Serial.print(hall_1_ontime); Serial.print(" h1Toff: "); Serial.print(hall_1_offtime); 
Serial.print(" Freq: "); Serial.print(freq); 
Serial.print(" Duty: "); Serial.print(duty);
Serial.print(" Period: "); Serial.print(period);
Serial.println("");
//end of loop
}



// put function definitions here:
/*
int myFunction(int x, int y) {
  return x + y;
}
*/