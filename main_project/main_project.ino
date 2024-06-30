#include <Servo.h>

#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 9
#define DHTTYPE DHT11
 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd (0x27,16,2);

#include <SoftwareSerial.h>
#define rxPin 13
#define txPin 12
SoftwareSerial transfer_serial(rxPin, txPin);

DHT dht(DHTPIN, DHTTYPE);
Servo myservo;

//---------------------------------------------

int countnum = 0;
int onetimeIN ; //หมุนservo 1ครั้งของ interrupt
int onetimeAU ; //หมุนservo 1ครั้งของ Auto

int doorAU; //สำหรับการปิดกันสาดของ Auto Mode ที่เปิดกันสาดค้างเอาไว้
int doorINT; //สำหรับการปิดกันสาดของ Manual Mode ที่เปิดกันสาดค้างเอาไว้

unsigned long previous_time;

boolean stateser = false; //สำหรับเปิดกันสาดค้างเอาไว้ใน Manual mode

boolean stateserOA = false;
boolean stateserCA = true;

//1 คือ Au เเละ 0 คือinter
const int sw = 2;
const int sw2 = 10;

//---------------------------------------------

bool wait_until_sw_pressed() {
  int sw_in;
  sw_in = digitalRead(sw);
  if(sw_in == LOW){
    _delay_ms(100);
     sw_in = digitalRead(sw);
    if(sw_in == LOW){
      while(sw_in == LOW){
        sw_in = digitalRead(sw);
      }
      return true;
    }
  }
  return false;
}

void controservo() {
  if (wait_until_sw_pressed() == true) { 
    onetimeIN = 1;
    Serial.println("ISR");
    if (countnum <= 1) {
      countnum = 2;
    } else {
      countnum = countnum - 1;
    }
    stateser = true;
    return stateser;
  }
}

//---------------------------------------------

void setup() {
  Serial.begin(9600);
  Serial.println("Let go!");
  transfer_serial.begin(115200); // Set the baud rate for SoftwareSerial
  dht.begin();

  myservo.attach(8);

  pinMode(sw, INPUT_PULLUP);
  pinMode(sw2, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(sw), controservo, FALLING);

  previous_time = 0;

  lcd.init();
  lcd.backlight();
  lcd.clear();   
}

//---------------------------------------------

void loop() { 
  
  float t = dht.readTemperature();    // สำหรับการอ่านค่าอุณหภูมิเป็น
  transfer_serial.println(t);  

  
  int a = digitalRead(sw2);
  
  //a=0 คือ Manual Mode
  if (a == 0) {
    lcd.setCursor(0,0);
    lcd.print("Mode = Manual");

    lcd.setCursor(0,1);
    lcd.print("Tem = ");
    lcd.print(t);
    lcd.print(char(223)); // ใส่เครื่องหมาย องศา
    lcd.print("C ");
    Serial.println("Interrupt mode");
    
    //Auto เปิดกันสาดค้างเอาไว้
    if (doorAU == 1) {
      doorAU = 0;
      Serial.println("ปิดกันสาดของ Auto");
      myservo.write(180);
      delay(2300);
      myservo.write(90);
      onetimeAU = 0;
      stateserOA = false;
      stateserCA = true;
      return;
    }

    
    //autoปิดประตูเรียบร้อย
    else {
      Serial.println("Auto ปิดกันสาดเรียบร้อย");
      if (stateser == true) {
        if (onetimeIN == 1 ) {
          if (countnum == 2 ) {
            doorINT = 1;
            Serial.println("open int");
            myservo.write(-180);
            delay(2300);
            myservo.write(90);
            onetimeIN = 0;

          } else if(countnum == 1) {
            doorINT = 0;
            Serial.println("close int");
            myservo.write(180);
            delay(2300);
            myservo.write(90);

            stateser = !stateser;
            onetimeIN = 0;
            stateserOA = false;
            stateserCA = true;
            return ;
          }
        }
      }
    }
  }

  else{//a=1 คือ Auto Mode
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Mode = Auto");


    
    Serial.println("Auto mode");
    onetimeAU = 1;
    
    lcd.setCursor(0,1);
    lcd.print("Tem = ");
    lcd.print(t);
    lcd.print(char(223)); // ใส่เครื่องหมาย องศา
    lcd.print("C ");
    
    Serial.println("Auto mode");
    Serial.print(t);
    Serial.println(" *C   ");

    //เวลาผ่านไป1วินาที
    if (millis() - previous_time > 1000) {
      previous_time = millis();
      
    //Manual เปิดกันสาดค้างเอาไว้  
    if (doorINT == 1){
      doorINT = 0;
      Serial.println("ปิดกันสาดของ Manual");
      myservo.write(180);
      delay(2300);
      myservo.write(90);
      onetimeAU = 0;
      doorAU = 0;
      countnum = 0;
      return;
    }
    
      if (onetimeAU == 1 ) {
        if (t >= 28 ) {
          //ไม่เคยเปิดเลย = false ถ้าเคยเปิดเเล้ว = true
          if (stateserOA == false ) {
            doorAU = 1;
            Serial.println("open Auto");
            myservo.write(-180);
            delay(2300);
            myservo.write(90);
            onetimeAU = 0;
            
            //กลับค่าให้เป็น true
            stateserOA = !stateserOA;
          }
          
          //เมื่อเคยเปิดเเล้ว ถึงสามารถปิดใหม่ได้อีกครั้ง
          stateserCA = false;
          return;
        }
        
        else if (t <= 27) {
          //ไม่เคยปิดเลย = false ถ้าเคยปิดเเล้ว = true
          if (stateserCA == false ) {
            doorAU = 0;
            Serial.println("close Auto");
            myservo.write(180);
            delay(2300);
            myservo.write(90);
            onetimeAU = 0;

            //กลับค่าให้เป็น true
            stateserCA = !stateserCA;

          }
          
          //เมื่อเคยปิดเเล้ว ถึงสามารถเปิดใหม่ได้อีกครั้ง
          countnum = 0;
          stateserOA = false;
          return;
        }
        
      }
    }
    
  }
}
