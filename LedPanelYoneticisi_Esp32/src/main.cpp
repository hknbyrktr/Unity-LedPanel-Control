#include <BluetoothSerial.h>
#include "esp_bt.h"
#include "esp_bt_main.h"

#include <DMD32.h>
#include "fonts/Arial_Black_16_ISO_8859_1.h"
#include "fonts/SystemFont5x7.h"
#include "fonts/Arial_Black_16.h"

//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

//Timer setup
//create a hardware timer  of ESP32
hw_timer_t* timer = NULL;

/*------------------------------------------------------------------------------------
  Interrupt handler for Timer1 (TimerOne) driven DMD refresh scanning, this gets
  called at the period set in Timer1.initialize();
--------------------------------------------------------------------------------------*/


void IRAM_ATTR triggerScan()
{
    dmd.scanDisplayBySPI();
}


#define BTLed 26                                                                            // Bluetooh durum ledi.
#define PanelBtn 32                                                                         // Panel'in Ac-Kapat butonu.
#define PanelLed 25                                                                         // Panel'in durum ledi. 

BluetoothSerial SerialBT;


bool BTActive;
bool ledStatus = false;
bool isPanelOpen = false;
bool initialized = false;                                                                   // Animasyonlarin hepsi ayni degiskeni kullaniyor ama sorun olusturmaz. 

short animIndex = 0;

String text1;                                                                               // Bluetooth'dan gelecek olan veriler
String text2;
String hour;
int text1_RepeatCount = 2;
int text1_Speed = 30;
int text2_Duration = 5;
int hourDuration = 5;
bool text2_Anim;

void TurnOnOffPanel();
void BluetoothLedControl();
void PrintToPanel();
void SetTheTime();


// state; true-> 'Arial_Black_16_ISO_8859_1' font'unda olmayan turkce karakterleri eklemek icin , false -> 'SystemFont5x7' font'una turkce karakter eklemek icin.
String convertTurkishChars(String input, bool state) {                                      // Bazi turkce karakterleri ekle.

  String output = "";                                                                       
  for (int i = 0; i < input.length(); i++) {
      uint8_t first = input[i];
      uint8_t second = input[i + 1];

      if (first == 0xC3) { // ç, Ç, ö, Ö, ü, Ü
          switch (second) {
          case 0xA7: output += (char)(state ? 0xE7 : 0x80); i++; break; // ç
          case 0x87: output += (char)(state ? 0xC7 : 0x81); i++; break; // Ç
          case 0xB6: output += (char)(state ? 0xF6 : 0x82); i++; break; // ö
          case 0x96: output += (char)(state ? 0xD6 : 0x83); i++; break; // Ö
          case 0xBC: output += (char)(state ? 0xFC : 0x84); i++; break; // ü
          case 0x9C: output += (char)(state ? 0xDC : 0x85); i++; break; // Ü
          default: output += (char)first; break;
          }
      }
      else if (first == 0xC4) { // ı, İ, ğ, Ğ
          switch (second) {
          case 0xB1: output += (char)(state ? 0x1A : 0x86); i++; break; // ı 
          case 0xB0: output += (char)(state ? 0x1B : 0x87); i++; break; // İ   
          case 0x9F: output += (char)(state ? 0x1C : 0x88); i++; break; // ğ   
          case 0x9E: output += (char)(state ? 0x1D : 0x89); i++; break; // Ğ    
          default: output += (char)first; break;
          }
      }
      else if (first == 0xC5) { // ş, Ş
          switch (second) {
          case 0x9F: output += (char)(state ? 0x1E : 0x8A); i++; break; // ş 
          case 0x9E: output += (char)(state ? 0x1F : 0x8B); i++; break; // Ş 
          default: output += (char)first; break;
          }
      }
      else {
          output += (char)first;
      }
  }
  return output;

}


