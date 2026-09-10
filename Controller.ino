//*********************************************************************************************************
// Remote control firmware
// Version 1.0
// Date: 14/04/2026
//*********************************************************************************************************


#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <driver/gpio.h>
#include "ESP32_NOW.h"
#include <WiFi.h>
#include "esp_wifi.h"


//*********************************************************************************************************
// Definition of the pin of the board
//*********************************************************************************************************

#define BUTTON_BACK   1
#define BUTTON_START  0
#define BUTTON_UP     10
#define BUTTON_DOWN   11
#define BUTTON_LEFT   12      
#define BUTTON_RIGHT  13

#define BUTTON_Y      15  
#define BUTTON_X      14 
#define BUTTON_B      21 
#define BUTTON_A      16 

//use as direction of travel
#define JOYSTICK1_BUTTON  6
//Vetical motion
#define JOYSTICK1_Pos1    5
//Horizontal motion
#define JOYSTICK1_Pos2    4
//Max value of board
#define JOYSTICK1_Pos1Max    919
#define JOYSTICK1_Pos1Mid    463
#define JOYSTICK1_Pos1Min    0
#define JOYSTICK1_Pos2Max    843
#define JOYSTICK1_Pos2Mid    391
#define JOYSTICK1_Pos2Min    0

#define JOYSTICK1_Threshold 50

//use for speed adjustment 
#define JOYSTICK2_BUTTON  9
//Vetical motion
#define JOYSTICK2_Pos1    8
//Horizontal motion
#define JOYSTICK2_Pos2    7
//Max value of board
#define JOYSTICK2_Pos1Max    856
#define JOYSTICK2_Pos1Mid   403
#define JOYSTICK2_Pos1Min    0
#define JOYSTICK2_Pos2Max    924
#define JOYSTICK2_Pos2Mid   437
#define JOYSTICK2_Pos2Min    0

#define JOYSTICK2_Threshold 50


#define LED3       46

#define BATTERY_V     2

// Pin for the Display
#define TFT_CS        39
#define TFT_DC        38
#define TFT_RST       42
#define TFT_MOSI      41
#define TFT_SCLK      40


//*********************************************************************************************************
// Display
//*********************************************************************************************************

// Initialise the display using the specific pins
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

//*********************************************************************************************************
// ESP Now Communication
//*********************************************************************************************************

//ESP32 Remote MAC Address
//Robot MAC Address NEED UPDATING IF THE BOARD CHANGE
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

struct_message myData;
struct_message2 incomingReadings;


//*********************************************************************************************************
// Structure of data to display to display and used in separate thread protected by mutex
//*********************************************************************************************************
//
struct DisplayState 
{
  float voltageRobot;
  bool ComStatus;
  int Direction;
  int Speed; 

};

// Global instances
DisplayState sharedData;
SemaphoreHandle_t xMutex;

//*********************************************************************************************************
// TASK 1: Display data
// on a separate thread
// Display the data onto the display.
//*********************************************************************************************************
void DisplayTask(void *pvParameters) 
{

 DisplayState localData; 
 
  // 4. Initialise TFT (240x240 resolution)
  tft.init(240, 240);
  tft.setSPISpeed(40000000); // 40MHz
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);

  // 2. Write Text
  tft.setCursor(20, 20);            // Move to x=20, y=20
  tft.setTextColor(ST77XX_WHITE);   // Set font to white
  tft.setTextSize(3);               // Set size to 3
  tft.println("Robot Remote");      // Write and move to next line
  
  tft.setCursor(20, 60);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.print("Ready");

  for (;;) 
  {
    // Try to take the mutex (wait up to 100ms)
    if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
      //copy structure to local structure
      localData = sharedData;
      // Release the mutex
      xSemaphoreGive(xMutex);

      
      //Read the battery Voltage
      int BatValue = analogRead(BATTERY_V);
      float BatValueV = (float)BatValue * 3.3/4095.0  *(40.2+10.0)/10.0;

      tft.setCursor(20, 80);           // Move to x=20, y=80
      tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);   // Set font to white
      tft.setTextSize(2);               // Set size to 3 (readable)
      char floatStr[10]; // Buffer for "00.00" + null terminator
      sprintf(floatStr, "Battery: %05.2fV", BatValueV);
      tft.println(floatStr);            // Write and move to next line

      tft.setCursor(20, 100);           // Move to x=20, y=100

      tft.setTextSize(2); 
      if(localData.ComStatus==1)
      {
        tft.setTextColor(ST77XX_GREEN,ST77XX_BLACK);   // Set font to green
        tft.println("Com OK  ");
      }
      else
      {
        tft.setTextColor(ST77XX_RED,ST77XX_BLACK);   // Set font to red
        tft.println("Com Fail");
      }
      tft.setCursor(20, 120);  

      sprintf(floatStr, "Robot: %05.2fV", localData.voltageRobot);
      tft.println(floatStr);            // Write and move to next line


      tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);   // Set font to white
      tft.setCursor(20, 140);  
      char Line[20]; 
      sprintf(Line,"Direction: %04i",localData.Direction); 
      tft.println(Line); 
      tft.setCursor(20, 160);    
      sprintf(Line,"Speed: %03i%%",localData.Speed); 
      tft.println(Line); 


    }

    // Delay task for 500ms
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}


