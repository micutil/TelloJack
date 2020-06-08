/*
 * TelloJack
 *   for Tello toy drone controller program.
 *   CC 4.0 BY Micono (https://github.com/micutil/TelloJack)
 *   
 *   ver 0.2.0 2020/06/10
 *   ver 0.1.0 2020/03/28
 * 
 * Application version (download http://ijutilities.micutil.com)
 * MicJack (TelloJack) version (download https://github.com/micutil/MicJack)
 * M5Stack JoyStick-unit version (this sketch: https://github.com/micutil/TelloJack)
 * M5Stack Accelometer version (this sketch: https://github.com/micutil/TelloJack)
 * M5Stack FACES JoyStick version (this sketch: https://github.com/micutil/TelloJack)
 * Odroid-GO version (this sketch: https://github.com/micutil/TelloJack)
 * 
 * Powered by 
 *   ESP32-Chimera-Core by tobozo https://github.com/tobozo/ESP32-Chimera-Core
 * 
 * ----------------------
 * Original
 * TelloControl_JoyFace
 * https://qiita.com/hsgucci/items/4571c060ece376f80db3
 * by @hsgucci
 * 
 * https://qiita.com/Mitsu-Murakita/items/b86ad79d3590adb3b5b9
 * by @Mitsu-Murakita
 * 
 * Tello
 * https://www.ryzerobotics.com/jp/tello
 * 
 * Tello Documents
 * https://www.ryzerobotics.com/jp/tello/downloads
 * 
 * ----------------------
 */

//****************
//ボードを選択
//****************
//#define ARDUINO_M5Stack_Core_ESP32 //ESP32 chimera board define
//#define ARDUINO_ODROID_ESP32
#define ARDUINO_M5StickC_ESP32

//*****************************
//M5Stackの場合のコントローラの選択
//*****************************
#ifdef ARDUINO_M5Stack_Core_ESP32 
  #include <M5Stack.h>
  #include "M5StackUpdater.h"   //for SD updater
  //#define useJoyStick         // JoyStick Unitで操作
  //#define useGrayAccel        // 加速度センサーで操作
  //#define useFacesJoyStick    // FACESのJoyStick操作
  //#define useFacesKey         //Underconstruction !!!!
  const int lcdW=320;
  const int lcdH=240;
#endif

//*****************************
//M5StickCの場合のコントローラの選択
//*****************************
#ifdef ARDUINO_M5StickC_ESP32
  #include <M5StickC.h>
  #include <Preferences.h>
  #define useM5CStick      
  //#define useM5CJoyS        // JoyStick Unitで操作
  //#define useHATStick     // HATのJoyStickで操作
  #define useHATJoyC      // HATのJoyCで操作
  const int lcdW=160;
  const int lcdH=80;
#endif

//------------
//Odroid用設定
//------------
#ifdef ARDUINO_ODROID_ESP32
  #include <M5Stack.h>
  #include "M5StackUpdater.h" //for SD updater
  const int lcdW=320;
  const int lcdH=240;
#endif

#include "Wire.h"
#include <WiFi.h>
#include <WiFiUdp.h> //<WiFiUdp.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// M5Stack Basic + Stick UNIT
#ifdef useJoyStick
  #define JOY_ADDR 0x52
  const String TelloJackName=" TelloJack JS ";
  const int TitleBannerColor=TFT_PURPLE;
  const String XaxisName=" Joy-X : ";
  const String YaxisName=" Joy-Y : ";
  const float jsr=1024.0/250.0;
#endif //useJoyStick

//M5Stack Gray (accelerometer)
#ifdef useGrayAccel
  #include "utility/MPU9250.h"
  MPU9250 IMU;
  const String TelloJackName=" TelloJack Accel ";
  const int TitleBannerColor=TFT_ORANGE;
  const String XaxisName=" Acc-X : ";
  const String YaxisName=" Acc-Y : ";
#endif //useGrayAccel

//M5Stack Basic or Gray + Faces Joystick
#ifdef useFacesJoyStick
  #define FACE_JOY_ADDR 0x5e
  const String TelloJackName=" TelloJack Faces JS ";
  const int TitleBannerColor=TFT_CYAN;
  const String XaxisName=" Joy-X : ";
  const String YaxisName=" Joy-Y : ";
#endif //useFacesJoyStick

//M5Stack Basic or Gray + Faces Cross
#ifdef useFacesKey
  #define KEYBOARD_I2C_ADDR     0x08
  #define KEYBOARD_INT          5
  const String TelloJackName=" TelloJack FACES ";
  const int TitleBannerColor=TFT_PINK;
  const String XaxisName=" Crs-X : ";
  const String YaxisName=" Crs-Y : ";
  int plusSpeed=0;
#endif //useFacesKey

//M5Stack fire用
#ifdef ARDUINO_M5STACK_FIRE
  const int TitleBannerColor=TFT_RED;
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// M5StickC
#if defined(useM5CStick)
  #if defined(useM5CJoyS)
    #define JOY_ADDR 0x52 //useM5CJoyS
  #elif defined(useHATStick) || defined(useHATJoyC)
    #define JOY_ADDR 0x38
  #endif
  
  const String TelloJackName="TelloJack JS";
  const int TitleBannerColor=TFT_MAROON;
  const String XaxisName=" X:";
  const String YaxisName=" Y:";
  const float jsr=1024.0/250.0;

  #if defined(useHATJoyC)
    #include "JoyC.h"
    JoyC joyc;
    TFT_eSprite img = TFT_eSprite(&M5.Lcd);
    const float stickRetio=1.28f;
  #endif
#endif //useM5CStick

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Odroid-GO crosskey
#ifdef ARDUINO_ODROID_ESP32
  const String TelloJackName=" TelloJack GO ";
  const int TitleBannerColor=TFT_MAGENTA;
  const String XaxisName=" Crs-X : ";
  const String YaxisName=" Crs-Y : ";
  int plusSpeed=0;
#endif //ARDUINO_ODROID_ESP32


void print_msg(String status_msg );
void print_msg(String status_msg, int c);
bool loadApData(fs::FS &fs, const char * path);

// TELLO SSID
//const char* TELLO_SSID = "TELLO-XXXXXX";// Replce your own Wi-Fi SSID of Tello

// UDP
WiFiUDP Udp;
const char* TELLO_IP = "192.168.10.1"; // TELLO Command IP
const int PORT = 8889; // TELLO_PORT
/*
WiFiUDP Sdp;
const char* Status_IP = "0.0.0.0"; //Status IP
const int Status_PORT = 8890; //StatusPORT
*/

char packetBuffer[256];
String message = "";

float xp=-100.0;
float yp=-100.0;
float xxx=0.0;
float yyy=0.0;
float rrr=0.0;
float uuu=0.0;
char msgx[10];
char msgy[10];
String status_msg;

// for rc command string
char command_str[20];

//Stick Value 
#ifdef ARDUINO_M5StickC_ESP32
  const int svxx=55;
  const int svyx=135;
  const int svy=28;
  const int svx=28;
#else
  const int svxx=82;
  const int svyx=298;
  const int svy=35;
  const int svx=35;
#endif

//For read passowrd of TELLO SSID
#include <FS.h>
#define SSIDMAX 30
typedef struct SSIDDATA {
    char val;
    String ssid;
};
SSIDDATA tellos[SSIDMAX];

String selectTello="";
String selectPass="";
int telloTgt,telloTotal;

