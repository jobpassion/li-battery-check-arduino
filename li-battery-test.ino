#include <U8g2lib.h>

#define MOSFET_Pin A2
#define Bat_Pin A0
#define Res_Pin A1

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* SCL=*/ A5, /* SDA=*/ A4, /* RST=*/ U8X8_PIN_NONE);



float Capacity = 0.0; 
float Res_Value = 10.1;  
float Vcc = 4.79; //Measure it and change  
float Current = 0.0; 
float mA=0;         
float Bat_Volt = 0.0;  
float Res_Volt = 0.018;  
unsigned long previousMillis = 0; 
unsigned long millisPassed = 0;  
float sample1 =0.000;
float sample2= 0.000;
int x = 0;
int row = 0;
int printTime = 0;
int newBatt = 0;
float inner_res = 0;
float inner_res_total = 0;
int battery_end = 0;

void draw(void) {

  int isPrint = 0;
  if(printTime % 3 == 0){
    isPrint = 1;
  }

  u8g2.setFont(u8g2_font_ncenB14_tr);    

  if ( Bat_Volt < 1.5){
    u8g2.drawStr(10,40,"No Battery");
  }
  else if ( Bat_Volt > 4.3){
    u8g2.drawStr(3,40,"High Voltage");
    if(isPrint){
      Serial.println( "High Voltage");
    }
  }
  else if(Bat_Volt < 2.9 && Capacity == 0655){
    u8g2.drawStr(3,40,"Low Voltage");
  }
  else{
    u8g2.setFont(u8g2_font_ncenB10_tr);    
    u8g2.setCursor(0,20);   
    u8g2.print("V: ");
    u8g2.print(Bat_Volt,2);
    u8g2.print("V");
    u8g2.setCursor(0,40);   
    u8g2.print("I: ");
    u8g2.print(mA,0);
    u8g2.print("mA");
    u8g2.setCursor(0,60);   
    u8g2.print("mAh: ");
    u8g2.print(Capacity ,1);

    if(inner_res < 100){
      u8g2.setCursor(80,20);   
    }else{
      u8g2.setCursor(70,20);   
    }

    u8g2.print(inner_res, 0);
    u8g2.print("mR");
    u8g2.setCursor(70,40); 
    u8g2.print("vcc:");
    u8g2.print(Vcc, 2);
  }
}
void buz()
{
  digitalWrite(9, HIGH);
  delay(100);          
  digitalWrite(9, LOW);  
  delay(10);            

}   
double getArduinoVcc(double expected_volt)
{
  double received_frac = (double)analogRead(A3) / 1023.0;    
  double expected_frac = (expected_volt / 5.0);  
  return (expected_frac / received_frac) * 5.0; // == vcc
}
void setup() {
  Serial.begin(9600);
  pinMode(MOSFET_Pin, OUTPUT);
  digitalWrite(MOSFET_Pin, LOW); 
  pinMode(LED_BUILTIN, OUTPUT);
  u8g2.begin();
}

void loop() {
  Vcc = getArduinoVcc(3.281);
  sample1=0;
  for(int i=0;i< 100;i++)
  {
    sample1=sample1+analogRead(Bat_Pin);
    delay (2);
  }
  sample1=sample1/100; 

  Bat_Volt = sample1 *Vcc/ 1023.0; 

  if ( Bat_Volt > 4.3){
    digitalWrite(MOSFET_Pin, LOW); 
    buz();
    Serial.println( String("Warning High-V! ") + String(Bat_Volt, 2));
    delay(1000);
    newBatt = 0;
  }
  else if(Bat_Volt < 1.5){
    Capacity = 0;
    battery_end = 0;
    newBatt = 0;
    previousMillis = 0;
    inner_res_total = 0;
    inner_res = 0;
  }
  else if(Bat_Volt < 2.9 || battery_end == 1){
    digitalWrite(MOSFET_Pin, LOW);
    buz();
    delay(1000);
    mA = 0;
    battery_end = 1;
  }
  else if(Bat_Volt > 2.9 && Bat_Volt < 4.3  ) { 
    millisPassed = millis() - previousMillis;
    if(previousMillis > 0){
      Current = (Bat_Volt - Res_Volt) / Res_Value;
      mA = Current * 1000.0 ;
      Capacity = Capacity + mA * (millisPassed / 3600000.0);
      if(newBatt < 10){
        digitalWrite(MOSFET_Pin, LOW);
        delay (2);
        sample1 = 0;
        for(int i=0;i< 100;i++)
        {
          delay (2);
          sample1=sample1+analogRead(Bat_Pin);
        }
        sample1=sample1/100; 


        float Bat_Volt_without = sample1 *Vcc/ 1023.0; 

        float curr_inner_res = 1000 * (Bat_Volt_without - Bat_Volt) / Current - 90;
        if(curr_inner_res <= 0){
        }else{
          inner_res_total += curr_inner_res;
          newBatt++;
          inner_res = inner_res_total / newBatt;
        }
      } 
    }
    digitalWrite(MOSFET_Pin, HIGH);
    previousMillis = millis();
    row++;
    x++;
    delay(100); 
    if(printTime % 2 < 1){
      digitalWrite(LED_BUILTIN, HIGH);
    }else{
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
  u8g2.firstPage();  
  do {
    draw();
  } while( u8g2.nextPage() );

  printTime++;
  if(printTime >= 1000){
    printTime = 0;
  }
}    