int splitString(String str, char delimiter, String result[], int maxParts) {                // Gelen veriyi bolmek icin.
  int count = 0;
  int startIndex = 0;
  int delimIndex = 0;

  while ((delimIndex = str.indexOf(delimiter, startIndex)) != -1 && count < maxParts) {
    result[count++] = str.substring(startIndex, delimIndex);
    startIndex = delimIndex + 1;
  }


  if (count < maxParts) {                                                                   // Son parca (delimiter'dan sonra gelen)
    result[count++] = str.substring(startIndex);
  }

  return count;
}



void setup()
{
    Serial.begin(115200);

    pinMode(BTLed, OUTPUT);
    pinMode(PanelBtn, INPUT);
    pinMode(PanelLed, OUTPUT);

    SerialBT.begin("ESP32-LED");                                                            // Ilk acilista bluetooth u ac.
    BTActive = true;
    Serial.println("Bluetooth Acildi");



    // return the clock speed of the CPU
    uint8_t cpuClock = ESP.getCpuFreqMHz();

    // Use 1st timer of 4 
    // devide cpu clock speed on its speed value by MHz to get 1us for each signal  of the timer
    timer = timerBegin(0, cpuClock, true);

    // Attach triggerScan function to our timer 
    timerAttachInterrupt(timer, &triggerScan, true);

    // Set alarm to call triggerScan function  
    // Repeat the alarm (third parameter) 
    timerAlarmWrite(timer, 300, true);

    // Start an alarm 
    timerAlarmEnable(timer);

    //clear/init the DMD pixels held in RAM
    dmd.clearScreen(true);   //true is normal (all pixels off), false is negative (all pixels on)

}


void loop()
{

    if (isPanelOpen) {
        digitalWrite(PanelLed, HIGH);  // LED yanar
    }
    else {
        digitalWrite(PanelLed, LOW);   // LED söner
    }

    BluetoothLedControl();
    TurnOnOffPanel();                                                                       // Led paneli duruma gore ac-kapat.

    if (BTActive && SerialBT.available())
    {
        String data = SerialBT.readStringUntil('\n');
        data.trim();                                                                        // Bosluklari kaldir.

        Serial.println("Gelen veri: |" + data + "|");

        String parts[8];  // En fazla 8 parçaya böl
        splitString(data, '@', parts, 8);

        text1 = parts[0];                                                                   // Kayan metin
        text2 = parts[1];                                                                   // Sabit yazan metin (yazildiysa)
        hour = parts[2];                                                                    // hour (secildiyse)
        text1_Speed = parts[3].toInt();                                                     // Kayan metnin hizi
        text1_RepeatCount = parts[4].toInt();                                               // Kayan metnin tekrar sayisi
        text2_Anim = parts[5] == "True";                                                    // Sabit yazan metinde animasyon olsunmu ?
        text2_Duration = parts[6].toInt();                                                  // Sabit yazan metnin ekranda kalis suresi
        hourDuration = parts[7].toInt();                                                    // hour in ekranda kalis suresi


        if (text1.equalsIgnoreCase("Kapat"))
        {
            isPanelOpen = false;
            dmd.clearScreen(true);
            text1 = "";
            Serial.println("Panel Bluetooth ile Kapandı");
        }
        else if (text1.equalsIgnoreCase("Ac"))
        {
            isPanelOpen = true;
            text1 = "";
            Serial.println("Panel Bluetooth ile Açıldı");
        }
        else if (isPanelOpen)
        {
            Serial.println("Kayan metin: |" + text1 + "|");
            Serial.println("Sabit metin: |" + text2 + "|");
            Serial.println("hour: |" + hour + "|");
            Serial.println("Kayan metnin hizi: |" + String(text1_Speed) + "|");
            Serial.println("Kayan metnin tekrar sayisi: |" + String(text1_RepeatCount) + "|");
            Serial.println("Sabit metnin animasyonu varmi: |" + String(text2_Anim) + "|");
            Serial.println("Sabit metnin ekranda kalma suresi: |" + String(text2_Duration) + "|");
            Serial.println("hour in ekranda kalma suresi: |" + String(hourDuration) + "|");

            text1 = convertTurkishChars(text1, true);

            text2 = convertTurkishChars(text2, false);
            
            dmd.selectFont(Arial_Black_16_ISO_8859_1);                                      // Font'u sec
            dmd.drawMarquee(text1.c_str(), text1.length(), (32 * DISPLAYS_ACROSS) - 1, 0);
            
            delay(100);
            animIndex = 0;
            initialized = false;

        }

    }

    if (isPanelOpen && text1 != "")
        PrintToPanel();
    else
        dmd.clearScreen(true);


}



