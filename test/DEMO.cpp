//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ESP32 ESP-NOW Web Server Master Sender AP Mode
/*
 * Reference :
 * - ESP32: ESP-NOW Web Server Sensor Dashboard (ESP-NOW + Wi-Fi) : https://randomnerdtutorials.com/esp32-esp-now-wi-fi-web-server/
 * - ESP-NOW with ESP32: Send Data to Multiple Boards (one-to-many) : https://randomnerdtutorials.com/esp-now-one-to-many-esp32-esp8266/
 * - ESP32 Async Web Server – Control Outputs with Arduino IDE (ESPAsyncWebServer library) : https://randomnerdtutorials.com/esp32-async-web-server-espasyncwebserver-library/
 * - and from several other sources.
 */

// ======================================== Including the libraries.
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
// ========================================

// Include the contents of the User Interface Web page, stored in the same folder as the .ino file
#include "PageIndex.h" 

// Defines the Wi-Fi channel.
// Configured channel range from 1~13 channels (by default).
// "ESP32 Master" and "ESP32 Slaves" must use the same Wi-Fi channel.
#define CHANNEL 1 

// Defines the Digital Pin of the "On Board LED".
#define ON_Board_LED 2  

// ======================================== Access Point Declaration and Configuration.
const char* ssid = "ESP32_WS";  //--> access point name
const char* password = "helloesp32WS"; //--> access point password

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
// ======================================== 