//*********************************************************
//Quick Sort: Sort SSIDs whose names start with TELLO first
//クイックソート:TELLOから始まるSSIDが最初にくるようにソート
//*********************************************************

int pivot(int i, int j) {
  int k = i + 1;
  while (k <= j && tellos[i].val == tellos[k].val) k++;
  if (k > j) return -1;
  if (tellos[i].val >= tellos[k].val) return i;
  return k;
}
int partition(int i, int j, int x) {
  int l = i, r = j;
  while (l <= r) {
    while (l <= j && tellos[l].val < x)  l++;
    while (r >= i && tellos[r].val >= x) r--;
    if (l > r) break;
    String t = tellos[l].ssid;
    char val = tellos[l].val;
    tellos[l].ssid = tellos[r].ssid;
    tellos[l].val = tellos[r].val;
    tellos[r].ssid = t;
    tellos[r].val = val;
    l++; r--;
  }
  return l;
}
void quickSort(int i, int j) {
  if (i == j) return;
  int p = pivot(i, j);
  if (p != -1) {
    int k = partition(i, j, tellos[p].val);
    quickSort(i, k - 1);
    quickSort(k, j);
  }
}
void SortSSID() {
  quickSort(0, telloTotal - 1);
}

//***************************************
// Make Wi-Fi SSID LIST
// Wi-Fi SSID リストを作る
//***************************************

void ScanWifiList() {
  telloTgt=0;
  telloTotal=0; //Number of tello

  print_msg("Wi-Fi Scanning ... ",TFT_YELLOW);
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks(); 
  if (n == 0) {
    print_msg("'No networks found ...", TFT_RED);
  } else {
    for (int i=0; i<SSIDMAX; i++) { tellos[i].ssid=""; }//clear
    for (int i = 0; i < n; ++i) {
      if(telloTotal<SSIDMAX) {
        String ssid=WiFi.SSID(i);
        tellos[telloTotal].ssid=ssid;
        if(ssid.startsWith("TELLO")||ssid.startsWith("Tello")||ssid.startsWith("tello")) {
          tellos[telloTotal].val=0;//Telloが最初に来るように
        } else {
          tellos[telloTotal].val=(char)ssid.c_str()[0];
        }
        telloTotal+=1;
      }
    }
  }
  //return telloTotal;
  if(telloTotal>0) {  
    SortSSID();//ソート
    drawWiFiList();//表示
  } else {
    print_msg("Couldn't find any Tello...");    
  }
}

//***************************************
// Display the target SSID name
//***************************************

void drawWiFiList() {
  if(telloTotal>0) {
    selectTello=tellos[telloTgt].ssid;
    print_msg(String(telloTgt+1)+"/"+String(telloTotal)+" "+selectTello);
  } else {
    //print_msg("");
  }
}

void drawWiFiList(int n) {
  if(telloTotal>0) {
    telloTgt+=n;
    if(telloTotal<=telloTgt) telloTgt=0;
    else if(telloTgt<0) telloTgt=telloTotal-1;
    drawWiFiList();
  }
}

#ifdef ARDUINO_M5StickC_ESP32

String getAP(String ssid) {
  Preferences preferences;
  preferences.begin("wifi-config");
  String pass=preferences.getString(ssid.c_str());
  preferences.end();
  return pass;
}

void putAP(String ssid, String pass) {
  Preferences preferences;
  preferences.begin("wifi-config");
  preferences.putString(ssid.c_str(), pass.c_str());
  preferences.end();
}

int tgtchrnum=0x61;//a
String selpwd="";
String selpwdr="";
int selWifiMode=0;

void drawWiFiPwd(int c) {
  print_msg("PWD: "+selpwd+selpwdr,c);
}

/*
void del1ChrPwd() {
  if(selpwdr=="") {
    int n=selpwd.length();
    if(n<=1) {
      selpwd="";
    } else {
      selpwd=selpwd.substring(0,n);
    }
  } else {
    selpwdr="";
  }
  drawWiFiPwd(TFT_RED);
}

void nxtPrvChrPwd(int n) {
  tgtchrnum+=n;
  if(tgtchrnum<0x21) {
    tgtchrnum=0x7E;
  } else if(tgtchrnum>127) {
    tgtchrnum=0x21;
  }
  selpwdr=String(x);
  drawWiFiPwd(TFT_RED);
}

void add1ChrPwd() {
  selpwd=selpwd+selpwdr;
  selpwdr="";
  drawWiFiPwd(TFT_RED);
}
*/

void getPwd(int c) {
  selpwd="";
  if(telloTotal>0) {
    selectTello=tellos[telloTgt].ssid;
    selpwd=getAP(selectTello);
  }
  drawWiFiPwd(c);
}

void setPwd(int c) {
  selpwd=selpwd+selpwdr;
  putAP(selectTello,selpwd);
  getPwd(c);
}

#endif

//***************************************
// Get password from file
// ファイルからpasswordを取得
//***************************************

bool loadApData(fs::FS &fs, const char * path) {
  selectPass="";
 
  //Serial.printf("Reading file: %s\n", path);
  File file = fs.open(path);
  if(!file) {
    Serial.println("Failed to open file for reading");
    //print_msg("Failed to open file for reading");
    return false;
  }
  
  char  v;
  while(file.available())
  {
    v=(char)file.read();
    if(v==','||v==0x09||v==0x0A||v==0x0D) {
      break;
    } else {
      selectPass.concat(v);
    }
  }
  file.close();
  Serial.println(selectPass);
  return true;
}