void PrintToPanel()
{
    static unsigned long scrollTimer = 0;   
    static short text1_Sayac = 0; 


    if (animIndex  == 0)                                                                    // animIndex ilk animasyon bu oldugu icin cokta onemli degi ama koyduk iste.
    {

      if(!initialized)
      {
        dmd.clearScreen(true);
        dmd.selectFont(Arial_Black_16_ISO_8859_1);                                          // Font'u sec
        
        text1_Sayac = 0;
        initialized = true;
      }

      static boolean text1_Bitti = false;
      if (millis() - scrollTimer > text1_Speed) {

        text1_Bitti = dmd.stepMarquee(-1, 0);                                               // Yazi bittimi kontrolu.
        scrollTimer = millis();
      }

      if (text1_Bitti)
      {
        text1_Sayac++;
        text1_Bitti = false;
        if (text1_Sayac == text1_RepeatCount)
        {
          animIndex++;   
          initialized = false;
        }
      }

    } 

    if(animIndex == 1 && text2 != "null")                                                   // text2 varsa text1 le arasina gecis animasyonu koy
    {
      static unsigned long previousMillis = 0;
      static bool state = false;
      static int blinkCount = 0;
      
      if (!initialized) {                                                                   // İlk girişte bir desen hemen çizilsin
                                                    
        dmd.drawTestPattern(PATTERN_ALT_1);                           
        previousMillis = millis();
        initialized = true;
      }

      if (millis() - previousMillis >= 1000) {
        previousMillis = millis();

        if (state) {
          dmd.drawTestPattern(PATTERN_ALT_1);
        } else {
          dmd.drawTestPattern(PATTERN_ALT_0);
        }

        state = !state;
        blinkCount++;

        if (blinkCount >= 3) {
          animIndex++;                                                                      // 2 kere yaptıktan sonra sıradaki ekrana geç
          
          initialized = false;                                                              // diğer kullanım için sıfırla
          blinkCount = 0;
          state = false;
        }

      }
      
    }
    else if (animIndex == 1 && text2 == "null")
      animIndex = 3;                                                                       // text2 yoksa hour gostermeden onceki animasyona gec
 

    if(animIndex == 2 && text2 != "null")
    {
      static unsigned long screenCounter = 0;
      static String part1;
      static String part2;

      if(!initialized)
      {
        screenCounter = millis();
        dmd.selectFont(System5x7);

        int index = text2.indexOf(' ');                      

        if(index != -1){                           // text2 = "falan filan" ise
          part1 = text2.substring(0, index);       // "falan"
          part2 = text2.substring(index + 1);      // "filan"
        }
        else{                                      // text2 = "falan"
          part1 = text2;                           // ""
          part2 = "";
        }                                          // Bu sekilde ayirmazsak part1'de part2'de 'falan' oluyor.


        if(!text2_Anim){                           // Eger sabit metinde animasyon yoksa direk yaz.
          dmd.clearScreen( true );
          dmd.drawBox(  0,  0, (32*DISPLAYS_ACROSS)-1, (16*DISPLAYS_DOWN)-1, GRAPHICS_NORMAL );
        
          for (byte x=0;x<DISPLAYS_ACROSS;x++) {
            for (byte y=0;y<DISPLAYS_DOWN;y++) {
              dmd.drawString(  2+(32*x),  0+(16*y), part1.c_str(), 5, GRAPHICS_NORMAL );
              dmd.drawString(  2+(32*x),  8+(16*y), part2.c_str(), 5, GRAPHICS_NORMAL );
            }
          }
        }

        initialized = true;
      }

      static unsigned long counter = 0; 
      static short upper = -8;
      static short bottom = 16;

      if(text2_Anim){                              // Kullanici sabit metinde animasyonu aktif ettiyse asagidaki animasyonu uygula
        if(millis() - screenCounter >= counter && upper <= 0 && bottom >= 8){

          dmd.clearScreen( true );
          dmd.drawBox(  0,  0, (32*DISPLAYS_ACROSS)-1, (16*DISPLAYS_DOWN)-1, GRAPHICS_NORMAL );
          
          for (byte x=0;x<DISPLAYS_ACROSS;x++) {
            for (byte y=0;y<DISPLAYS_DOWN;y++) {
              dmd.drawString(  2+(32*x),  upper+(16*y), part1.c_str(), 5, GRAPHICS_NORMAL );
              dmd.drawString(  2+(32*x),  bottom+(16*y), part2.c_str(), 5, GRAPHICS_NORMAL );
            }
          }
          
          upper++;
          bottom--;
          counter += 300;
        }
      }

      if( millis() - screenCounter >= (text2_Duration)*1000)                              // Kullanici sn cinsinden girdi (5 gibi) bizde bizde dogru deger olamsi icin 1000 ile carptik (5000 ms)
      {
        animIndex++;                                                                      // 5 kere yaptıktan sonra işlemi bitir
        
        upper = -8;
        bottom = 16;
        counter = 500;

        initialized = false;                                                              // diğer kullanım için sıfırla
        screenCounter = 0;
      }

    }

    if(animIndex == 3 && hour != "null")                                                  // hour degeri varsa ondan bir onceki animasyon
    {
      static unsigned long previousMillis = millis();
      static int b = 0;
      static bool animationFinished = false;
      
      if (!animationFinished && millis() - previousMillis >= 200) {

        previousMillis = millis();
        
        dmd.drawTestPattern((b & 1) + PATTERN_STRIPE_0);
        b++;

        if (b >= 20) {                                                                    // Dikey cizgileri 20 kere kaydir
          animationFinished = true;
          previousMillis = millis();                                                      // 200ms'lik ek bekleme için sıfırla
        }
      }

      if (animationFinished && millis() - previousMillis >= 200) {                        // animasyon bitince 200ms bekleyip işlemi sonlandırmak istersen:

        animIndex++;                                                                      // veya başka bir işleme geç
        b = 0;
        animationFinished = false;
      }

    }
    else if (animIndex == 3 && hour == "null")
      animIndex = 0 ;


    if (animIndex == 4 && hour != "null")
    {
        static long screenTime = 0;
        static unsigned long previousMillis = millis();

        static bool colonVisible = false;

        if (!initialized) 
        {
          screenTime = millis();

          dmd.clearScreen( true );
          dmd.selectFont(Arial_Black_16);

          dmd.drawChar(  0,  3, hour[0], GRAPHICS_NORMAL );
          dmd.drawChar(  7,  3, hour[1], GRAPHICS_NORMAL );
          dmd.drawChar( 15,  3, hour[2], GRAPHICS_OR     );         // clock colon overlay on
          dmd.drawChar( 17,  3, hour[3], GRAPHICS_NORMAL );
          dmd.drawChar( 25,  3, hour[4], GRAPHICS_NORMAL );

          initialized = true;          

        }

        SetTheTime();                                               // Zaman gectikce houri isle

        if (millis() - previousMillis >= 1000) 
        {
            previousMillis = millis();

            if (colonVisible) {
                dmd.drawChar(15, 3, ':', GRAPHICS_OR);              // yak
            }
            else {
                dmd.drawChar(15, 3, ':', GRAPHICS_NOR);             // sondur
            }

            colonVisible = !colonVisible;                           // durum değiştir

            if (millis() - screenTime >=  hourDuration * 1000)      // Kullanici sn cinsinden girdi (5 gibi) bizde bizde dogru deger olamsi icin 1000 ile carptik (5000 ms)
            {
                animIndex = 0;                                      // 5 kere yaptıktan sonra işlemi bitir
                
                initialized = false;                                // diğer kullanım için sıfırla
                screenTime = 0;
                colonVisible = false;
            }

        }

    }


}