// ======================================== REPLACE WITH THE MAC ADDRESS OF YOUR SLAVES / RECEIVERS / ESP32 RECEIVERS.
uint8_t broadcastAddressESP32Slave1[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Slave 1 MAC Address
uint8_t broadcastAddressESP32Slave2[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Slave 2 MAC Address
// ========================================

// ======================================== The variables used to check the parameters passed in the URL.
// Look in the "PageIndex.h" file.
// "set_LED?board="+board+"&gpio_output="+gpio+"&val="+value
// For example :
// set_LED?board=ESP32Slave1&gpio_output=13&val=1
// PARAM_INPUT_1 = ESP32Slave1
// PARAM_INPUT_2 = 13
// PARAM_INPUT_3 = 1
const char* PARAM_INPUT_1 = "board";
const char* PARAM_INPUT_2 = "gpio_output";
const char* PARAM_INPUT_3 = "val";
// ======================================== 

// ======================================== Structure example to send data
// Must match the receiver structure
typedef struct struct_message_send {
  int send_GPIO_num;
  int send_Val;
} struct_message_send;

struct_message_send send_Data; //--> Create a struct_message to send data.
// ======================================== 

// Create a variable of type "esp_now_peer_info_t" to store information about the peer.
esp_now_peer_info_t peerInfo;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// ________________________________________________________________________________ Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  digitalWrite(ON_Board_LED, HIGH); //--> Turn on ON_Board_LED.
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  Serial.println(">>>>>");
  digitalWrite(ON_Board_LED, LOW); //--> Turn off ON_Board_LED.
}
// ________________________________________________________________________________

// ________________________________________________________________________________ Subroutine to prepare data and send it to the Slaves.
void send_data_to_slave(int slave_number, int gpio_number, int value) {
  Serial.println();
  Serial.print(">>>>> ");
  Serial.println("Send data");

  send_Data.send_GPIO_num = gpio_number;
  send_Data.send_Val = value;

  esp_err_t result;

  // ::::::::::::::::: Sending data to Slave 1
  if (slave_number == 1) {
    result = esp_now_send(broadcastAddressESP32Slave1, (uint8_t *) &send_Data, sizeof(send_Data));
  }
  // :::::::::::::::::

  // ::::::::::::::::: Sending data to Slave 2
  if (slave_number == 2) {
    result = esp_now_send(broadcastAddressESP32Slave2, (uint8_t *) &send_Data, sizeof(send_Data));
  }
  // :::::::::::::::::
  
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
}
// ________________________________________________________________________________

// ________________________________________________________________________________ VOID SETUP()
void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);
  Serial.println();

  pinMode(ON_Board_LED,OUTPUT); //--> On Board LED port Direction output
  digitalWrite(ON_Board_LED, LOW); //--> Turn off Led On Board

  // ---------------------------------------- Set Wi-Fi as Access Point and Wifi Station.
  WiFi.mode(WIFI_AP_STA);
  // ----------------------------------------

  // ---------------------------------------- Set the Wi-Fi channel.
  // "ESP32 Master" and "ESP32 Slaves" must use the same Wi-Fi channel.
  
  int cur_WIFIchannel = WiFi.channel();

  if (cur_WIFIchannel != CHANNEL) {
    //WiFi.printDiag(Serial); // Uncomment to verify channel number before
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
    //WiFi.printDiag(Serial); // Uncomment to verify channel change after
  }
  // ---------------------------------------- 

  // ---------------------------------------- Setting up ESP32 to be an Access Point.
  Serial.println();
  Serial.println("-------------");
  Serial.println("Setting up ESP32 to be an Access Point.");
  WiFi.softAP(ssid, password); //--> Creating Access Points
  delay(1000);
  Serial.println("Setting up ESP32 softAPConfig.");
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.println("-------------");
  //----------------------------------------

  // ---------------------------------------- Init ESP-NOW
  Serial.println();
  Serial.println("-------------");
  Serial.println("Start initializing ESP-NOW...");
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    Serial.println("Restart ESP32...");
    Serial.println("-------------");
    delay(1000);
    ESP.restart();
  }
  Serial.println("Initializing ESP-NOW was successful.");
  Serial.println("-------------");
  // ----------------------------------------

  // ---------------------------------------- Once ESPNow is successfully Init, we will register for Send CB to get the status of Trasnmitted packet
  Serial.println();
  Serial.println("get the status of Trasnmitted packet");
  esp_now_register_send_cb(OnDataSent);
  // ---------------------------------------- 

  // ---------------------------------------- Register Peer
  Serial.println();
  Serial.println("-------------");
  Serial.println("Register peer");
  peerInfo.encrypt = false;
  // ::::::::::::::::: register first peer
  Serial.println("Register first peer (ESP32 Slave 1)");
  memcpy(peerInfo.peer_addr, broadcastAddressESP32Slave1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add first peer");
    return;
  }
  // :::::::::::::::::
  // ::::::::::::::::: register second peer
  Serial.println("Register second peer (ESP32 Slave 2)");
  memcpy(peerInfo.peer_addr, broadcastAddressESP32Slave2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add second peer");
    return;
  }
  // :::::::::::::::::
  Serial.println("-------------");
  // ---------------------------------------- 

  // ---------------------------------------- Handle Web Server
  Serial.println();
  Serial.println("Setting Up the Main Page on the Server.");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", MAIN_page);
  });
  // ----------------------------------------

  // ---------------------------------------- Send a GET request to <ESP_IP>/set_LED?board=<inputMessage1>&gpio_output=<inputMessage2>&val=<inputMessage3>
  server.on("/set_LED", HTTP_GET, [] (AsyncWebServerRequest *request) {
    // :::::::::::::::::
    // GET input value on <ESP_IP>/set_LED?board=<inputMessage1>&gpio_output=<inputMessage2>&val=<inputMessage3>
    // PARAM_INPUT_1 = inputMessage1
    // PARAM_INPUT_2 = inputMessage2
    // PARAM_INPUT_3 = inputMessage3
    // Board = PARAM_INPUT_1
    // LED_gpio_num = PARAM_INPUT_2
    // LED_val = PARAM_INPUT_3
    // :::::::::::::::::

    String Board;
    String LED_gpio_num;
    String LED_val;
    
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2) && request->hasParam(PARAM_INPUT_3)) {
      Board = request->getParam(PARAM_INPUT_1)->value();
      LED_gpio_num = request->getParam(PARAM_INPUT_2)->value();
      LED_val = request->getParam(PARAM_INPUT_3)->value();

      String Rslt = "Board : " + Board + " || GPIO : " + LED_gpio_num + " || Set to :" + LED_val;
      Serial.println();
      Serial.println(Rslt);
      // Conditions for sending data to Slave 1.
      if (Board == "ESP32Slave1") send_data_to_slave(1, LED_gpio_num.toInt(), LED_val.toInt());
      // Conditions for sending data to Slave 2.
      if (Board == "ESP32Slave2") send_data_to_slave(2, LED_gpio_num.toInt(), LED_val.toInt());
    }
    else {
      Board = "No message sent";
      LED_gpio_num = "No message sent";
      LED_val = "No message sent";
    }
    request->send(200, "text/plain", "OK");
  });
  // ---------------------------------------- 
  
  Serial.println();
  Serial.println("Starting the Server.");
  server.begin();

  Serial.println();
  Serial.println("------------");
  Serial.print("SSID name : ");
  Serial.println(ssid);
  Serial.print("IP address : ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Wi-Fi channel : ");
  Serial.println(WiFi.channel());
  Serial.println();
  Serial.println("Connect your computer or mobile Wifi to the SSID above.");
  Serial.println("Visit the IP Address above in your browser to open the main page.");
  Serial.println("------------");
  Serial.println();
}
// ________________________________________________________________________________

