#include <Arduino.h>

/*
test for git
default DWIN request 5A A5 04 83 10 01 01
  where 5A and A5 are headers, 04 number of bytes following, 83 request read data, 10 and 01 Vp address and 01 number of words expected
  actual poll request should look like 
  Serial1.write(0x5a, 0xa5, 0x04, 0x83, 0x10, 0x01, 0x01);
  OR
  Serial1.write(0x5a);
  Serial1.write(0xa5);
  Serial1.write(0x04);
  Serial1.write(0x83);
  Serial1.write(0x10);
  Serial1.write(0x01);
  Serial1.write(0x01);

Either of those solutions might throw an error if the byte being sent is 0x00, to go around that use prefix (byte), Serial1.write((byte)0x00);


*/

int left_pot_pin = A0;
int right_pot_pin = A1;

int r_led_pin = 9;
int y_led_pin = 8;
int g_led_pin = 7;

int but_1_pin = 5;
int but_2_pin = 4;
int but_3_pin = 3;
int but_4_pin = 2;

int leftpotval = 0;
int rightpotval = 0;
int but1state = 0;
int but2state = 0;
int but3state = 0;
int but4state = 0;

int resetbuttonstate = 0; //DWIN on-screen reset button
unsigned char Buffer[9]; //for receiving data from DWIN
int delaytime = 2;
int delaymillis = 16;
unsigned int reset_height_millis = 10000;
unsigned long startmillis = 0;
unsigned long currentmillis = 0;

// put function declarations here:
//int myFunction(int, int);


void setup() {
  Serial.begin(115200); //for PC debug (USB Serial)
  Serial1.begin(115200); //for DWIN comms (TX/RX pins Serial)
  startmillis = millis();

  pinMode(left_pot_pin, INPUT);
  pinMode(right_pot_pin, INPUT);

  pinMode(but_1_pin, INPUT_PULLUP);
  pinMode(but_2_pin, INPUT_PULLUP);
  pinMode(but_3_pin, INPUT_PULLUP);
  pinMode(but_4_pin, INPUT_PULLUP);

  pinMode(r_led_pin, OUTPUT);
  pinMode(y_led_pin, OUTPUT);
  pinMode(g_led_pin, OUTPUT);

  // put your setup code here, to run once:
  
}



void loop() {

leftpotval = analogRead(left_pot_pin);
rightpotval = analogRead(right_pot_pin);

but1state = !digitalRead(but_1_pin);
but2state = !digitalRead(but_2_pin);
but3state = !digitalRead(but_3_pin);
but4state = !digitalRead(but_4_pin);

if (resetbuttonstate == 1) {digitalWrite(r_led_pin, HIGH);}
if (resetbuttonstate == 0) {digitalWrite(r_led_pin, LOW);}

if (but1state == 1) {digitalWrite(r_led_pin, HIGH);}
if (but1state == 0) {digitalWrite(r_led_pin, LOW);}

if (but2state == 1) {digitalWrite(y_led_pin, HIGH);}
if (but2state == 0) {digitalWrite(y_led_pin, LOW);}

if (but3state == 1) {digitalWrite(g_led_pin, HIGH);}
if (but3state == 0) {digitalWrite(g_led_pin, LOW);}

if (but4state == 1) {digitalWrite(r_led_pin, LOW), digitalWrite(y_led_pin, LOW), digitalWrite(g_led_pin, LOW);}


currentmillis = millis();  //get the number of milliseconds since the program started
  
if ((currentmillis - startmillis >= reset_height_millis) && (resetbuttonstate == 1))  //test whether the period has elapsed
  { digitalWrite(r_led_pin, HIGH);
    
    /*
    //0x2002, reset the on-screen button to default and the arduino 
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
*/
  }

//=======write data to DWIN

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
  Serial1.write(map(rightpotval, 0, 1023, 0, 35)); // value
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

//request data from DWIN
  Serial1.write((byte)0x5a);  //header
  Serial1.write((byte)0xa5);  //header
  Serial1.write((byte)0x04);  //number of bytes in packet
  Serial1.write((byte)0x83);  //command to read/write
  Serial1.write((byte)0x20);  //address
  Serial1.write((byte)0x02);  //address
  Serial1.write((byte)0x01);  //number of words to return
 
if(Serial1.available())
  {
    for(int i=0;i<=8;i++)   //this loop will store whole frame in buffer array.
    {
    Buffer[i]= Serial1.read();
    }
    
    if(Buffer[0]==0x5A)
      {
        switch(Buffer[4])
        {
          case 0x20:   //variable adress?
            Serial.print(" TEST RETURN: "); Serial.print(Buffer[8]);
            if (Buffer[8] == 01) {resetbuttonstate = 1;}
            if (Buffer[8] == 00) {resetbuttonstate = 0; startmillis = currentmillis;}
            break;
        }
      }
  }
delay(delaytime);

//serial debug
Serial.print("   Startmillis: "); Serial.print(startmillis); Serial.print("  Currentmillis: "); Serial.print(currentmillis);

Serial.println("");
//end of loop
}



// put function definitions here:
/*
int myFunction(int x, int y) {
  return x + y;
}
*/