void TurnOnOffPanel()
{
    static unsigned long basmaZamani = 0;
    static bool butonBasili = false;
    static bool islemYapildi = false;

    bool butonDurumu = digitalRead(PanelBtn);

    if (butonDurumu == HIGH && !butonBasili) {                                // Butona ilk kez basıldı

        basmaZamani = millis();
        butonBasili = true;
        islemYapildi = false;
    }

    if (butonDurumu == HIGH && butonBasili && !islemYapildi) {                // Buton hâlâ basılı ve işlem yapılmadıysa

        if (millis() - basmaZamani >= 2000) {
            isPanelOpen = !isPanelOpen;

            if (!isPanelOpen)
                dmd.clearScreen(true);

            Serial.println(isPanelOpen ? "Panel Açıldı" : "Panel Kapandı");

            islemYapildi = true;                                              // Bu basışta tekrar işlem yapma
        }
    }

    if (butonDurumu == LOW && butonBasili)
        butonBasili = false;                                                  // Buton birakildi.


}


void BluetoothLedControl()                                                    // Bluetooth durumuna gore yanan led.
{
  
    static unsigned long ledTimer = 0;
    static bool state1 = false;                                               // Ekrana milyonlarca BT aktif... yazmasin diye.
    static bool state2 = false;
    static bool state3 = false;

    if (BTActive)
    {
        state3 = false;

        if (SerialBT.hasClient())                                             // Biri baglaninca.
        {
            digitalWrite(BTLed, HIGH);                                        // Sürekli açık
            if (!state1)
            {
                Serial.println("BT aktif -- Birisi baglandi !!");
                state1 = true;
                state2 = false;
            }

        }
        else
        {
            if (millis() - ledTimer >= 700)
            {
                ledStatus = !ledStatus;
                digitalWrite(BTLed, ledStatus ? HIGH : LOW);
                ledTimer = millis();
            }
            if (!state2)
            {
                Serial.println("BT aktif -- Kimse Bagli degil !!");
                state2 = true;
                state1 = false;
            }

        }

    }
    else
    {
        digitalWrite(BTLed, LOW);                                            // Bluetooth kapalıysa LED kapali (ama kapanmiyor)
        if (!state3)
        {
            Serial.println("BT aktif degil !!");
            state3 = true;
        }
    }

}