//ファイルからSSIDとpasswordを取得して、接続してみる
void try_Connect_to_Tello() { //{ String filename)
  if(telloTotal<=0) return; 
  //Serial.println(String(selectTello+".txt"));
  #if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_ODROID_ESP32)
    if(!loadApData(SD,(const char*)String("/"+selectTello).c_str())) {
      Serial.println(String(selectTello+".txt"));
      loadApData(SD,(const char*)String("/"+selectTello+".txt").c_str());
    }
  #else
    selectPass=getAP(selectTello);
  #endif
  //Serial.println(selectPass);
  
  //WiFi通信の開始
  WiFi.begin(selectTello.c_str(),selectPass.c_str());
  //WiFi接続　接続するまでループ
  int64_t timeout = millis() + 10000;//8秒
  while (WiFi.status() != WL_CONNECTED && timeout - millis() > 0 ) {    
    print_msg(selectTello+" connecting...");//print_msg("Now, WiFi Connecting......");
    delay(500);
  }
   
  if(WiFi.status()==WL_CONNECTED) {
    //WiFi Connected.
    print_msg("WiFi Connected.");
    
    // UDP
    Udp.begin(PORT);
    //Telloへ”command”送信
    print_msg("Connected...");
    tello_command_exec("command");
    
    //方向インターフェイスカラー変更s
    drawConnectDisconnect(true);
    
    #ifdef useFacesJoyStick
    //ジョイスティック点減
    randomLED();randomLED();randomLED();
    #endif //useFacesJoyStick
    
    print_msg("Connected to "+selectTello);
    
  } else {
    print_msg("Connect failure...",TFT_RED);
  }
    
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Multi task
/*
void Read_Tello_Status(void *args) {
  int packetSize;
  for (;;) {
    //tello_command_exec("battery?");
    packetSize = Sdp.parsePacket();
    if(packetSize) {
      //IPAddress remoteIp = Sdp.remoteIP();
      int len = Udp.read(packetBuffer, 255);
      if (len > 0) { 
        packetBuffer[len] = 0;
        print_msg(String((char*)packetBuffer));
      }
    }
    vTaskDelay(1000);
  }
}
*/
void setMultiTask() {
/*
  //xTaskCreatePinnedToCore
    xTaskCreatePinnedToCore(
                    Read_Tello_Status,     // Function to implement the task
                    "Read_Tello_Status",   // Name of the task 
                    4096,                 // Stack size in words 
                    NULL,      // Task input parameter 
                    1,         // Priority of the task 
                    NULL,      // Task handle. 
                    0);        // Core where the task should run 

*/
}


//--------------------------------------------
//--------------------------------------------
//--------------------------------------------

//--------------------------------------------
//  JoyStick of Faces
//--------------------------------------------

#ifdef useFacesJoyStick

// Connect!
void randomLED() {
  for (int i = 0; i < 256; i++)
  {
    Wire.beginTransmission(FACE_JOY_ADDR);
    Wire.write(i % 4);
    Wire.write(random(256) * (256 - i) / 256);
    Wire.write(random(256) * (256 - i) / 256);
    Wire.write(random(256) * (256 - i) / 256);
    Wire.endTransmission();
    delay(2);
  }
  Led(0, 0, 0, 0);
  Led(1, 0, 0, 0);
  Led(2, 0, 0, 0);
  Led(3, 0, 0, 0);  
}

// Joystick点灯制御
void Led(int indexOfLED, int r, int g, int b){
  Wire.beginTransmission(FACE_JOY_ADDR);
  Wire.write(indexOfLED);
  Wire.write(r);
  Wire.write(g);
  Wire.write(b);
  Wire.endTransmission();
}

#endif //useFacesJoyStick

void drawInterface() {
  //画面表示
  //---タイトル
  #if defined(ARDUINO_M5StickC_ESP32)
    M5.Lcd.fillRect(0,0,lcdW,22,TitleBannerColor);
    M5.Lcd.drawCentreString(TelloJackName,80,3,2);

    //---方向矢印
    drawConnectDisconnect(false);
    //---方向の文字
    //drawButtonName(1);
    //---スティックの値
    //drawStickValue(false);
  
    //---メッセージ領域
    M5.Lcd.drawRoundRect(1,50,lcdW-2,30,4,TFT_WHITE);
    //---メッセージのタイトル文字
    M5.Lcd.setTextColor(TFT_CYAN,TFT_BLACK);
    //M5.Lcd.drawCentreString("<Msg>",25,45,1);

  #else
    M5.Lcd.fillRect(0,0,lcdW,30,TitleBannerColor);
    M5.Lcd.drawCentreString(TelloJackName,160,2,4);

    //---方向矢印
    drawConnectDisconnect(false);
    //---方向の文字
    //drawButtonName(1);
    //---スティックの値
    //drawStickValue(false);
  
    //---メッセージ領域
    M5.Lcd.drawRoundRect(0,180,lcdW-1,30,4,TFT_WHITE);
    //---メッセージのタイトル文字
    M5.Lcd.setTextColor(TFT_CYAN,TFT_DARKGREEN);
    M5.Lcd.drawCentreString("<Message>",38,170,2);
    
  #endif

}

bool prvdrw=true; //前描画状況
  
#ifdef ARDUINO_ODROID_ESP32
const int ckposx=-80;
const int ckposy=13;
#else
const int ckposx=0;
const int ckposy=0;
#endif //ARDUINO_ODROID_ESP32

#ifdef ARDUINO_ODROID_ESP32
void drawABbutton(int ab, bool b) {
  //---A,B button
  int c=TFT_LIGHTGREY;
  if(b) c=TFT_GREEN; //push!
  if(ab==0) {//A
    M5.Lcd.fillEllipse(280,110,22,22,c);//Forwaod
    M5.Lcd.setTextColor(TFT_BLACK,c);
    M5.Lcd.drawCentreString("F",280,100,4);
  } else {
    M5.Lcd.fillEllipse(220,130,22,22,c);//Back
    M5.Lcd.setTextColor(TFT_BLACK,c);
    M5.Lcd.drawCentreString("B",220,120,4);
  }
}

void drawCrossKey(int ulrud, bool b) {
  //---A,B button
  int c=TFT_BLACK;
  if(b) c=TFT_GREEN; //push!

  M5.Lcd.setTextColor(c,TFT_LIGHTGREY);
  switch(ulrud) {
    case 0: M5.Lcd.drawCentreString("U",160+ckposx,64+ckposy,2); break;
    case 1: M5.Lcd.drawCentreString("D",160+ckposx,120+ckposy,2); break;
    case 2: M5.Lcd.drawCentreString("L",120+ckposx,92+ckposy,2); break;
    case 3: M5.Lcd.drawCentreString("R",200+ckposx,92+ckposy,2); break;
  }
}

#endif //ARDUINO_ODROID_ESP32

void drawConnectDisconnect(bool b) {
  if(b==prvdrw) return;

  //---X, Y 値
  drawStickValue(b);
  
  int c=TFT_LIGHTGREY;
  if(b) c=TFT_GREEN;
  
  #if defined(ARDUINO_ODROID_ESP32)
  #elif defined(ARDUINO_M5StickC_ESP32)
  #else
    //---方向矢印
    M5.Lcd.fillTriangle(159+ckposx,40+ckposy,189+ckposx,60+ckposy,129+ckposx,60+ckposy,c);
    M5.Lcd.fillTriangle(159+ckposx,160+ckposy,189+ckposx,140+ckposy,129+ckposx,140+ckposy,c);
    M5.Lcd.fillTriangle(269+ckposx,100+ckposy,220+ckposx,80+ckposy,220+ckposx,120+ckposy,c);
    M5.Lcd.fillTriangle(98+ckposx,80+ckposy,98+ckposx,120+ckposy,49+ckposx,100+ckposy,c);
  #endif
  
  #ifdef ARDUINO_ODROID_ESP32
  //---A,B button
  drawABbutton(0,false);
  drawABbutton(1,false);
  #endif //ARDUINO_ODROID_ESP32
  
  //---方向名文字
  int n=(b==true)?1:-1;
  drawButtonName(n);
  
  //---ボタンエリア
  M5.Lcd.fillRect(0,217,lcdW,20,TFT_LIGHTGREY);

  //---ボタン文字
  if(b) {
    #if defined(ARDUINO_ODROID_ESP32)
      M5.Lcd.setTextColor(TFT_BLACK,TFT_RED);
      M5.Lcd.drawCentreString(" LANDING ",5,220,2);
      M5.Lcd.setTextColor(TFT_BLACK,TFT_MAGENTA);
      M5.Lcd.drawCentreString(" <<- CCW ",100,220,2);
      M5.Lcd.setTextColor(TFT_BLACK,TFT_CYAN);
      M5.Lcd.drawCentreString(" CW ->> ",215,220,2);
      M5.Lcd.setTextColor(TFT_BLACK,TFT_YELLOW);
      M5.Lcd.drawCentreString(" TAKE OFF ",300,220,2);

    #elif defined(ARDUINO_M5StickC_ESP32)
    
    #else
      M5.Lcd.setTextColor(TFT_BLACK,TFT_YELLOW);
      M5.Lcd.drawCentreString(" TAKE OFF ",64,220,2);
      
      #if defined(useJoyStick) || defined(useFacesJoyStick) || defined(useGrayAccel)
        M5.Lcd.setTextColor(TFT_BLACK,TFT_CYAN);
        M5.Lcd.drawCentreString("CW/CCW_U/D",160,220,2);
      #endif //defined(useJoyStick) || defined(useFacesJoyStick) || defined(useGrayAccel)
      
      M5.Lcd.setTextColor(TFT_BLACK,TFT_RED);
      M5.Lcd.drawCentreString(" LANDING  ",250,220,2);
    
    #endif
    
  } else {
    #if defined(ARDUINO_ODROID_ESP32)
      M5.Lcd.setTextColor(TFT_BLACK,TFT_YELLOW);
      M5.Lcd.drawCentreString(" PREV ",5,220,2);
      M5.Lcd.setTextColor(TFT_BLACK,TFT_GREEN);
      M5.Lcd.drawCentreString(" NEXT ",95,220,2);
      M5.Lcd.setTextColor(TFT_BLACK,TFT_CYAN);
      M5.Lcd.drawCentreString(" CONNECT ",220,220,2);
      M5.Lcd.setTextColor(TFT_BLACK,TFT_MAGENTA);
      M5.Lcd.drawCentreString(" SCAN ",300,220,2);
    
    #elif defined(ARDUINO_M5StickC_ESP32)
    
    #else
      M5.Lcd.setTextColor(TFT_BLACK,TFT_GREEN);
      M5.Lcd.drawCentreString(" NEXT ",64,220,2);
      M5.Lcd.setTextColor(TFT_BLACK,TFT_CYAN);
      M5.Lcd.drawCentreString(" CONNECT ",160,220,2);
      M5.Lcd.setTextColor(TFT_BLACK,TFT_MAGENTA);
      M5.Lcd.drawCentreString(" SCAN ",250,220,2);
    
    #endif
    //print_msg("Please select A, B or C button!",TFT_YELLOW);
    ScanWifiList();
  }
  prvdrw=b;
}

int drawButtonName(int n) {
  // アナログスティック押し込みボタン処理
  if( n == 0 ) {
     //---Zボタンを押しているとき 上昇下降/左右旋回
    #if defined(useFacesKey) || defined(ARDUINO_ODROID_ESP32)
      M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
      M5.Lcd.drawCentreString("  UP  ",160+ckposx,64+ckposy,2);
      M5.Lcd.drawCentreString("  DOWN  ",160+ckposx,120+ckposy,2);
    #elif defined(ARDUINO_M5StickC_ESP32)
    #else
      int c=TFT_GREEN;
      if(n<0) c=TFT_LIGHTGREY;
      M5.Lcd.setTextColor(TFT_BLACK,c);
      M5.Lcd.drawCentreString("D",160+ckposx,64+ckposy-20,2);
      M5.Lcd.drawCentreString("U",160+ckposx,120+ckposy+20,2);
      /*
      M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
      M5.Lcd.drawCentreString("  DOWN  ",160+ckposx,64+ckposy,2);
      M5.Lcd.drawCentreString("  UP  ",160+ckposx,120+ckposy,2);
      */
    #endif

    #if defined(ARDUINO_M5StickC_ESP32)
    #else
      M5.Lcd.drawCentreString("  ",120+ckposx-30,92+ckposy,2);
      M5.Lcd.drawCentreString("  ",200+ckposx+30,92+ckposy,2);
      M5.Lcd.drawCentreString("TL",120+ckposx-30,92+ckposy,2);
      M5.Lcd.drawCentreString("TR",200+ckposx+30,92+ckposy,2);
    /*
    M5.Lcd.drawCentreString(" CCW",120+ckposx,92+ckposy,2);
    M5.Lcd.drawCentreString("  CW ",200+ckposx,92+ckposy,2);
    */
    #endif
    
  } else {
    //---押していないとき 通常表示
    #ifdef ARDUINO_ODROID_ESP32
      //Drawing crosskey
      M5.Lcd.fillRoundRect(120+ckposx-17,92+ckposy-7,116,30,4,TFT_LIGHTGREY);//yoko
      M5.Lcd.fillRoundRect(160+ckposx-15,64+ckposy-8,30,86,4,TFT_LIGHTGREY);//tate
    #endif
    
    #ifdef ARDUINO_ODROID_ESP32
      drawCrossKey(0,false);
      drawCrossKey(1,false);
      drawCrossKey(2,false);
      drawCrossKey(3,false);

    #elif defined(ARDUINO_M5StickC_ESP32)
    #else
      int c=TFT_GREEN;
      if(n<0) c=TFT_LIGHTGREY;
      
      M5.Lcd.setTextColor(TFT_BLACK,c);
      M5.Lcd.drawCentreString("F",160+ckposx,64+ckposy-20,2);
      M5.Lcd.drawCentreString("B",160+ckposx,120+ckposy+20,2);
      M5.Lcd.drawCentreString("  ",120+ckposx-30,92+ckposy,2);
      M5.Lcd.drawCentreString("  ",200+ckposx+30,92+ckposy,2);
      M5.Lcd.drawCentreString("L",120+ckposx-30,92+ckposy,2);
      M5.Lcd.drawCentreString("R",200+ckposx+30,92+ckposy,2);
      /*
      M5.Lcd.setTextColor(c,TFT_BLACK);
      M5.Lcd.drawCentreString("FORWARD",160+ckposx,64+ckposy,2);
      M5.Lcd.drawCentreString("BACK",160+ckposx,120+ckposy,2);
      M5.Lcd.drawCentreString("LEFT",120+ckposx,92+ckposy,2);
      M5.Lcd.drawCentreString("RIGHT",200+ckposx,92+ckposy,2);
      */
  
      M5.Lcd.fillEllipse(160+xp*40,100-yp*26,15,15,TFT_BLACK);
      M5.Lcd.fillEllipse(160,100,15,15,TFT_LIGHTGREY);
      xp=yp=0;
    
    #endif

  }
  return n;
}

void drawStickValue(bool b) {
  //---X, Yの表示
  int c=TFT_LIGHTGREY;
  if(b) c=TFT_YELLOW;

  #if defined(ARDUINO_ODROID_ESP32)
    M5.Lcd.setTextColor(c,TFT_BLACK);
  
    M5.Lcd.drawString("L/R=",5,svy,2);
    sprintf(msgx," %-2.2f   ",xxx);
    M5.Lcd.drawString(msgx,svx,svy,2);
  
    M5.Lcd.drawString("U/D=",85,svy,2);
    sprintf(msgy," %-2.2f   ",uuu);
    M5.Lcd.drawString(msgy,svx+80,svy,2);
  
    M5.Lcd.drawString("ROT=:",165,svy,2);
    sprintf(msgy," %-2.2f   ",rrr);
    M5.Lcd.drawString(msgy,svx+160,svy,2);
  
    M5.Lcd.drawString("F/B=",245,svy,2);
    sprintf(msgy," %-2.2f   ",yyy);
    M5.Lcd.drawString(msgy,svx+lcdH,svy,2);

  #elif defined(ARDUINO_M5StickC_ESP32)
    M5.Lcd.setTextColor(c,TFT_BLACK);
    #ifdef useHATJoyC
    #else
      M5.Lcd.drawCentreString(XaxisName,10,svy,2);
      sprintf(msgx," %-2.2f   ",xxx);
      M5.Lcd.drawCentreString(msgx,svxx,svy,2);
      //---X, Y値の表示
      M5.Lcd.drawCentreString(YaxisName,90,svy,2);
      sprintf(msgy," %-2.2f   ",yyy);
      M5.Lcd.drawCentreString(msgy,svyx,svy,2);
    #endif
  #else
    M5.Lcd.setTextColor(c,TFT_BLACK);
    M5.Lcd.drawCentreString(XaxisName,20,svy,2);
    sprintf(msgx," %-2.2f   ",xxx);
    M5.Lcd.drawCentreString(msgx,svxx,svy,2);
    //---X, Y値の表示
    M5.Lcd.drawCentreString(YaxisName,lcdH,svy,2);
    sprintf(msgy," %-2.2f   ",yyy);
    M5.Lcd.drawCentreString(msgy,svyx,svy,2);

  #endif
}

//--------------------------------------------
//--------------------------------------------
// Set up function
// 初期化関数
//--------------------------------------------
//--------------------------------------------

void setup() {
  
  //M5Stack initialization
  M5.begin();

  //Initialization of JoyStick unit
  //ジョイスティックの初期設定
  #if defined(useJoyStick)
    Wire.begin(21, 22, 400000);
  #elif defined(useM5CStick) // || defined(useHATJoyC)
    #if defined(useHATStick) || defined(useHATJoyC)
      Wire.begin(0, 26, 100000);
    #else //defined(useM5CJoyS)
      Wire.begin(32, 33, 400000);
    #endif
    M5.Lcd.setRotation(1);
    M5.Lcd.fillRect(0, 0, 80, 160, BLACK);
    #if defined(useHATJoyC)
      img.createSprite(80, 160);
    #endif
  #else
    Wire.begin();
  #endif //useJoyStick

  #if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_ODROID_ESP32)
    //for SD Uploader
    if (digitalRead(BUTTON_A_PIN) == 0) {
      Serial.println("Will Load menu binary");
      updateFromFS(SD);
      ESP.restart();
    }

    //Access for microSD
    if(!SD.begin()) Serial.println("File IO failed...");
  #endif
  
  //disable the speak noise
  //dacWrite(25, 0);
  
  //Draw interface
  //インターフェイスの描画
  drawInterface();

  //LED flashing of JoyStick FACES
  //ジョイスティック用
  #ifdef useFacesJoyStick  
    //ジョイスティック点減
    randomLED();
  #endif //useFacesJoyStick

  //Initization of M5Stack Gray accelerator
  #ifdef useGrayAccel
    //MPU9250を初期化
    IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);
    IMU.initMPU9250();
  #endif //useGrayAccel

  //Initialization for FACES Keyboard
  #ifdef useFacesKey
    //Facesのキーボードを使っている場合
    pinMode(KEYBOARD_INT, INPUT_PULLUP); 
  #endif //useFacesKey

  //MultiTask
  setMultiTask();

  delay(2000);
}