// ________________________________________________________________________________ VOID LOOP()
void loop() {
  // put your main code here, to run repeatedly:

}
// ________________________________________________________________________________
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

------------------------------------------------------------------------------------------- PageIndex.h
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE HTML>
<html>
  <head>
    <title>ESP32 ESP-NOW & WEB SERVER (CONTROLLING)</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      html {font-family: Arial; display: inline-block; text-align: center;}
      p {font-size: 1.2rem;}
      body {margin: 0;}
      .topnav {overflow: hidden; background-color: #6C0BA9; color: white; font-size: 1.5rem;}
      .content {padding: 20px; }
      .card {background-color: white; box-shadow: 0px 0px 10px 1px rgba(140,140,140,.5); border: 1px solid #6C0BA9; border-radius: 15px;}
      .card.header {background-color: #6C0BA9; color: white; border-bottom-right-radius: 0px; border-bottom-left-radius: 0px; border-top-right-radius: 12px; border-top-left-radius: 12px;}
      .cards {max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));}
      .reading {font-size: 2.8rem;}
      .packet {color: #bebebe;}
      .temperatureColor {color: #fd7e14;}
      .humidityColor {color: #1b78e2;}
      .LEDColor {color: #183153;}
      
      /* ----------------------------------- Toggle Switch */
      .switch {
        position: relative;
        display: inline-block;
        width: 70px;
        height: 34px;
      }

      .switch input {display:none;}

      .sliderTS {
        position: absolute;
        cursor: pointer;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        background-color: #D3D3D3;
        -webkit-transition: .4s;
        transition: .4s;
        border-radius: 34px;
      }

      .sliderTS:before {
        position: absolute;
        content: "";
        height: 26px;
        width: 26px;
        left: 4px;
        bottom: 4px;
        background-color: #f7f7f7;
        -webkit-transition: .4s;
        transition: .4s;
        border-radius: 50%;
      }

      input:checked + .sliderTS {
        background-color: #62c934;
      }

      input:focus + .sliderTS {
        box-shadow: 0 0 1px #2196F3;
      }

      input:checked + .sliderTS:before {
        -webkit-transform: translateX(26px);
        -ms-transform: translateX(26px);
        transform: translateX(36px);
      }

      .sliderTS:after {
        content:'OFF';
        color: white;
        display: block;
        position: absolute;
        transform: translate(-50%,-50%);
        top: 50%;
        left: 70%;
        font-size: 10px;
        font-family: Verdana, sans-serif;
      }

      input:checked + .sliderTS:after {  
        left: 25%;
        content:'ON';
      }
      /* ----------------------------------- */
      
      /* ----------------------------------- Slider */
      .slidecontainer {
        width: 100%;
      }

      .slider {
        -webkit-appearance: none;
        width: 50%;
        height: 10px;
        border-radius: 5px;
        background: #d3d3d3;
        outline: none;
        opacity: 0.7;
        -webkit-transition: .2s;
        transition: opacity .2s;
      }

      .slider:hover {
        opacity: 1;
      }

      .slider::-webkit-slider-thumb {
        -webkit-appearance: none;
        appearance: none;
        width: 20px;
        height: 20px;
        border-radius: 50%;
        background: #1b78e2;
        cursor: pointer;
      }

      .slider::-moz-range-thumb {
        width: 20px;
        height: 20px;
        border-radius: 50%;
        background: #1b78e2;
        cursor: pointer;
      }
      /* ----------------------------------- */
    </style>
  </head>
  
  <body>
    <div class="topnav">
      <h3>ESP32 ESP-NOW & WEB SERVER (CONTROLLING)</h3>
    </div>
    
    <br>
    
    <div class="content">
      <div class="cards">
         
        <div class="card">
          <div class="card header">
            <h2>ESP32 BOARD #1</h2>
          </div>
          <br>
          
          <h4 class="LEDColor">LED 1 : </h4> 
          <label class="switch">
            <input type="checkbox" id="togLED1" onclick="send_LED_State_Cmd('ESP32Slave1','togLED1','13')">
            <div class="sliderTS"></div>
          </label>
          <br><br>
          <h4 class="LEDColor">LED 2 : </h4> 
          <div class="slidecontainer">
            <input type="range" min="0" max="10" value="0" class="slider" id="mySlider1">
          </div>
          <br>
        </div>
        
        <div class="card">
          <div class="card header">
            <h2>ESP32 BOARD #2</h2>
          </div>
          <br>
          
          <h4 class="LEDColor">LED 1 : </h4> 
          <label class="switch">
            <input type="checkbox" id="togLED2" onclick="send_LED_State_Cmd('ESP32Slave2','togLED2','13')">
            <div class="sliderTS"></div>
          </label>
          <br><br>
          <h4 class="LEDColor">LED 2 : </h4> 
          <div class="slidecontainer">
            <input type="range" min="0" max="10" value="0" class="slider" id="mySlider2">
          </div>
          <br>
        </div>
        
      </div>
    </div>
    
    <script>
      //------------------------------------------------------------ The variables for the slider.
      var sliderLED1 = document.getElementById("mySlider1");
      var last_sliderLED1_val = 0;
      var sliderLED2 = document.getElementById("mySlider2");
      var last_sliderLED2_val = 0;
      let sliders_used = "";
      //------------------------------------------------------------
      
      //------------------------------------------------------------ The variables for the Timer.
      var myTmr_send;
      var myTmr_send_interval = 250;
      var myTmr_send_start = 1;
      var myTmr_count_to_stop = 0;
      //------------------------------------------------------------
      
      //------------------------------------------------------------ The function called by "Toggle Switch" to control the LED.
      function send_LED_State_Cmd(board,id,gpio) {
        var tgLEDFlash = document.getElementById(id);
        var tgState;

        if (tgLEDFlash.checked == true) tgState = 1;
        if (tgLEDFlash.checked == false) tgState = 0;

        send_cmd(board,gpio,tgState);
      }
      //------------------------------------------------------------
      
      //------------------------------------------------------------ Function to detect when the slider is used and get its value and activate the Timer.
      sliderLED1.oninput = function() {
        sliders_used = "SL1";
        myTmr_count_to_stop = 0;
        if (myTmr_send_start == 1) {
          myTmr_send = setInterval(myTmr_send_LED_PWM_Cmd, myTmr_send_interval);
          myTmr_send_start = 0;
        }
      }
      
      sliderLED2.oninput = function() {
        sliders_used = "SL2";
        myTmr_count_to_stop = 0;
        if (myTmr_send_start == 1) {
          myTmr_send = setInterval(myTmr_send_LED_PWM_Cmd, myTmr_send_interval);
          myTmr_send_start = 0;
        }
      }
      // ------------------------------------------------------------
      
      // ---------------------------------------------------------------------- Timer for sending slider values and other data from this page to ESP32.
      function myTmr_send_LED_PWM_Cmd() {
        if (sliders_used == "SL1") {
          if (last_sliderLED1_val != sliderLED1.value) {
            send_cmd('ESP32Slave1',12,sliderLED1.value);
          }
          last_sliderLED1_val = sliderLED1.value;
        }
        
        if (sliders_used == "SL2") {
          if (last_sliderLED2_val != sliderLED2.value) {
            send_cmd('ESP32Slave2',12,sliderLED2.value);
          }
          last_sliderLED2_val = sliderLED2.value;
        }
        
        myTmr_count_to_stop++;
        if (myTmr_count_to_stop > 5) {
          myTmr_send_start = 1;
          clearInterval(myTmr_send);
        }
      }
      // ---------------------------------------------------------------------- 
      
      // ---------------------------------------------------------------------- XMLHttpRequest to submit data.
      function send_cmd(board,gpio,value) {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "set_LED?board="+board+"&gpio_output="+gpio+"&val="+value, true);
        xhr.send();
      }
      // ---------------------------------------------------------------------- 
    </script>
  </body>
</html>
)=====";
-------------------------------------------------------------------------------------------



#################################################################################################
#################################################################################################
#################################################################################################
#################################################################################################



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ESP32 ESP-NOW Web Server Slave Receiver AP Mode
/*
 * Reference :
 * - ESP32: ESP-NOW Web Server Sensor Dashboard (ESP-NOW + Wi-Fi) : https://randomnerdtutorials.com/esp32-esp-now-wi-fi-web-server/
 * - ESP-NOW with ESP32: Send Data to Multiple Boards (one-to-many) : https://randomnerdtutorials.com/esp-now-one-to-many-esp32-esp8266/
 * - ESP32 Async Web Server – Control Outputs with Arduino IDE (ESPAsyncWebServer library) : https://randomnerdtutorials.com/esp32-async-web-server-espasyncwebserver-library/
 * - and from several other sources.
 */

// ======================================== Including the libraries.
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
// ========================================

// Defines the Wi-Fi channel.
// Configured channel range from 1~13 channels (by default).
// "ESP32 Master" and "ESP32 Slaves" must use the same Wi-Fi channel.
#define CHANNEL 1 

// Defines the Digital Pin of the "On Board LED".
#define ON_Board_LED 2 

// Defines GPIO 13 as LED_1.
#define LED_1 13 

// Defines GPIO 12 as LED_2.
#define LED_2 12 

// ======================================== Variables for PWM settings.
const int PWM_freq = 5000;
const int PWM_Channel = 0;
const int PWM_resolution = 8;
// ========================================

// ======================================== Structure example to receive data.
// Must match the sender structure
typedef struct struct_message_receive {
  int receive_GPIO_num;
  int receive_Val;
} struct_message_receive;

// Create a struct_message to receive data.
struct_message_receive receive_Data;
// ========================================

// ________________________________________________________________________________ Callback when data is received.
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  digitalWrite(ON_Board_LED, HIGH); //--> Turn on ON_Board_LED.

  // ---------------------------------------- Get the MAC ADDRESS of the sender / slave.
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  // ---------------------------------------- 

  memcpy(&receive_Data, incomingData, sizeof(receive_Data)); //--> Copy the information in the "incomingData" variable into the "receive_Data" structure variable.

  // ---------------------------------------- Prints the MAC ADDRESS of sender / Master and Bytes received.
  Serial.println();
  Serial.println("<<<<< Receive Data:");
  Serial.print("Packet received from: ");
  Serial.println(macStr);
  Serial.print("Bytes received: ");
  Serial.println(len);
  // ---------------------------------------- 

  // ---------------------------------------- Conditions for controlling the LEDs.
  if (receive_Data.receive_GPIO_num == 13) digitalWrite(LED_1, receive_Data.receive_Val);
  if (receive_Data.receive_GPIO_num == 12) {
    int pwm = map(receive_Data.receive_Val, 0, 10, 0, 255);
    ledcWrite(PWM_Channel, pwm);  
  }
  // ---------------------------------------- 
  
  // ---------------------------------------- Prints all data received from the sender / Master.
  Serial.println("Receive Data: ");
  Serial.println(receive_Data.receive_GPIO_num);
  Serial.println(receive_Data.receive_Val);
  Serial.println("<<<<<");
  // ---------------------------------------- 

  digitalWrite(ON_Board_LED, LOW); //--> Turn off ON_Board_LED.
}
// ________________________________________________________________________________

// ________________________________________________________________________________ VOID SETUP()
void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);
  Serial.println();

  pinMode(ON_Board_LED,OUTPUT); //--> On Board LED port Direction output
  digitalWrite(ON_Board_LED, LOW); //--> Turn off Led On Board

  pinMode(LED_1,OUTPUT); //--> LED_1 port Direction output

  pinMode(LED_2,OUTPUT); //--> LED_2 port Direction output

  // ---------------------------------------- PWM settings.
  ledcSetup(PWM_Channel, PWM_freq, PWM_resolution);
  ledcAttachPin(LED_2, PWM_Channel);
  // ----------------------------------------

  // ---------------------------------------- Set Wi-Fi as Wifi Station Mode.
  WiFi.mode(WIFI_STA);
  // ----------------------------------------

  // ---------------------------------------- Set the Wi-Fi channel.
  // "ESP32 Master" and "ESP32 Slaves" must use the same Wi-Fi channel.
  
  int cur_WIFIchannel = WiFi.channel();

  if (cur_WIFIchannel != CHANNEL) {
    //WiFi.printDiag(Serial); // Uncomment to verify channel number before
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
    //WiFi.printDiag(Serial); // Uncomment to verify channel change after
  }

  Serial.println("-------------");
  Serial.print("Wi-Fi channel : ");
  Serial.println(WiFi.channel());
  Serial.println("-------------");
  // ---------------------------------------- 

  // ---------------------------------------- Init ESP-NOW
  Serial.println("-------------");
  Serial.println("Start initializing ESP-NOW...");
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    Serial.println("Restart ESP32...");
    Serial.println("-------------");
    delay(1000);
    ESP.restart();
  }
  Serial.println("Initializing ESP-NOW was successful.");
  Serial.println("-------------");
  // ---------------------------------------- 

  // ---------------------------------------- Register for a callback function that will be called when data is received
  Serial.println();
  Serial.println("Register for a callback function that will be called when data is received");
  esp_now_register_recv_cb(OnDataRecv);
  // ----------------------------------------
}
// ________________________________________________________________________________

// ________________________________________________________________________________ VOID LOOP()
void loop() {
  // put your main code here, to run repeatedly:
  
}
// ________________________________________________________________________________
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<