void SetTheTime()
{
  unsigned long currentMillis = millis();
  static unsigned long previousMillis = 0;  

    // hour, 09:35 icin
    char c1 = hour[0];  // '0'
    char c2 = hour[1];  // '9'
    
    char c3 = hour[3];  // '3'
    char c4= hour[4];   // '5'

    int currentHour = (c1 - '0') * 10 + (c2 - '0');     // karakterleri rakama çevir
    int currentMinute = (c3 - '0') * 10 + (c4 - '0');   // karakterleri rakama çevir

  if (currentMillis - previousMillis >= 60000) {        // Her dakika bir kere çalışsın
    previousMillis = currentMillis;
    Serial.println("hour:" + hour);
    currentMinute++;
    if (currentMinute >= 60) {
      currentMinute = 0;
      currentHour++;
      if (currentHour >= 24) {
        currentHour = 0;
      }
    }

    if(currentHour >=10){
      hour[0] = String(currentHour)[0];
      hour[1] = String(currentHour)[1];
    }
    else{
      hour[0] = '0';
      hour[1] = String(currentHour)[0];
    }

    if(currentMinute >=10){
      hour[3] = String(currentMinute)[0];
      hour[4] = String(currentMinute)[1];
    }
    else{
      hour[3] = '0';
      hour[4] = String(currentMinute)[0];
    }
    Serial.println("hour:" + String(hour));


  }
} 