//--------------------------------------------
//--------------------------------------------

uint16_t x_data;
uint16_t y_data;
uint8_t button_data;
uint8_t button_prev_data=1;

#ifdef ARDUINO_ODROID_ESP32
  const uint16_t plusCtrVal=512+400;
  const uint16_t minusCtrVal=512-400;
#endif

uint16_t u_data;
uint16_t r_data;
uint8_t button_left;

//ジョイスティック用
#ifdef useFacesJoyStick  
  // FACE Joystick用変数
  uint8_t x_data_L;
  uint8_t x_data_H;
  uint8_t y_data_L;
  uint8_t y_data_H;
  char data[100];
#endif //useFacesJoyStick

//--------------------------------------------
//--------------------------------------------
// ループ関数
//--------------------------------------------
//--------------------------------------------

unsigned long noOpr=millis();
bool takeOff=false;
void DeepSleepPowerOff() {
  if(takeOff==true) {
    noOpr=millis();
  } else {
    //Auto power off
    if(noOpr==0) noOpr=millis();
    if (300000<millis()-noOpr) {
      #if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_ODROID_ESP32)
        //Serial.println( "Power OFF" );
        M5.powerOFF();
        /*
        M5.Lcd.setBrightness(0);
        M5.Lcd.sleep();
        esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_B_PIN, LOW);
        esp_deep_sleep_start();
        */
      #else
      #endif
    }
  }
}