//*********************************************************************************************************
// Callback from ESP now communication
//*********************************************************************************************************

void OnDataRecv(const esp_now_recv_info_t * recv_info, const uint8_t *incomingData, int len) 
{
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  
  // Use recv_info->src_addr to get the sender's MAC
  Serial.printf("\nReceived from %02X:%02X:%02X\n", 
                recv_info->src_addr[0], recv_info->src_addr[1], recv_info->src_addr[2]);

  if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(100)) == pdTRUE)
  {
    sharedData.voltageRobot=incomingReadings.voltageRobot;      
    // Release the mutex
    xSemaphoreGive(xMutex);
            
  }

}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
  Serial.print("Last Packet Delivery Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");

 if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(100)) == pdTRUE)
  {
     if(status == ESP_NOW_SEND_SUCCESS)
       sharedData.ComStatus=1;
      else 
       sharedData.ComStatus=0;      
      // Release the mutex
      xSemaphoreGive(xMutex);
  }

}

//*********************************************************************************************************
// TASK 1: Read button data
// on a separate thread
// Get the data from the buttons and send the data using ESP Now
//*********************************************************************************************************

void ComTask(void *pvParameters) 
{

  WiFi.mode(WIFI_STA); 
  vTaskDelay(100); 
    
  Serial.print("ESP32 Remote MAC Address: ");
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

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    ESP.restart();
  }  

  for (;;) 
  {

    //read the buttons data
    
    //Prepare the string to send to robot
    sprintf(myData.text, "");

    //Read joystick 1 values used for direction
    int potValue1 = analogRead(JOYSTICK1_Pos1);
    int potValue2 = analogRead(JOYSTICK1_Pos2);
    if((potValue1>JOYSTICK1_Pos1Mid+JOYSTICK1_Threshold)||(potValue1<JOYSTICK1_Pos1Mid-JOYSTICK1_Threshold)||(potValue2>JOYSTICK1_Pos2Mid+JOYSTICK1_Threshold)||(potValue2<JOYSTICK1_Pos2Mid-JOYSTICK1_Threshold))
    {
      //Joystick 1 value is above threshold
        
      if(potValue1>JOYSTICK1_Pos1Mid)
      {
        potValue1= (potValue1-JOYSTICK1_Pos1Mid)*100/(JOYSTICK1_Pos1Max-JOYSTICK1_Pos1Mid);
      }
      else
      {
        potValue1= (potValue1-JOYSTICK1_Pos1Mid)*100/(JOYSTICK1_Pos1Mid-JOYSTICK1_Pos1Min);
      }
      if(potValue2>JOYSTICK1_Pos2Mid)
      {
        potValue2= (potValue2-JOYSTICK1_Pos2Mid)*100/(JOYSTICK1_Pos2Max-JOYSTICK1_Pos2Mid);
      }
      else
      {
        potValue2= (potValue2-JOYSTICK1_Pos2Mid)*100/(JOYSTICK1_Pos2Mid-JOYSTICK1_Pos2Min);
      }
      float potRadian=atan2((float)potValue2,(float)potValue1);
      int potDegree=(int) (potRadian*180.0/PI);

      Serial.print("Joystick 1 Pos1V:");
      Serial.print(potValue1);
      Serial.print(" Pos2H:");
      Serial.print(potValue2);
      Serial.print(" PosD:");
      Serial.println(potDegree);

      // Prepare data to send
      sprintf(myData.text, "Joystick 1: %i",potDegree);
      myData.counter++;
      myData.Direction=potDegree;

      //Read joystick 2 values
      potValue1 = analogRead(JOYSTICK2_Pos1);
      //potValue2 = analogRead(JOYSTICK2_Pos2); //not used as only want to use 1 value
      if((potValue1>JOYSTICK2_Pos1Mid+JOYSTICK2_Threshold)||(potValue1<JOYSTICK2_Pos1Mid-JOYSTICK2_Threshold))
      {
          //if value above threshold
          if(potValue1>JOYSTICK2_Pos1Mid)
          {
            potValue1= (potValue1-JOYSTICK2_Pos1Mid)*50/(JOYSTICK2_Pos1Max-JOYSTICK2_Pos1Mid);
          }
          else
          {
            potValue1= (potValue1-JOYSTICK2_Pos1Mid)*50/(JOYSTICK2_Pos1Mid-JOYSTICK2_Pos1Min);
          }
          //clamp value
          if(potValue1<-50) potValue1=-50;
          if(potValue1>50) potValue1=50;
          potValue1=potValue1+50;  


        Serial.print("Joystick 2 Pos1V:");
        Serial.print(potValue1);
        //Serial.print(" Pos2H:");
        //Serial.println(potValue2);     
        sprintf(myData.text, "%s Joystick 2: %i",myData.text,potValue1);      
        myData.Speed=potValue1;
      }
      else
          myData.Speed=50; 

    }

    //if the joysticks are above the threahold, send the data
    if(strlen(myData.text)>0)
    {
      esp_err_t result = esp_now_send(peerAddress, (uint8_t *) &myData, sizeof(myData));
        
      if (result != ESP_OK) 
      {
        Serial.println("Error sending the data");
      }
      else
      {
        Serial.println("No Error sending the data");  
      }
    
    } 
    else
    {
      //update speed and directino for display
      myData.Speed = 0;
      myData.Direction = 0;
    }

    // Take the mutex
    if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(200)) == pdTRUE)
    {
      sharedData.Speed = myData.Speed;
      sharedData.Direction = myData.Direction;
      // Release the mutex
      xSemaphoreGive(xMutex);  
    }

    // Delay task for 100 mS
    vTaskDelay(pdMS_TO_TICKS(100));
  }

}

