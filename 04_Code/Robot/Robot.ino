//*********************************************************************************************************
// Robot firmware to work with remote control
// Version 1.0
// Date: 16/04/2026
//*********************************************************************************************************

#include <HardwareSerial.h> // Reference the ESP32 built-in serial port library
#include <Wire.h> // Include the I2C library
#include <esp_now.h>
#include <WiFi.h>
#include "esp_wifi.h"

int Speed=0; //0 stop 100 full speed
int Direction=0; //direction of travel
int CurrentDirectionLeft=0; //direction of travel of motor
int CurrentDirectionRight=0; //direction of travel of motor


double TimeOfCMD=0; //store time of received command
double TimeOfCMDSend=1000; //store time of send command

//*********************************************************************************************************
// ESP Now Communication
//*********************************************************************************************************
//Robot Mac address: xx:xx:xx:xx:xx:xx
//Remote board MAC Address NEED UPDATING IF THE BOARD CHANGE
uint8_t peerAddress[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX}; 

// Encryption Keys
// 16 bytes exactly
const uint8_t pmk_key[16] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};
const uint8_t lmk_key[16] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};

// Data structure to send/receive
typedef struct struct_message 
{
  char text[32];
  int Direction;
  int Speed;
  int counter;
} struct_message;

typedef struct struct_message2 
{
  float voltageRobot;
} struct_message2;

struct_message2 myData;
struct_message incomingReadings;

//*********************************************************************************************************
// Callback from ESP now communication
//*********************************************************************************************************