void loop() {
  
  //接続状況
  bool wfc=(WiFi.status()==WL_CONNECTED);
  if(wfc) {
    //***************************
    // 接続の場合
    //***************************

    //アクセラレーター
    #ifdef useGrayAccel
      //x,y値の取得と表示
      if (IMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01){
        IMU.readAccelData(IMU.accelCount);
        IMU.getAres();
        xxx = IMU.accelCount[0] * IMU.aRes;
        yyy = IMU.accelCount[1] * IMU.aRes;
        //Serial.println(xxx);
      }
    #endif //useGrayAccel

    //ジョイスティック
    #if defined(useJoyStick) || defined(useM5CJoyS) || defined(useHATStick)
      #if defined(useHATStick) || defined(useM5CJoyS)
      Wire.beginTransmission(JOY_ADDR);
      Wire.write(0x02); 
      Wire.endTransmission();
      #endif
      Wire.requestFrom(JOY_ADDR, 3);
      if (Wire.available()) {
        x_data = Wire.read(); //10-250
        y_data = Wire.read(); //10-250
        // Zボタン(1:押している時 0:離している時 )
        button_data=(Wire.read()==0)?1:0;
        
        x_data = (uint16_t)(float)x_data*jsr;//(64.0/15.0)-(128.0/3.0);
        y_data = (uint16_t)(float)y_data*jsr;//(64.0/15.0)-(128.0/3.0);
  
      }
    #endif //useJoyStick

    //HAT用JoyC
    #ifdef useHATJoyC
      r_data=joyc.GetX(0)*stickRetio;
      u_data=joyc.GetY(0)*stickRetio;
      x_data=joyc.GetX(1)*stickRetio;
      y_data=joyc.GetY(1)*stickRetio;
      button_left=joyc.GetPress(0);
      button_data=joyc.GetPress(1);
      r_data = (uint16_t)(float)r_data*jsr;//(64.0/15.0)-(128.0/3.0);
      u_data = (uint16_t)(float)u_data*jsr;//(64.0/15.0)-(128.0/3.0);
      x_data = (uint16_t)(float)x_data*jsr;//(64.0/15.0)-(128.0/3.0);
      y_data = (uint16_t)(float)y_data*jsr;//(64.0/15.0)-(128.0/3.0);
    #endif

    //フェイス用ジョイスティック
    #ifdef useFacesJoyStick  
      // Joystick値の取得
      Wire.requestFrom(FACE_JOY_ADDR, 5);
      if (Wire.available()) {
    
        y_data_L = Wire.read();
        y_data_H = Wire.read();
        x_data_L = Wire.read();
        x_data_H = Wire.read();
    
        // Zボタン(0:押している時 1:離している時 )
        button_data = Wire.read();
    
        x_data = x_data_H << 8 |x_data_L;
        y_data = y_data_H << 8 |y_data_L;
      }
    #endif //useFacesJoyStick

    //フェイス用キーボード
    #ifdef useFacesKey
      //Underconstruction !
      x_data=y_data=512;
      button_data=1;
      plusSpeed=0;
      if (digitalRead(KEYBOARD_INT) == LOW) {
        Wire.requestFrom(KEYBOARD_I2C_ADDR, 1);  // request 1 byte from keyboard
        while (Wire.available()) {
          doFacesKey(Wire.read()); // receive a byte as character
        }
        if(x_data>512) x_data+=plusSpeed;
        else if(x_data<512) x_data-=plusSpeed;
        if(y_data>512) y_data+=plusSpeed;
        else if(y_data<512) y_data-=plusSpeed;
        
      }
    #endif //useFacesKey

    //-------------------------------------------
    //オドロイドの場合
    #if defined(ARDUINO_ODROID_ESP32)
      x_data=y_data=u_data=r_data=512;
      if(M5.BtnA.isPressed()) { y_data=plusCtrVal; drawABbutton(0,true); }//Forward
      else if(M5.BtnB.isPressed()) { y_data=minusCtrVal; drawABbutton(1,true); }//Back
      else if(M5.BtnA.wasReleased()) { drawABbutton(0,false); }//Forward
      else if(M5.BtnB.wasReleased()) { drawABbutton(1,false); }//Back
  
      if(M5.JOY_Y.isAxisPressed() == 2) { u_data=plusCtrVal; drawCrossKey(0,true); } //Up
      else if(M5.JOY_Y.isAxisPressed() == 1) { u_data=minusCtrVal; drawCrossKey(1,true); } //Down
      else if(M5.JOY_Y.wasReleased()) { drawCrossKey(0,false); drawCrossKey(1,false); }//Up,down
   
      if(M5.JOY_X.isAxisPressed() == 2) { x_data=minusCtrVal; drawCrossKey(2,true); } //Left
      else if(M5.JOY_X.isAxisPressed() == 1) { x_data=plusCtrVal; drawCrossKey(3,true); } //Right
      else if(M5.JOY_X.wasReleased()) { drawCrossKey(2,false); drawCrossKey(3,false); }//Left, right
  
      if(M5.BtnSelect.isPressed()) r_data=plusCtrVal;//cw
      else if(M5.BtnVolume.isPressed()) r_data=minusCtrVal;//ccw
      
      // 0〜1024を±1.0に正規化→本家プログラムと同じ形式に
      xxx = float(x_data-512) / 512.0;
      yyy = float(y_data-512) / 512.0;   
      rrr = float(r_data-512) / 512.0;
      uuu = float(u_data-512) / 512.0;
  
      M5.Lcd.setTextColor(TFT_YELLOW,TFT_BLACK);
      sprintf(msgx," %-2.2f ",xxx); M5.Lcd.drawString(msgx,30,svy,2);
      sprintf(msgy," %-2.2f ",uuu); M5.Lcd.drawString(msgy,110,svy,2);
      sprintf(msgx," %-2.2f ",rrr); M5.Lcd.drawString(msgx,190,svy,2);
      sprintf(msgy," %-2.2f ",yyy); M5.Lcd.drawString(msgy,270,svy,2);
      //print_msg("Operation Start!");
  
      //Takeoff > ボタンA処理 > START
      if(M5.BtnStart.wasPressed() ) { //離陸
        noOpr=millis();//print_msg("TAKE OFF"); 
        tello_command_exec("takeoff");
        takeOff=true;
      }
      //Land > ボタンC処理 > Select   
      if(M5.BtnMenu.wasPressed()) {//着陸
        noOpr=millis();//print_msg("LAND");
        tello_command_exec("land");
        takeOff=false;
      }
  
      // Joystick入力に応じたコマンド送信
      if (fabs(xxx)> 0.3 || fabs(yyy)> 0.3 || fabs(uuu)> 0.3 || fabs(rrr)> 0.3) {
        xxx=min(100,max(-100,(int)(xxx*100))); yyy=min(100,max(-100,(int)(yyy*100)));
        uuu=min(100,max(-100,(int)(uuu*100))); rrr=min(100,max(-100,(int)(rrr*100)));
        sprintf(command_str,"rc %d %d %d %d",int(xxx), int(yyy), int(uuu), int(rrr) ); // 上昇下降と旋回
        tello_command_exec(command_str);  // rcコマンドを送信
      } else {
        // 傾いていない時は停止命令を送信し続ける
        tello_command_exec("rc 0 0 0 0");//lr,fb,ud,cw
      }

    #else
    //-------------------------------------------
    
      //X,Y換算
      #if defined(useJoyStick) || defined(useFacesJoyStick) || defined(useFacesKey) || defined(useM5CStick)
        // 0〜1024を±1.0に正規化→本家プログラムと同じ形式に
        xxx = float(x_data-512) / 512.0;
        yyy = float(y_data-512) / 512.0;
      #endif //
      #if defined(useHATJoyC)
        rrr = float(r_data-512) / 512.0;
        uuu = float(u_data-512) / 512.0;
      #endif

      #if defined(useM5CStick) //defined(useM5CStick) || defined(useHATJoyC)
      #else
        if((xxx<xp-0.03||xxx>xp+0.03)||(yyy<yp-0.03||yyy>yp+0.03)) { 
          M5.Lcd.fillEllipse(160+xp*40,100-yp*26,15,15,TFT_BLACK);
          M5.Lcd.fillEllipse(160+xxx*40,100-yyy*26,15,15,TFT_GREEN);
          xp=xxx;yp=yyy;
        }
      #endif
      
      #ifdef useHATJoyC
        M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
        sprintf(msgx,"LR: %-2.2f  ",xxx);
        M5.Lcd.drawCentreString(msgx,svxx,svy,1);//drawCentreString
        sprintf(msgy,"FB: %-2.2f  ",yyy);
        M5.Lcd.drawCentreString(msgy,svyx,svy,1);
        sprintf(msgx,"RO: %-2.2f  ",rrr);
        M5.Lcd.drawCentreString(msgx,svxx,svy+10,1);//drawCentreString
        sprintf(msgy,"UD: %-2.2f  ",uuu);
        M5.Lcd.drawCentreString(msgy,svyx,svy+10,1);
      #else
        M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
        sprintf(msgx," %-2.2f  ",xxx);//M5.Lcd.drawCentreString("      ",88,33,2);
        M5.Lcd.drawCentreString(msgx,svxx,svy,2);//drawCentreString
        sprintf(msgy," %-2.2f  ",yyy);//M5.Lcd.drawCentreString("      ",294,33,2);
        M5.Lcd.drawCentreString(msgy,svyx,svy,2);
      #endif
      
      //print_msg("Operation Start!");
      
      #if defined(useGrayAccel) || defined(useJoyStick) || defined(useFacesJoyStick)
        //ボタンに応じた処理
        // ボタンA処理
        if(M5.BtnA.wasPressed()) {
          //離陸
          //print_msg("TAKE OFF"); 
          tello_command_exec("takeoff");
          takeOff=true;
        }
        //ボタンB処理
        if(M5.BtnB.isPressed()) {
          //print_msg("CW/CCW/UP/DOWM");
          //tello_command_exec("cw 45");
          button_data=0;
        }
        if(M5.BtnB.wasReleased()) {
          button_data=1;
        }
        // ボタンC処理    
        if(M5.BtnC.wasPressed()) {
          //着陸
          noOpr=millis();//print_msg("LAND");
          tello_command_exec("land");
          takeOff=false;
        }         
      #endif //defined(useJoyStick) || defined(useFacesJoyStick) || defined(useGrayAccel)

      #if defined(useM5CJoyS) || defined(useHATStick)

        //離陸着陸
        if(button_data==0) {
          if(takeOff) {
            noOpr=millis();//print_msg("LAND");
            tello_command_exec("land");
            takeOff=false;
          } else {
            tello_command_exec("takeoff");
            takeOff=true;            
          }
          delay(500);
        }
        
        // ボタンA処理
        if(M5.BtnA.isPressed()) {//ボタンB処理
           if(takeOff) {
            //print_msg("CW/CCW/UP/DOWM");
            //tello_command_exec("cw 45");
            button_data=0;//0が離しているとき?
          }
        }
        // ボタンA処理
        if(M5.BtnA.wasPressed()) {
          if(takeOff) {
            button_data=1;//1が離しているとき?
          } else { 
            //離陸
            //print_msg("TAKE OFF");
            tello_command_exec("takeoff");
            takeOff=true;
          }
        }
        // ボタンB処理
        if(M5.BtnB.wasPressed()) {
          ;
        }
        // ボタンAxp処理
        if(M5.Axp.GetBtnPress()==2) {//着陸
          if(takeOff) {
            noOpr=millis();//print_msg("LAND");
            tello_command_exec("land");
            takeOff=false;
          } else {
            WiFi.disconnect();//udp.stop();
            ScanWifiList();
          }
        }
         
      #elif defined(useHATJoyC)
      
        //離陸着陸
        if(button_data||button_left) {
          if(takeOff) {
            noOpr=millis();//print_msg("LAND");
            tello_command_exec("land");
            takeOff=false;
          } else {
            tello_command_exec("takeoff");
            takeOff=true;            
          }
          delay(500);
        }

        // ボタンA処理
        if(M5.BtnA.wasPressed()) {
          if(takeOff) {
            noOpr=millis();//print_msg("LAND");
            tello_command_exec("land");
            takeOff=false;
          } else { 
            //離陸
            //print_msg("TAKE OFF");
            tello_command_exec("takeoff");
            takeOff=true;
          }
        }
        // ボタンB処理
        if(M5.BtnB.wasPressed()) {
          ;
        }
        // ボタンAxp処理
        if(M5.Axp.GetBtnPress()==2) {//着陸
          if(takeOff) {
            noOpr=millis();//print_msg("LAND");
            tello_command_exec("land");
            takeOff=false;
          } else {
            WiFi.disconnect();//udp.stop();
            ScanWifiList();
          }
        }
         
      #endif
      
      // アナログスティック押し込みボタン処理
      if( button_data != button_prev_data ) {
        //---Zボタンを押しているとき 上昇下降/左右旋回 = 0
        //---押していないとき 通常表示 = 1
        button_prev_data=drawButtonName(button_data);
      }

      if(takeOff) {
      // Joystick入力に応じたコマンド送信
        #ifdef useHATJoyC
        if (fabs(xxx)> 0.3 || fabs(yyy)> 0.3 || fabs(rrr)> 0.3 || fabs(uuu)> 0.3) {
          xxx=min(100,max(-100,(int)(-xxx*100))); yyy=min(100,max(-100,(int)(yyy*100)));
          uuu=min(100,max(-100,(int)(uuu*100))); rrr=min(100,max(-100,(int)(-rrr*100)));
          sprintf(command_str,"rc %d %d %d %d",(int)xxx,(int)yyy,(int)uuu,(int)rrr ); // 上昇下降と旋回
        #else
        if (fabs(xxx)> 0.3 || fabs(yyy)> 0.3) {
          // 傾きx,yに応じてrcコマンドの文字列を作成
          xxx=min(100,max(-100,(int)(xxx*100))); yyy=(int)min(100,max(-100,(int)(yyy*100)));
          //ボタンZ処理
          if( button_data == 0 ) {
              #if defined(useJoyStick) || defined(useM5CJoyS)
                sprintf(command_str,"rc 0 0 %d %d",(int)-yyy,(int)-xxx); // 上昇下降と旋回
              #else
                sprintf(command_str,"rc 0 0 %d %d",(int)-yyy,(int)xxx); // 上昇下降と旋回
              #endif
          } else {
              #if defined(useJoyStick) || defined(useM5CJoyS)
                sprintf(command_str,"rc %d %d 0 0",(int)-xxx,(int)-yyy); // 左右移動と前後進
              #else
                sprintf(command_str,"rc %d %d 0 0",(int)xxx,(int)yyy); // 左右移動と前後進
              #endif
              
          }
        #endif
          tello_command_exec(command_str);  // rcコマンドを送信
        } else {
          // 傾いていない時は停止命令を送信し続ける
          tello_command_exec("rc 0 0 0 0");
        }
      }
  
    #endif 
  
    #ifdef useFacesJoyStick
      //StickのLEDを点減
      LedFaceJoyStick();
    #endif //useFacesJoyStick
   
  } else {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // 未接続の場合
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~
    
    //ボタンの色
    drawConnectDisconnect(false);

    #if defined(ARDUINO_ODROID_ESP32)
      if(M5.BtnMenu.wasPressed())   { drawWiFiList(-1); } //Prev
      else if(M5.BtnVolume.wasPressed()) { drawWiFiList(1); } //Next
      else if(M5.JOY_X.wasAxisPressed() == 2) { drawWiFiList(-1); } //Left=Prev
      else if(M5.JOY_X.wasAxisPressed() == 1) { drawWiFiList(1); } //Right=Next
      else if(M5.BtnSelect.wasPressed()) { try_Connect_to_Tello(); } //Connect
      else if(M5.BtnB.wasPressed()) { try_Connect_to_Tello(); } //Connect
      else if(M5.BtnStart.wasPressed()) { ScanWifiList(); } //Scan
      else if(M5.BtnA.wasPressed()) { ScanWifiList(); } //Scan

    #elif defined(ARDUINO_M5StickC_ESP32)

      switch(selWifiMode) {
        case 0://
          if(M5.BtnB.wasPressed())      { drawWiFiList(1); } //Next
          else if(M5.BtnA.wasPressed()) { selWifiMode=1; getPwd(TFT_WHITE); } //Pass word
          else if(M5.Axp.GetBtnPress()==2) { ScanWifiList(); } //Rescan
          break;
        case 1://connect
          if(M5.BtnB.wasPressed())      { selWifiMode=0; drawWiFiList(0); } //Cancel
          else if(M5.BtnA.wasPressed()) { selWifiMode=0; drawWiFiList(0); try_Connect_to_Tello(); } //Selet
          else if(M5.Axp.GetBtnPress()==2) { selWifiMode=2; getPwd(TFT_RED); } //Edit mode
          break;
        case 2://edit password
          if(M5.BtnB.wasPressed())      { selWifiMode=0; drawWiFiList(0); } //Cancel
          else if(M5.BtnA.wasPressed()) { ; } //Selet
          else if(M5.Axp.GetBtnPress()==2) { setPwd(TFT_WHITE); selWifiMode=1; } //Finish edit mode
          break;
      }

      if(selWifiMode==2) {
        String sps="";
        while(Serial.available()) {
          char v=(char)Serial.read();
          if(v==0xA || v==0xD) {
            selpwd=sps;
            drawWiFiPwd(TFT_RED);
            break;
          } else if(v>=0x20&&v<0x7F) {
            sps=sps+String(v);
          }
        }
      }
      
      #if defined(useM5CJoyS) || defined(useHATStick)
        #if defined(useHATStick)
        Wire.beginTransmission(JOY_ADDR);
        Wire.write(0x02); 
        Wire.endTransmission();
        #endif
        Wire.requestFrom(JOY_ADDR, 3);
        if (Wire.available()) {
          x_data = Wire.read(); //10-250
          y_data = Wire.read(); //10-250
          // Zボタン(1:押している時 0:離している時 )
          button_data=(Wire.read()==1)?1:0;
          /*
          Serial.print(x_data);
          Serial.print(" / ");
          Serial.print(y_data);
          Serial.print(" / ");
          Serial.println(button_data,DEC);
          */
        }
      #elif defined(useHATJoyC)  //HAT用JoyC
        r_data=joyc.GetX(0)*stickRetio;
        u_data=joyc.GetY(0)*stickRetio;
        x_data=joyc.GetX(1)*stickRetio;
        y_data=joyc.GetY(1)*stickRetio;
        button_left=joyc.GetPress(0);
        button_data=joyc.GetPress(1);
        /*
        Serial.print(x_data);
        Serial.print(" / ");
        Serial.print(y_data);
        Serial.print(" / ");
        Serial.print(button_data,DEC);
        Serial.print(" / ");
        Serial.print(r_data);
        Serial.print(" / ");
        Serial.print(u_data);
        Serial.print(" / ");
        Serial.println(button_left,DEC);
        */
      
      #endif
      
      switch(selWifiMode) {
      case 0://
        if(button_data)      { drawWiFiList(1); delay(250); } //Next
        else if(button_left) { drawWiFiList(-1); delay(250); } //Previous
        break;
      case 1://connect
      case 2://edit password
        if(button_data)      { selWifiMode=0; drawWiFiList(0); delay(250); } //Next
        else if(button_left) { selWifiMode=0; drawWiFiList(0); delay(250); } //Previous
        break;
      }

      x_data = (uint16_t)(float)x_data*jsr;//(64.0/15.0)-(128.0/3.0);
      y_data = (uint16_t)(float)y_data*jsr;//(64.0/15.0)-(128.0/3.0);
      xxx = float(x_data-512) / 512.0;
      yyy = float(y_data-512) / 512.0;
      
      #ifdef useHATJoyC
        r_data = (uint16_t)(float)r_data*jsr;//(64.0/15.0)-(128.0/3.0);
        u_data = (uint16_t)(float)u_data*jsr;//(64.0/15.0)-(128.0/3.0);
        rrr = float(r_data-512) / 512.0;
        uuu = float(u_data-512) / 512.0;
      #endif

      #ifdef useHATJoyC
        M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
        sprintf(msgx,"LR: %-2.2f  ",xxx);
        M5.Lcd.drawString(msgx,svxx-40,svy,1);//drawCentreString
        sprintf(msgy,"FB: %-2.2f  ",yyy);
        M5.Lcd.drawString(msgy,svyx-40,svy,1);
        sprintf(msgx,"RO: %-2.2f  ",rrr);
        M5.Lcd.drawString(msgx,svxx-40,svy+10,1);//drawCentreString
        sprintf(msgy,"UD: %-2.2f  ",uuu);
        M5.Lcd.drawString(msgy,svyx-40,svy+10,1);
      #else
        M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
        sprintf(msgx," %-2.2f  ",xxx);
        M5.Lcd.drawCentreString(msgx,svxx,svy,2);//drawCentreString
        sprintf(msgy," %-2.2f  ",yyy);
        M5.Lcd.drawCentreString(msgy,svyx,svy,2);
      #endif
      
      /*
      if(selWifiMode>0) {        
        if(button_data) {
          if(selWifiMode==1) {
            selWifiMode=2;
            getPwd(TFT_RED);
          } else if(selWifiMode==2) {
            selWifiMode=1;
            setPwd(TFT_WHITE);
          }
        }
      }
      */

    #else //M5Stackなど
      if(M5.BtnA.wasPressed())      { drawWiFiList(1); } //Next
      else if(M5.BtnB.wasPressed()) { try_Connect_to_Tello(); } //Selet
      else if(M5.BtnC.wasPressed()) { ScanWifiList(); } //Rescan
      
    #endif
    
    takeOff=false;
    
  }

  DeepSleepPowerOff();//スリープ？
  delay(50);  // 500ミリ秒のウェイトを50ミリ秒に減らした
  M5.update();
  
}

//*******************************
// Faces JoyStick LED
//*******************************

#ifdef useFacesJoyStick
void LedFaceJoyStick() {
  if (x_data > 600){
    Led(2,  0, (button_data)?0:50, (button_data)?50:0);
    Led(0, 0, 0, 0);
  } else  if (x_data < 400) {
    Led(0,  0, (button_data)?0:50, (button_data)?50:0);
    Led(2, 0, 0, 0);
  } else {
    Led(0,  0, 0,0); Led(2, 0, 0, 0);
  }
  if (y_data > 600) {
    Led(3,  0, (button_data)?0:50, (button_data)?50:0);
    Led(1, 0, 0, 0);
  } else if (y_data < 400) {
    Led(1,  0, (button_data)?0:50, (button_data)?50:0);
    Led(3, 0, 0, 0);
  } else{
    Led(1,  0, 0, 0); Led(3, 0, 0, 0);
  }
}
#endif //useFacesJoyStick

//*******************************
// FACES Keyboard
//*******************************

#ifdef useFacesKey
void doFacesKey(int key_val) {
  //Serial.println(key_val);
  if (key_val != 0) {
    if (key_val >= 0x30 && key_val <= 0x39) {
      return;
    } else {
      doTenKey(key_val);
      return;
    }
  }
}

void doTenKey(int key_val) {

  switch (key_val) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '=': break;
    case 127://Start
      //離陸
      tello_command_exec("takeoff");
      takeOff=true;
      break;
      
    case 191://Select
      //着陸
      noOpr=millis();
      tello_command_exec("land");
      takeOff=false;
      break;
    
    case 239://A
      //Accel
      plusSpeed=200;
      break;
      
    case 223://B
      //cw, ccw, up, down
      button_data=0;
      break;
      
    case 253://Down -> back
      y_data=812;
      break;

    case 254://Up -> foreward
      y_data=212;
      break;

    case 247://Right
      x_data=812;
      break;

    case 251://Left
      x_data=212;
      break;
      
    //default: return;
  }
}

