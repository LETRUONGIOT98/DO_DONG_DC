#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp32.h>
#define ACS_PIN_1 39  // Chân analog của cảm biến ACS712 cho thiết bị 1
#define ACS_PIN_2 34  // Chân analog của cảm biến ACS712 cho thiết bị 2
#define ACS_PIN_3 35  // Chân analog của cảm biến ACS712 cho thiết bị 3
#define VOLTAGE_PIN 32  // Chân analog của cảm biến áp

#define RELAY_1 26  // Chân kết nối relay cho thiết bị 1
#define RELAY_2 27 // Chân kết nối relay cho thiết bị 2
#define RELAY_3 14 // Chân kết nối relay cho thiết bị 3

char auth[] = "ipWvfP56X18ULHq2-okdj5-1jW1yCPpH";
char ssid[] = "Thu Thao";
char pass[] = "12052002";

WiFiUDP ntpUDP;
// Define NTP client
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7* 3600); // UTC+7

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Địa chỉ I2C của LCD và kích thước 16x2

const float maxCurrent = 1.5;  // Đặt giá trị dòng tối đa cho việc so sánh
float P1, P2, P3, U,TONG;
void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  pinMode(ACS_PIN_1, INPUT);
  pinMode(ACS_PIN_2, INPUT);
  pinMode(ACS_PIN_3, INPUT);
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  digitalWrite(RELAY_1, HIGH);
  digitalWrite(RELAY_2, HIGH);
  digitalWrite(RELAY_3, HIGH);
  Blynk.begin(auth, ssid, pass,"blynk-server.com",8080);
  timeClient.begin();
  timeClient.update();
}
int currentHour;
long kk;
void loop() {
  timeClient.update();
  // Get the hour from the tm structure
  currentHour = timeClient.getHours();
//  displayDateTime();
  float voltage = analogRead(VOLTAGE_PIN);
  voltage = (voltage/4095)*17; // Đo điện áp sử dụng cảm biến áp
  U = voltage;
  int value1 = analogRead(ACS_PIN_1);     // Ta sẽ đọc giá trị hiệu điện thế của cảm biến
  float  current_1 = (value1 / 4095.0) * 40.0;  // Bây giờ ta chỉ cần tính ra giá trị dòng điện
  current_1 = current_1 - 27.1;
  
  int value2 = analogRead(ACS_PIN_2);     // Ta sẽ đọc giá trị hiệu điện thế của cảm biến
  float  current_2 = (value2 / 4095.0) * 40.0;  // Bây giờ ta chỉ cần tính ra giá trị dòng điện
  current_2 = current_2 - 26.9 ;
  
  int value3 = analogRead(ACS_PIN_3);     // Ta sẽ đọc giá trị hiệu điện thế của cảm biến
  float  current_3 = (value3 / 4095.0) * 40.0;  // Bây giờ ta chỉ cần tính ra giá trị dòng điện
  current_3 = current_3 - 26.25;
  
  if(current_1 <= 0) current_1 = 0;
  if(current_2 <= 0) current_2 = 0;
  if(current_3 <= 0) current_3 = 0;
  P1 = current_1 * U;
  P2 = current_2 * U;
  P3 = current_3 * U;
  TONG = P1 + P2 + P3;
  Blynk.virtualWrite(V0,  U);
  Blynk.virtualWrite(V1,  current_1);
  Blynk.virtualWrite(V2,  current_2);
  Blynk.virtualWrite(V3,  current_3);
  Blynk.virtualWrite(V4,  P1);
  Blynk.virtualWrite(V5,  P2);
  Blynk.virtualWrite(V6,  P3);
  Blynk.virtualWrite(V7,  TONG);
  Serial.print(value1);
  Serial.print(" ");
  Serial.print(value2);
  Serial.print(" ");
  Serial.print(value3);
  Serial.println(" ");
  // Hiển thị dòng và điện áp lên LCD
  lcd.setCursor(0, 0);
  lcd.print("I1:     ");
  lcd.setCursor(3, 0);
  lcd.print(current_1, 1);

  lcd.setCursor(8, 0);
  lcd.print("I2:     ");
  lcd.setCursor(11, 0);
  lcd.print(current_2, 1);

  lcd.setCursor(0, 1);
  lcd.print("I3:     ");
  lcd.setCursor(3, 1);
  lcd.print(current_3, 1);

  lcd.setCursor(8, 1);
  lcd.print("U:      ");
  lcd.setCursor(11, 1);
  lcd.print(voltage, 1);
  
  Serial.println(currentHour);
  // Kiểm tra và ngắt thiết bị nếu dòng vượt quá giá trị đặt trong khoảng cao điểm
  if (currentHour >= 18 && currentHour < 23) {
    Serial.println("GIO CAO DIEM");
    if((current_1 + current_2 + current_3) >= maxCurrent){
      Serial.println("CONG SUAT DUNG QUA CAO TRONG GIO CAO DIEM");
      if((current_1 > current_2) && (current_1 > current_3)){
        digitalWrite(RELAY_1, LOW);
      }
      if((current_2 > current_3) && (current_2 > current_1)){
        digitalWrite(RELAY_2, LOW);
      }
      if((current_3 > current_2) && (current_3 > current_1)){
        digitalWrite(RELAY_3, LOW);
      }      
    }
  }

  delay(1000);  // Đợi 1 giây trước khi đọc lại
}
/*
void connectToWiFi() {
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}
/*
void displayDateTime() {
  lcd.clear();
  lcd.print("Time: ");
  lcd.print(timeClient.getFormattedTime());
}
*/