void OnDataRecv(const esp_now_recv_info_t * recv_info, const uint8_t *incomingData, int len) 
{
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  
  // Use recv_info->src_addr to get the sender's MAC
  Serial.printf("\nReceived from %02X:%02X:%02X: %s | Direction: %i | Speed: %d | Counter: %d \n", 
                recv_info->src_addr[0], recv_info->src_addr[1], recv_info->src_addr[2], 
                incomingReadings.text, incomingReadings.Direction,incomingReadings.Speed,incomingReadings.counter);
  Speed=incomingReadings.Speed; //0 stop 100 full speed
  Direction=incomingReadings.Direction; //direction of travel
  TimeOfCMD=millis();
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
  Serial.print("Last Packet Delivery Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

//*********************************************************************************************************
// Definition of the pin of the board
//*********************************************************************************************************

//motor control pin
#define ENA 32
#define IN1 33
#define IN2 25

#define ENB 26
#define IN3 27
#define IN4 23

//read battery voltage
#define BATTERY_V 34

//Lidar
HardwareSerial lidarSerial(2); // Using serial port 2
#define RXD2 21  //ESP receive data
#define TXD2 22  //ESP transmit data
#define LidarPower 2  //active low

//Mems
#define SCL 19
#define SDA 18

//Raspberry Pi
HardwareSerial PiSerial(3); // Using serial port 3
#define RXD3 17  //ESP receive data
#define TXD3 16  //ESP transmit data
#define PiPower 4   //active low


//*********************************************************************************************************
// Functions to control the motors
//*********************************************************************************************************

//Left motor counterclockwise
void MotorLeftCCW_Forward(int SpeedPercentage)
{
  int PWMvalue;
  if(SpeedPercentage<0) SpeedPercentage=0;
  if(SpeedPercentage>100)SpeedPercentage=100;
  PWMvalue=SpeedPercentage*255/100;

  //motor turn counterclockwise
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,HIGH);
  analogWrite(ENA,PWMvalue); 

  //Serial.print("Left F:");
  //Serial.print(PWMvalue); 

}

void MotorLeftCCW_Backward(int SpeedPercentage)
{
    int PWMvalue;
  if(SpeedPercentage<0) SpeedPercentage=0;
  if(SpeedPercentage>100)SpeedPercentage=100;
  PWMvalue=SpeedPercentage*255/100;   

  //motor turn clockwise
  digitalWrite(IN1,HIGH);
  digitalWrite(IN2,LOW);
  analogWrite(ENA,PWMvalue);

  //Serial.print("Left B:");
  //Serial.print(PWMvalue); 
}

//It is important to break before changing direction
void MotorLeftCCW_Brake()
{
     digitalWrite(IN1,LOW);
     digitalWrite(IN2,LOW);
     digitalWrite(ENA, LOW); 
}

//Right motor clockwise
void MotorRightCW_Forward(int SpeedPercentage)
{
    int PWMvalue;
  if(SpeedPercentage<0) SpeedPercentage=0;
  if(SpeedPercentage>100)SpeedPercentage=100;
  PWMvalue=SpeedPercentage*255/100;
  
     digitalWrite(IN3,HIGH);
     digitalWrite(IN4,LOW);
     analogWrite(ENB,PWMvalue);

   // Serial.print("Right F:");
   // Serial.print(PWMvalue); 
}

void MotorRightCW_Backward(int SpeedPercentage)
{
  int PWMvalue;
  if(SpeedPercentage<0) SpeedPercentage=0;
  if(SpeedPercentage>100)SpeedPercentage=100;
  PWMvalue=SpeedPercentage*255/100;
  
     digitalWrite(IN3,LOW);
     digitalWrite(IN4,HIGH);
     analogWrite(ENB,PWMvalue);

   //Serial.print("Right B:");
    //Serial.print(PWMvalue); 
}

//It is important ot break before changing direction
void MotorRightCW_Brake()
{
     digitalWrite(IN3,LOW);
     digitalWrite(IN4,LOW);
     digitalWrite(ENB, LOW); 
}


//*********************************************************************************************************
// Initialise the board
//*********************************************************************************************************

void setup() 
{

  Serial.begin(115200); // Initializing serial port

  WiFi.mode(WIFI_STA); 
  delay(100); 
    
  Serial.print("ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());

  //Enable Long Range mode on the station interface
  //esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);

  if (esp_now_init() != ESP_OK) ESP.restart();

  // Setup Encryption
  esp_now_set_pmk(pmk_key);

  // Register callbacks  
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register the other board as an encrypted peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;  // Use current WiFi channel
  peerInfo.encrypt = true;
  memcpy(peerInfo.lmk, lmk_key, 16);

  if (esp_now_add_peer(&peerInfo) != ESP_OK) 
  {
    Serial.println("Failed to add peer");
    ESP.restart();
  }  

  lidarSerial.begin(115200, SERIAL_8N1, RXD2, TXD2); //Initializing serial port for Lidar    
  PiSerial.begin(115200, SERIAL_8N1, RXD2, TXD2); //Initializing serial port RaspberryPi
  Wire.begin(SDA, SCL); //Initialise I2C pin for Mems sensor

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
     
  pinMode(ENB, OUTPUT);   
  pinMode(IN4, OUTPUT);
  pinMode(IN3, OUTPUT);

  pinMode(BATTERY_V, INPUT);

  MotorLeftCCW_Brake();
  MotorRightCW_Brake();

  TimeOfCMDSend=millis();
  TimeOfCMD=millis();
   
}


//*********************************************************************************************************
// MAIN Loop
//*********************************************************************************************************

void loop() 
{ 

  //if no message for 500ms stop the motors
  if(abs(TimeOfCMD-millis())>500) Speed=0;

  if(Speed == 0)
  {
    //If the speed is 0 stop the motors
    MotorLeftCCW_Brake();
    MotorRightCW_Brake();
    CurrentDirectionLeft=0;
    CurrentDirectionRight=0; 
  }
  else
  {
    if(abs(Direction)<5)  //need to go forward
    {
      //Reduce sensitivity of joystick, so it +/- 5 degrees, go straight 
      if(CurrentDirectionLeft==-1) MotorLeftCCW_Brake();
      if(CurrentDirectionRight==-1) MotorRightCW_Brake();
      MotorLeftCCW_Forward(Speed);
      MotorRightCW_Forward(Speed);   
      CurrentDirectionLeft=1;
      CurrentDirectionRight=1;     
    }
    else if(abs(Direction)>175) //need to go backwards
    {
      //Reduce sensitivity of joystick, so it +/- 5 degrees of 180, go backwards   
      if(CurrentDirectionLeft==1) MotorLeftCCW_Brake();
      if(CurrentDirectionRight==1) MotorRightCW_Brake();
      MotorLeftCCW_Backward(Speed);
      MotorRightCW_Backward(Speed);   
      CurrentDirectionLeft=-1;
      CurrentDirectionRight=-1;   
    }
    else 
    {
      //Determine the speed and direction for each motor from the direction provided by the remote
          
      // Convert to radians
      float rad = Direction * PI / 180.0f;

      // Compute components
      float forward = cosf(rad) * Speed;
      float turn = sinf(rad) * Speed;

      // Mix
      float left = forward + turn;
      float right = forward - turn;

      // Normalize if needed
      float max_mag = fabsf(left);
      if (fabsf(right) > max_mag) max_mag = fabsf(right);

      if (max_mag > 100.0f) 
      {
        left = (left / max_mag) * 100.0f;
        right = (right / max_mag) * 100.0f;
      }

      // Left Motor Counter Clokcwise
      if (left > 1.0f)
      {
        if(CurrentDirectionLeft==-1) MotorLeftCCW_Brake();
        MotorLeftCCW_Forward(left);
        CurrentDirectionLeft=1;
      } 
      else if (left < -1.0f)
      {
        if(CurrentDirectionLeft==1) MotorLeftCCW_Brake();
        MotorLeftCCW_Backward(-left);
        CurrentDirectionLeft=-1;
      }
      else
      {
        MotorLeftCCW_Brake();
        CurrentDirectionLeft=0;
      }

      // Right Motor Clockwise
      if (right > 1.0f) 
      {
        if(CurrentDirectionRight==-1) MotorRightCW_Brake();
        MotorRightCW_Forward(right);
        CurrentDirectionRight=1;    
      } 
      else if (right < -1.0f)
      {
        if(CurrentDirectionRight==1) MotorRightCW_Brake();
        MotorRightCW_Backward(-right);
        CurrentDirectionRight=-1;
      } 
      else
      {
        MotorRightCW_Brake();
        CurrentDirectionRight=0;
      }
    } 
  }

  //Read Battery Voltage every 1sec
  if(abs(TimeOfCMDSend-millis())>1000)
  {
    //send the voltage within 50ms of a received command to avoid clash of packet
    if(abs(TimeOfCMD-millis())<100)
    {
      int BatValue = analogRead(BATTERY_V);
      float BatValueV = (float)BatValue * 3.3/4095.0  *(68+10.0)/10.0;
      Serial.print("Battery:");
      Serial.println(BatValueV); 
      myData.voltageRobot=BatValueV;
      //send battery voltage to remote
      esp_err_t result = esp_now_send(peerAddress, (uint8_t *) &myData, sizeof(myData));
      TimeOfCMDSend=millis();
      if (result != ESP_OK) 
      {
        Serial.println("Error sending the data");
      }
      else
      {
        Serial.println("No Error sending the data");  
      }

    }
  }

}