//*********************************************************************************************************
// Initialise the board
//*********************************************************************************************************

void setup() {

  //remove default settings for those pins
  gpio_reset_pin((gpio_num_t)38); // DC
  gpio_reset_pin((gpio_num_t)39);
  gpio_reset_pin((gpio_num_t)41);
  gpio_reset_pin((gpio_num_t)40);
  gpio_reset_pin((gpio_num_t)42);
  
  pinMode(LED3, OUTPUT);
  
  pinMode(JOYSTICK1_BUTTON, INPUT_PULLUP);
  pinMode(JOYSTICK1_Pos1, INPUT);
  pinMode(JOYSTICK1_Pos2, INPUT);
  pinMode(JOYSTICK2_BUTTON, INPUT_PULLUP);
  pinMode(JOYSTICK2_Pos1, INPUT);
  pinMode(JOYSTICK2_Pos2, INPUT);

  pinMode( BUTTON_BACK   , INPUT_PULLUP);
  pinMode( BUTTON_START  , INPUT_PULLUP);
  pinMode( BUTTON_UP     , INPUT_PULLUP);
  pinMode( BUTTON_DOWN   , INPUT_PULLUP);
  pinMode( BUTTON_LEFT   , INPUT_PULLUP);      
  pinMode( BUTTON_RIGHT  , INPUT_PULLUP);

  pinMode( BUTTON_Y      , INPUT_PULLUP);  
  pinMode( BUTTON_X      , INPUT_PULLUP); 
  pinMode( BUTTON_B      , INPUT_PULLUP); 
  pinMode( BUTTON_A      , INPUT_PULLUP); 
  
  digitalWrite(LED3, LOW); //turn off LED

  pinMode(BATTERY_V, INPUT);

  Serial.begin(115200); // baudrate is ignored for native USB but required for compatibility

  // Initialize the Mutex
  xMutex = xSemaphoreCreateMutex();

  if (xMutex != NULL) 
  {
    // Initialize default values
    sharedData = {0};

    // Create tasks 
    xTaskCreate(DisplayTask, "DisplayTask", 10000, NULL, 1, NULL);
    xTaskCreate(ComTask, "ComTask", 4096, NULL, 1, NULL);
    
    Serial.println("System Initialised: Multithreading active.");
  } 
  else
  {
    Serial.println("Error: Mutex could not be created!");
    ESP.restart();
  }


}


//*********************************************************************************************************
// MAIN Loop
//*********************************************************************************************************

void loop()
{

  //LED
  digitalWrite(LED3, LOW);
  vTaskDelay(1800);
  digitalWrite(LED3, HIGH);
  vTaskDelay(200);
    

  /*
  if we want to use other button in the future, here is the code to read the button state
  int buttonState=0;
  //read button states without debounce, 1 is not pressed and 0 is pressed.
  buttonState = digitalRead(JOYSTICK1_BUTTON);  Serial.println("JOYSTICK1_BUTTON "+String(buttonState)); 
  buttonState = digitalRead(JOYSTICK2_BUTTON);  Serial.println("JOYSTICK2_BUTTON "+String(buttonState)); 
  buttonState = digitalRead(BUTTON_BACK);  Serial.println("BUTTON_BACK "+String(buttonState)); 
  buttonState = digitalRead(BUTTON_START);  Serial.println("BUTTON_START "+String(buttonState)); 
  buttonState = digitalRead(BUTTON_UP);  Serial.println("BUTTON_UP "+String(buttonState)); 
  buttonState = digitalRead(BUTTON_DOWN);  Serial.println("BUTTON_DOWN "+String(buttonState)); 
  buttonState = digitalRead(BUTTON_LEFT);  Serial.println("BUTTON_LEFT "+String(buttonState)); 
  buttonState = digitalRead(BUTTON_RIGHT);  Serial.println("BUTTON_RIGHT "+String(buttonState)); 
  buttonState = digitalRead(BUTTON_Y);  Serial.println("BUTTON_Y "+String(buttonState)); 
  buttonState = digitalRead(BUTTON_X);  Serial.println("BUTTON_X "+String(buttonState)); 
  buttonState = digitalRead(BUTTON_B);  Serial.println("BUTTON_B "+String(buttonState)); 
  buttonState = digitalRead(BUTTON_A);  Serial.println("BUTTON_A "+String(buttonState)); 
  */
  
}
