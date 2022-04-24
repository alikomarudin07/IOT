#include "src/OV2640.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include "esp_camera.h"
#include "esp_system.h"

hw_timer_t *timer = NULL;
void IRAM_ATTR resetModule(){
    ets_printf("reboot\n");
    esp_restart();
}


#include <TridentTD_LineNotify.h>
#define SSID        "Redmi S2"
#define PASSWORD    "8sampai1"
#define LINE_TOKEN  "sgFMKn7IOrzFXGJCY5yqNdUwpeSNkX8M5Y296u5q0PH"

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

//Your Sensor Pins
const int trigPin = 15;
const int echoPin = 14;
const int Led_Flash = 4;
const int Led_run = 13;
int PIR_Sensor = 12;
const int buzzer = 27;

#define buzzer 27


boolean startTimer = false;
unsigned long time_now = 0;
int pinSensor = 0 ,Cam_capture = 0 ,time_capture=0;
long duration;
int distance;


void setup()
{
  Serial.begin(115200);
   while (!Serial) {  ;  }
  pinMode(Led_Flash, OUTPUT);
  pinMode(Led_run, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);
  WiFi.begin(SSID, PASSWORD);
  Serial.printf("WiFi connecting to %s\n",  SSID);
  while(WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(400); }
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());  
  LINE.setToken(LINE_TOKEN);
  
    timer = timerBegin(0, 80, true); //timer 0, div 80Mhz 
    timerAttachInterrupt(timer, &resetModule, true);
    timerAlarmWrite(timer, 20000000, false);
    timerAlarmEnable(timer); //enable interrupt

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
// FRAMESIZE_ +
//QQVGA/160x120//QQVGA2/128x160//QCIF/176x144//HQVGA/240x176
//QVGA/320x240//CIF/400x296//VGA/640x480//SVGA/800x600//XGA/1024x768
//SXGA/1280x1024//UXGA/1600x1200//QXGA/2048*1536
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QQVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }



  
}

void loop()
{

    timerWrite(timer, 0); //reset timer (feed watchdog)
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance= duration*0.034/2;
    Serial.print("Distance: ");
    Serial.println(distance);


    long tme = millis();
    if(Cam_capture == 1)
       Camera_capture();
    if(digitalRead(PIR_Sensor) == 1 && startTimer != true && distance <= 30){
           Camera_capture();
           startTimer = true;
           digitalWrite(buzzer, HIGH);
    delay(1000); 
    digitalWrite(buzzer, HIGH);
    delay(1000); 
    digitalWrite(buzzer, HIGH);
    delay(1000);
    Serial.print ("Jarak Alat Dari Objek : ");
    Serial.print (distance);
    Serial.println (" cm");
    Serial.println ("BAHAYA");
    LINE.notify("Jarak Alat Dari Objek:  "+ String(distance)  +  " Cm (BAHAYA)");


    
      }else if(digitalRead(PIR_Sensor) == 1 && distance >= 70){
        startTimer = false;
        time_capture=0;
    digitalWrite(buzzer, LOW);
    delay(300); 
    digitalWrite(buzzer, LOW);
    delay(300); 
    digitalWrite(buzzer, LOW);
    delay(300); 
    Serial.print ("Jarak Alat Dari Objek : ");
    Serial.print (distance);
    Serial.println (" cm");
    Serial.println ("AMAN");
    LINE.notify("Jarak Alat Dari Objek:  "+ String(distance)  +  " Cm (AMAN)");

    
      }
    if(digitalRead(PIR_Sensor) == 1 && distance < 70 && distance >= 30 ){
         if(++time_capture > 4){
            time_capture=0;
            Camera_capture();
            Serial.println("Over Time");digitalWrite(buzzer, LOW);
    delay(300); 
    digitalWrite(buzzer, HIGH);
    delay(300); 
    digitalWrite(buzzer, LOW);
    delay(300);
    Serial.print ("Jarak Alat Dari Objek : ");
    Serial.print (distance);
    Serial.println (" cm");
    Serial.println ("HATI_HATI");
    LINE.notify("Jarak Alat Dari Objek:  "+ String(distance)  +  " Cm (HATI-HATI)");
    
            
          }
        }
      Serial.println(digitalRead(PIR_Sensor));
      delay(500);
}

void Camera_capture() {
   digitalWrite(Led_Flash, LOW);
  delay(300); 
  digitalWrite(Led_Flash,HIGH );
  delay(300);
  digitalWrite(Led_Flash, LOW);
  camera_fb_t * fb = NULL;
  delay(200); 
  // Take Picture with Camera
  fb = esp_camera_fb_get(); 
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  
   Send_line(fb->buf,fb->len);
   esp_camera_fb_return(fb);
   Serial.println("OK"); 

}

void Send_line(uint8_t *image_data,size_t   image_size){
   LINE.notifyPicture("DETECT!!",image_data, image_size);
  }