#endif //useFacesKey

////////////////////////////
//      ユーザ関数定義       //
////////////////////////////

// 画面メッセージエリアへ状況メッセージ表示
int prevmsglen=0;
String brankstr="                                ";

void print_msg(String status_msg) {
  print_msg(status_msg, TFT_WHITE);
}

void print_msg(String status_msg, int c) {
  M5.Lcd.setTextColor(c,TFT_BLACK);
  int msglen=status_msg.length();
  if(prevmsglen>msglen) {
    status_msg=String(status_msg+brankstr).substring(0,prevmsglen);
  }

  #if defined(ARDUINO_M5StickC_ESP32)
    brankstr=brankstr.substring(0,25);
    status_msg=status_msg.substring(0,25);
    M5.Lcd.drawString(brankstr,6,62,1);
    M5.Lcd.drawString(status_msg,6,62,1);
  #else
    M5.Lcd.drawString(brankstr,6,190,2);
    M5.Lcd.drawString(status_msg,6,190,2);
  #endif
  prevmsglen=msglen+3;
}

// Telloへメッセージ送信＆コマンド実行
void tello_command_exec(char* tello_command) {
  Udp.beginPacket(TELLO_IP, PORT);
  Udp.printf(tello_command);
  Udp.endPacket();

  Serial.println(tello_command);
  print_msg(tello_command);
  //message = listenMessage();  // UDP受信の関数を走らせない

  delay(4);//delay(10);
}

// Telloからのメッセージ受信
String listenMessage() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    IPAddress remoteIp = Udp.remoteIP();
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
  }
  delay(10);
  return (char*) packetBuffer;
}
