/*
 Example for Open-Meteo library, adapted from: https://github.com/Bodmer/OpenWeather
 Adapted by Bodmer to use the TFT_eSPI library:  https://github.com/Bodmer/TFT_eSPI
 Adapted by br60-1 as OpenMeteo to use with the open-meteo-api (api.open-meteo.com - no api-key, 10000 calls per day free)
 All displays have been converted into German.
 Displays current weather plus daily forecasts. The second sreen displays the weather for the next 4 hours.

 The OpenMeteoTFT example works with the ESP32 and ESP8266. It uses LittleFS and displays the weather data on a TFT screen.
 This example uses the TFT_eSPI library by Bodmer: https://github.com/Bodmer/TFT_eSPI 
 The above example will work with free Open-Meteo service (see the Open-Meteo.com website for details).
 This example use anti-aliased fonts and newly created icons from the OpenWeather library: https://github.com/Bodmer/OpenWeather
 This sketch is compatible with ESP32 and ESP32 S2/3 and ESP8266, RP2040 and Rasberry not testet. 

Beispiel für die Open-Meteo-Bibliothek, basierend auf: https://github.com/Bodmer/OpenWeather
Von Bodmer für die Verwendung der TFT_eSPI-Bibliothek angepasst: https://github.com/Bodmer/TFT_eSPI
Von br60-1 als „OpenMeteo“ für die Nutzung der Open-Meteo-API angepasst (api.open-meteo.com – kein API-Schlüssel erforderlich, 10.000 kostenlose Abrufe pro Tag).
Alle Bildschirmanzeigen wurden ins Deutsche übersetzt.
Es werden das aktuelle Wetter sowie Tagesvorhersagen angezeigt. Der zweite Bildschirm zeigt das Wetter für die nächsten 4 Stunden an.
Das OpenMeteoTFT-Beispiel funktioniert mit ESP32 und ESP8266. Es verwendet LittleFS und stellt die Wetterdaten auf einem TFT-Bildschirm dar.
Dieses Beispiel nutzt die TFT_eSPI-Bibliothek von Bodmer: https://github.com/Bodmer/TFT_eSPI
Das oben genannte Beispiel funktioniert mit dem kostenlosen Open-Meteo-Dienst (Details dazu auf der Website Open-Meteo.com).
Dieses Beispiel verwendet Anti-Aliasing-Schriftarten sowie neu erstellte Symbole aus der OpenWeather-Bibliothek: https://github.com/Bodmer/OpenWeather
Dieser Sketch ist kompatibel mit ESP32, ESP32 S2/3 und ESP8266; RP2040 und Raspberry wurden nicht getestet.
 Durch die Verwendung der API von Open-Meteo beschränkt sich der Request auf die benötigten Daten, für einen anderen Anwendungsfall müssen daher der Request und der Parser
 in der Bibliothek OpenMeteo.cpp angepasst werden. Der Sketch wurde getestet mit einem ESP8266 D1 mini und dem 2.8" ILI9341 mit 240x320 Pixel im Hochformat.

                           >>>  IMPORTANT  <<<
         Modify setup in All_Settings.h tab to configure your location etc

                >>>  EVEN MORE IMPORTANT TO PREVENT CRASHES <<<
>>>>>>  For ESP8266 set LittleFS to at least 2Mbytes before uploading files  <<<<<<

Beachte: Mit geöffneter Arduino IDE kann man mit der Tastenkombination [Strg] + [Shift] + [P] das Command-Menü aufrufen.
Dort sollte ein Eintrag "Upload LittleFS to Pico/ESP8266/ESP32" zu finden sein. Die hochzuladenden Daten müssen in [Sketchordner]\data vorhanden sein.
Es ist auf die aktuelle Version des plugins in der Arduino-IDE zu achten.

ESP8266/ESP32/RP2040 pin connections to the TFT are defined in the TFT_eSPI library. (File: User_Setup_Select.h)
Original by Daniel Eichhorn, see license at end of file.
*/

// This sketch uses font files created from the Noto family of fonts as bitmaps
// generated from these fonts may be freely distributed: https://www.google.com/get/noto/

// A processing sketch to create new fonts can be found in the Tools folder of TFT_eSPI
// https://github.com/Bodmer/TFT_eSPI/tree/master/Tools/Create_Smooth_Font/Create_font
// New fonts can be generated to include language specific characters. The Noto family
// of fonts has an extensive character set coverage.

// Json streaming parser (do not use IDE library manager version) to use is here:
// https://github.com/Bodmer/JSON_Decoder

//#define SERIAL_MESSAGES  // aktiv machen für Ausgabe über serielle Schnittstelle
//#define RANDOM_LOCATION  // nur für Test
//#define FORMAT_LittleFS  // formatiert das LittleFS und löscht alle Dateien!

#include <FS.h>
#include <LittleFS.h>

#define AA_FONT_SMALL "fonts/NSBold15"  // 15 point Noto sans serif bold
#define AA_FONT_LARGE "fonts/NSBold36"  // 36 point Noto sans serif bold
/***************************************************************************************
**                          Load the libraries and settings
***************************************************************************************/
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>  // https://github.com/Bodmer/TFT_eSPI

// Additional functions
#include "GfxUi.h"     // Attached to this sketch

// Choose library to load
#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_MBED) || defined(ARDUINO_ARCH_RP2040)
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#else
#include <WiFiNINA.h>
#endif
#else  // ESP32
#include <WiFi.h>
#endif

// check All_Settings.h for adapting to your needs
#include "All_Settings.h"
#include <JSON_Decoder.h>  // https://github.com/Bodmer/JSON_Decoder
#include <OpenMeteo.h>     // die neue/geänderte library vormals "OpenWeather"
#include "NTP_Time.h"      // Attached to this sketch, see that tab for library needs

/***************************************************************************************
**                          Define the globals and class instances
***************************************************************************************/

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

OM_Weather om;              // Weather forecast instance

SOM_weather* weather;

GfxUi ui = GfxUi(&tft);     // Jpeg and bmpDraw functions

long lastDownloadUpdate = millis();
const int ain = A0;           //Spannung durch Berührung-> A0 ;-)
int inputVal  =  0;           //input A0 für die Umschaltung des Bildschirms (firstscreen/secondscreen) zur Anzeige der aktuelle und tageweise Vorschau oder der stündlichen Vorschau
boolean booted = true;        //Bootvorgang
boolean dayligth = true;      //Tag/Nacht? ->passende Wettersymbole auswählen
boolean firstscreen = true;   //Selektor, welcher Bildschirm angezeigt wird
boolean switchscreen = false; //Schaltvorgang liegt vor?
boolean updated = false;      //neue Daten gelesen?

/***************************************************************************************
**                          Declare prototypes
***************************************************************************************/
void updateData();
void drawFirstscreen();
void drawSecondscreen();
void drawProgress(uint8_t percentage, String text);
void drawTime();
void drawCurrentWeather();
void drawForecast();
void drawHourlyWeather();
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex);
const char* getMeteoconIcon(uint16_t id, bool today, bool is_day);
void drawAstronomy();
void drawSeparator(uint16_t y);
String strDate(time_t unixTime);
String strTime(time_t unixTime);
String strTimeLocal();
String getWeatherText(uint16_t id);
void printWeather(void);
int leftOffset(String text, String sub);
int rightOffset(String text, String sub);
int splitIndex(String text);
int getNextDayIndex(void);

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  // Stop further decoding as image is running off bottom of screen
  if (y >= tft.height()) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);
  // Return 1 to decode next block
  return 1;
}

/***************************************************************************************
**                          Setup
***************************************************************************************/
void setup() {
  Serial.begin(250000);     //Alle Meldungen für die serielle Schnittstelle sind optional zu Testzwecken
 
  tft.begin();
  tft.setRotation(0);  
  tft.fillScreen(TFT_BLACK);

  if (!LittleFS.begin()) {
    Serial.println("Fehler Initialisierung Dateisystem!");
    while (1) yield();  // Stay here twiddling thumbs waiting
  }
 
  Serial.println("\nDateisystem verfügbar!");

// Enable if you want to erase LittleFS, this takes some time!
// then disable and reload sketch to avoid reformatting on every boot!
#ifdef FORMAT_LittleFS
  tft.setTextDatum(BC_DATUM);  // Bottom Centre
  tft.drawString("Formatiere Dateisystem...", 120, 195);
  LittleFS.format();
#endif

  delay(1000);
  // Draw splash screen
  if (LittleFS.exists("/splash/OpenMeteo.bmp") == true) ui.drawBmp("/splash/OpenMeteo.bmp", 0, 40);
  delay(1500);

  // Clear bottom section of screen
  tft.fillRect(0, 206, 240, 320 - 206, TFT_BLACK);

  tft.loadFont(AA_FONT_SMALL, LittleFS);
  tft.setTextDatum(BC_DATUM);  // Bottom Centre (Textkoordinate unten mittig)
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);

  tft.drawString("Original: blog.squix.org", 120, 250);
  tft.drawString("TFT_eSPI: Bodmer", 120, 270);
  tft.drawString("Icons: Bodmer, br60-1", 120, 290);
  tft.drawString("OpenMeteo: br60-1", 120, 310);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);

  delay(1500);

  tft.fillRect(0, 206, 240, 320 - 206, TFT_BLACK);

  tft.drawString("Verbinde mit WLAN...", 120, 240);
  tft.setTextPadding(240);  // Pad next drawString() text to full width to over-write old text

// Call once for ESP32 and ESP8266
#if !defined(ARDUINO_ARCH_MBED)
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  
#endif

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");

#if defined(ARDUINO_ARCH_MBED) || defined(ARDUINO_ARCH_RP2040)
    if (WiFi.status() != WL_CONNECTED) WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif
    delay(500);
  }
  
  Serial.println("...mit WLAN verbunden.");

  tft.setTextDatum(BC_DATUM);
  tft.setTextPadding(240);        // Pad next drawString() text to full width to over-write old text
  tft.drawString(" ", 120, 220);  // Clear line above using set padding width
  tft.drawString("Rufe Wetterdaten ab...", 120, 240);

  // Fetch the time
  udp.begin(localPort);
  syncTime();

  tft.unloadFont();
}

/***************************************************************************************
**                          Loop
***************************************************************************************/
void loop() {

inputVal=analogRead(ain);             //analog input A0 lesen (Werte von 0 bis 1023)
//Serial.print("Wert A0: ");          //nur Test
//Serial.println(inputVal);           //nur Test
if(inputVal>40 && inputVal<500) {     //Werte ggf. anpassen, haben sich aber bisher bewährt
  firstscreen = !firstscreen;         //negiere die Variable firstscreen bei Berührung
  switchscreen = true;                //Bildschirm soll geändert werden
  delay(300);                         //einen Moment warten wegen "Prelleffekt"
}

// Check, ob frisch gestartet oder Zeit für Wetter-Update ist
if (booted || (millis() - lastDownloadUpdate > 1000UL * UPDATE_INTERVAL_SECS)) {
  if (WiFi.status() == WL_CONNECTED) {
    updateData();                     //ohne WLAN bleiben die bisherigen Daten zur Anzeige und mit ihrer Aktualität erhalten;
    updated = true; 
    lastDownloadUpdate = millis();
  }
}

if (booted || switchscreen || updated) {
  if (firstscreen) drawFirstscreen();
  else drawSecondscreen();
  switchscreen = false;
  updated = false; 
}

// If minute has changed then request new time from NTP server
if (booted || minute() != lastMinute) {
  // Update displayed time first as we may have to wait for a response
  drawTime();
  lastMinute = minute();
  // Request and synchronise the local clock
  syncTime();
}
booted = false;  
}

/***************************************************************************************
**                          Wetterdaten auswerten und anzeigen
***************************************************************************************/
void updateData() {
    if (booted) {
      drawProgress(20, "Aktualisiere Zeit...");
      drawProgress(50, "Aktualisiere Wetterdaten...");
    } 
  
    // bisherige Struktur löschen. Jedoch erst hier, so sind alle Daten in der Struktur für beide Ansichten dauerhaft bis zum nächsten erfolgreichen Update verfügbar
    if (weather) delete weather;
    // Struktur neu erstellen
    weather = new SOM_weather;
 
    bool parsed = om.getForecast(weather, latitude, longitude, true);
    
    if (parsed) Serial.println("Empfange Daten.");
    else Serial.println("Empfang fehlgeschlagen.");
    
    //printWeather();  //for debug, turn on output with #define SERIAL_MESSAGES

    if (booted) {
      drawProgress(100, "Fertig...");
      delay(1000);
      tft.fillScreen(TFT_BLACK);
    } else {
       tft.fillSmoothCircle(15, 32, 3, TFT_BLUE);                                  //ein (Funk)-Empfangssymbol anzeigen ;-)
       delay(300);
       tft.drawArc(15, 32, 7, 6, 230, 310, TFT_BLUE, TFT_BLACK, true);
       delay(300);
       tft.drawArc(15, 32, 12, 11, 230, 310, TFT_BLUE, TFT_BLACK, true);
       delay(300);
       tft.drawArc(15, 32, 17, 16, 230, 310, TFT_BLUE, TFT_BLACK, true);
       delay(300);
       tft.drawArc(15, 32, 22, 21, 230, 310, TFT_BLUE, TFT_BLACK, true);
       delay(500);
       tft.fillRect(10, 15, 28, 30, TFT_BLACK);                                     //und wieder löschen
    }

    if (parsed) {
      time_t local_time = TIMEZONE.toLocal(now(), &tz1_Code);
      String date = "Daten vom " + strDate(local_time);
      tft.loadFont(AA_FONT_SMALL, LittleFS);
      //hier eingefügt, da Font schon geladen und die Aktualisierung nicht in jeder Minute erforderlich ist.
      tft.setTextDatum(TR_DATUM);  //oben, rechts
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      tft.setTextPadding(tft.textWidth("-88 dBm"));  
      String rssi = "---";         //WiFi -Empfangsstärke anzeigen für optimale Platzierung des Gerätes. Achtung, Wert nur beim Update aktuell, also aller 15 Minuten!
      rssi = String(WiFi.RSSI()) + " dBm";
      tft.drawString(rssi, 239, 28); 
      tft.setTextDatum(BC_DATUM);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextPadding(tft.textWidth(" Daten vom 44.Mmm   44:44 "));  // String width + margin
      tft.drawString(date, 120, 16);
      tft.unloadFont();
    } else {
      tft.loadFont(AA_FONT_SMALL, LittleFS);
      tft.setTextDatum(BC_DATUM);
      Serial.println("Fehler Wetterempfang.");
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.setTextPadding(tft.textWidth(" Daten vom 44.Mmm   44:44 "));  // String width + margin
      tft.drawString("Fehler Wetterempfang.", 120, 16);
      tft.unloadFont();
      delay(60000);   // 1 Minute warten, dann Neustart, zumindest beim ESP
      ESP.restart();
    }
}

void drawFirstscreen() {
  tft.fillRect(0, 45, 240, 320, TFT_BLACK);  //Bildschirm tlw. löschen, oben Datum und Zeit stehen lassen
  tft.loadFont(AA_FONT_SMALL, LittleFS);
  drawCurrentWeather();  //aktuelle Daten
  drawForecast();        //4 Tage Vorhersage
  drawAstronomy();       //Mond, Sonne usw. 
  tft.unloadFont();
  // Aktualisieren der Temperatur bereits hier, so muss die Schrift nicht wiederholt in den Speicher geladen und entladen werden
  tft.loadFont(AA_FONT_LARGE, LittleFS);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  // Font ASCII code 0xB0 is a degree symbol, but o used instead in small font
  tft.setTextPadding(tft.textWidth(" -88"));  // Max width of values
  String weatherText = "";
  weatherText = String(weather->temperature_2m, 0);      // Make it integer temperature
  tft.drawString(weatherText, 214, 95);                  //  + "°" symbol is big... use o in small font
  tft.unloadFont();
}

void drawSecondscreen() {
  tft.fillRect(0, 45, 240, 320, TFT_BLACK);  //Bildschirm unten tlw. löschen
  drawHourlyWeather();  // Vorhersage für folgende 4 Stunden, angefordert werden aber 5, "Stunde 0" braucht es für die Sonnendaten (Auf- und Untergang)
}

/***************************************************************************************
**                          Update progress bar
***************************************************************************************/
void drawProgress(uint8_t percentage, String text) {
  tft.loadFont(AA_FONT_SMALL, LittleFS);
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextPadding(240);
  tft.drawString(text, 120, 260);
  ui.drawProgressBar(10, 269, 240 - 20, 15, percentage, TFT_WHITE, TFT_BLUE);
  tft.setTextPadding(0);
  tft.unloadFont();
}

/***************************************************************************************
**                          Draw the clock digits
***************************************************************************************/
void drawTime() {
  tft.loadFont(AA_FONT_LARGE, LittleFS);
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextPadding(tft.textWidth(" 44:44 "));  // String width + margin
  tft.drawString(strTimeLocal(), 120, 53);
  tft.setTextPadding(0);
  tft.unloadFont();
}

/***************************************************************************************
**                          Draw the current weather
***************************************************************************************/
void drawCurrentWeather() {
  String weatherText = "None";
  String weatherIcon = "";

  weatherIcon = getMeteoconIcon(weather->weather_code, true, (weather->is_day == 1));
  ui.drawBmp("/icon/" + weatherIcon + ".bmp", 0, 53);
    
  weatherText = getWeatherText(weather->weather_code);

  tft.setTextDatum(BR_DATUM);                 //bottom right
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);

  int splitPoint = 0;
  int xpos = 235;
  splitPoint = splitIndex(weatherText);       //Zeilenumbruch

  tft.setTextPadding(xpos - 100);             //xpos - icon width
  if (splitPoint) tft.drawString(weatherText.substring(0, splitPoint), xpos, 69);
  else tft.drawString(" ", xpos, 69);
  tft.drawString(weatherText.substring(splitPoint), xpos, 86);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextDatum(TR_DATUM);
  tft.setTextPadding(0);
  tft.drawString("oC", 237, 95);
  //die großen Zahlen für den Temperaturwert werden in "updateData()" geschrieben, so wird der Font-Wechsel vermieden
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  weatherText = String(weather->wind_speed_10m, 0);
  weatherText += " km/h";
  
  tft.setTextDatum(TC_DATUM);                     //top center  
  tft.setTextPadding(tft.textWidth("888 km/h"));  //max string length
  tft.drawString(weatherText, 124, 136);

  weatherText = String(weather->pressure_msl, 0);
  weatherText += " hPa";

  tft.setTextDatum(TR_DATUM);
  tft.setTextPadding(tft.textWidth(" 888 hPa"));  //max string length
  tft.drawString(weatherText, 230, 136);
  int windAngle = (weather->wind_direction_10m + 22.5) / 45;
  if (windAngle > 7) windAngle = 0;
  String wind[] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
  ui.drawBmp("/wind/" + wind[windAngle] + ".bmp", 101, 86);
  drawSeparator(153);
  tft.setTextDatum(TL_DATUM);  //reset datum to normal oben, links
  tft.setTextPadding(0);       //reset padding width to none
}

/***************************************************************************************
**                          Draw the hourly weather (2.screen)
***************************************************************************************/

void drawHourlyWeather() {
  tft.loadFont(AA_FONT_SMALL, LittleFS);
  int bH = 66;                              //Zeilen-/Blockhöhe für 1 Stunde
  int ystart = 54;                          //y-Startwert 
  int ypos = 0;                             //Blockstart
  String weatherText = "";
  for (int i = 2; i <= (MAX_HOURS); i++) {  //für die nächsten 2 bis 5 Stunden (i - 1 wegen Gruppierung in Zeilen)
    ypos = ystart + ((i - 2) * bH);
    drawSeparator(ypos);                    //Trennlinie zeichnen
    tft.setTextDatum(TL_DATUM);             //oben links
    tft.setTextPadding(tft.textWidth(" 88:00 "));
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString(strTime(weather->h_dt[i-1]), 2, ypos+2);  // die Stunde
    tft.setTextDatum(TR_DATUM); 
    tft.setTextPadding(0);
    tft.drawString("oC", 237, ypos + 8);    //oC in kleiner Schrift
    tft.setTextDatum(TL_DATUM);             //oben links
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setTextPadding(tft.textWidth(" Wolken: 100%  "));
    weatherText = "Wolken: ";
    weatherText += weather->h_cloud_cover[i-1];
    weatherText += "%";
    tft.drawString(weatherText, 52, ypos + 5);
    tft.setTextPadding(tft.textWidth(" Regen: 100% "));
    if ((weather->h_weather_code[i-1] == 71) || (weather->h_weather_code[i-1] == 73) || (weather->h_weather_code[i-1] == 75)) weatherText = "Schnee: ";
    else weatherText = "Regen: ";                                                    //hier Text Regen oder Schnee auswählen
    weatherText += String(weather->h_precipitation_probability[i-1], 0) + "%";       //Niederschlagswahrscheinlichkeit unspezifisch (Regen oder Schnee/Hagel)
    tft.drawString(weatherText, 52, ypos + 22);
    tft.setTextPadding(tft.textWidth(" Menge: 100mm "));
    weatherText = "Menge: ";
    weatherText += String(weather->h_precipitation[i-1], 0) + "mm";                  //Niederschlagsmenge kmpl.
    tft.drawString(weatherText, 52, ypos + 40);
    dayligth = ((weather->h_dt[i-1] > weather->d_sunrise[0]) && (weather->h_dt[i-1] < weather->d_sunset[0]));   //Tageslicht, wenn Sonnenaufgang > Vorhersagezeit < Sonnenuntergang -> richtige Symbole (Tag/Nacht)
    String weatherIcon = getMeteoconIcon(weather->h_weather_code[i-1], true, dayligth);
    ui.drawBmp("/icon50/" + weatherIcon + ".bmp", 0, ypos + 15);                     //kleine Wettersymbole laden
  }
  tft.unloadFont();
  tft.loadFont(AA_FONT_LARGE, LittleFS);         //Temperatur in groß 
  tft.setTextDatum(TR_DATUM);                    //oben rechts
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextPadding(tft.textWidth("-88"));      //Max width of values
  for (int i = 2; i <= (MAX_HOURS); i++) {       //für die nächsten 2 bis 5 Stunden
    ypos = ystart + ((i - 2) * bH);
    weatherText = String(weather->h_temperature_2m[i-1], 0);   //Make it integer temperature
    tft.drawString(weatherText, 214, ypos + 10);               // + "°" symbol is big... use o in small font
  }
  drawSeparator(318);
  tft.unloadFont();
}

/***************************************************************************************
**                          Draw the 4 daily forecast columns
***************************************************************************************/

void drawForecast() {               //tageweise Vorhersage
  drawForecastDetail(8, 171, 1);    //das ist morgen, 0 = heute
  drawForecastDetail(66, 171, 2);  
  drawForecastDetail(124, 171, 3); 
  drawForecastDetail(182, 171, 4); 
  drawSeparator(171 + 69);
}

/***************************************************************************************
**                          Draw 1 forecast column at x, y
***************************************************************************************/
// helper for the forecast columns
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex) {
  if (dayIndex > MAX_DAYS) return;
  String dayow = shortDOW[weekday(TIMEZONE.toLocal(weather->d_dt[dayIndex], &tz1_Code))];  //Bezeichnung nächste 4 Wochentage ermitteln
  dayow.toUpperCase();
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth("WWW"));
  tft.drawString(dayow, x + 25, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth("-88   -88"));
  String highTemp = String(weather->d_temperature_2m_max[dayIndex], 0);
  String lowTemp = String(weather->d_temperature_2m_min[dayIndex], 0);
  tft.drawString(lowTemp + " " + highTemp, x + 25, y + 17);                                //ich habe die Temperaturanzeige lieber von niedrig zu hoch/links nach rechts
  String weatherIcon = getMeteoconIcon(weather->d_weather_code[dayIndex], false, true);
  ui.drawBmp("/icon50/" + weatherIcon + ".bmp", x, y + 18);
  tft.setTextPadding(0);  // Reset padding width to none
}

/***************************************************************************************
**                          Draw Sun rise/set, Moon, cloud cover and humidity
***************************************************************************************/
void drawAstronomy() {

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth(" Abnehmend "));

  time_t local_time = TIMEZONE.toLocal(weather->dt, &tz1_Code);
  uint16_t y = year(local_time);
  uint8_t m = month(local_time);
  uint8_t d = day(local_time);
  uint8_t h = hour(local_time);
  int ip;
  uint8_t icon = moon_phase(y, m, d, h, &ip);

  tft.drawString(moonPhase[ip], 120, 319);
  ui.drawBmp("/moon/moonphase_L" + String(icon) + ".bmp", 120 - 30, 318 - 16 - 60);

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextPadding(0);  // Reset padding width to none
  tft.drawString(sunStr, 40, 270);

  tft.setTextDatum(BR_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth(" 88:88 "));

  String rising = strTime(weather->d_sunrise[0]) + " ";     //Daten gibt es nur im "daily"-Satz, also aus day[0] -> heute
  int dt = rightOffset(rising, ":");                        //Draw relative to colon to them aligned
  tft.drawString(rising, 40 + dt, 290);

  String setting = strTime(weather->d_sunset[0]) + " ";     //Daten gibt es nur im "daily"-Satz
  dt = rightOffset(setting, ":");
  tft.drawString(setting, 40 + dt, 305);

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.drawString(cloudStr, 195, 260);                       // hier die Texte "Wolken", "Sonne" und "Feuchte" schreiben, siehe All_Settings

  String cloudCover = "";
  cloudCover += weather->cloud_cover;
  cloudCover += "%";

  tft.setTextDatum(BR_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth(" 100%"));
  tft.drawString(cloudCover, 210, 277);

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.drawString(humidityStr, 195, 300 - 2); 

  String humidity = "";
  humidity += weather->relative_humidity_2m;
  humidity += "%";

  tft.setTextDatum(BR_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth("100%"));
  tft.drawString(humidity, 210, 315);

  tft.setTextPadding(0);  // Reset padding width to none
}

/***************************************************************************************
**                          Get the icon file name from the index number
***************************************************************************************/
const char* getMeteoconIcon(uint16_t id, bool today, bool is_day) {
  if (today) {
    if (is_day) {
      if (id == 0) return "clear-day";
      if (id == 1) return "partly-cloudy-day";  
    }
    else {
      if (id == 0) return "clear-night";
      if (id == 1) return "partly-cloudy-night"; 
    }
  } 
  if (id == 0) return "clear-day";
  if (id == 1) return "partly-cloudy-day";
  if ((id == 2) || (id == 3)) return "cloudy";
  if ((id == 45) || (id == 48)) return "fog";
  if ((id == 51) || (id == 53) || (id == 55) || (id == 61) || (id == 80)) return "drizzle";
  if (id == 63) return "lightRain";
  if ((id == 65) || (id == 82)) return "rain";  
  if ((id == 66) || (id == 67)) return "sleet";
  if (id == 71) return "snow_l";                //nur eine Flocke ;-) also "snow light"
  if ((id == 73) || (id == 75) || (id == 85) || (id == 86)) return "snow";
  if (id == 77) return "hail";                  // hier für Graupel, richtig Hagel gibt es i.d.R. nur bei Gewitter
  if (id == 81) return "lightRain"; 
  if ((id == 95) || (id == 96)) return "thunderstorm";
  if (id == 99) return "tstorm_hail";           //Gewitter mit Hagel
  return "unknown";
}
/***************************************************************************************
**               Get the weatherText from the index number en->de ohne Umlaute, weil nicht im Font
***************************************************************************************/
String getWeatherText(uint16_t id) {
  if (id == 0) return "Klarer Himmel";
  if (id == 1) return "Wenige Wolken";    
  if (id == 2) return "Teilweise wolkig";           
  if (id == 3) return "Bedeckter Himmel";           
  if (id == 45) return "Nebel";
  if (id == 48) return "Rauhreif und Nebel";
  if (id == 51) return "Leichter Nieselregen";
  if (id == 53) return "Nieselregen";
  if (id == 55) return "Starker Nieselregen";
  if (id == 61) return "Leichter Regen";
  if (id == 63) return "Regen";
  if (id == 65) return "Starker Regen";
  if (id == 66) return "Leichter Eisregen";
  if (id == 67) return "Starker Eisregen";
  if (id == 71) return "Leichter Schneefall";
  if (id == 73) return "Schneefall";
  if (id == 75) return "Starker Schneefall";
  if (id == 77) return "Graupel";
  if (id == 80) return "Leichte Regenschauer";
  if (id == 81) return "Regenschauer";
  if (id == 82) return "Heftige Regenschauer";
  if (id == 85) return "Leichte Schneeschauer";
  if (id == 86) return "Starke Schneeschauer";
  if (id == 95) return "Gewitter";
  if (id == 96) return "Starke Gewitter";
  if (id == 99) return "Extrem Gewitter/Hagel";   
  return "unknown";                                           //case-structure oder array mit 3 Spalten ginge hier auch
}

/***************************************************************************************
**                          Draw screen section separator line
***************************************************************************************/
void drawSeparator(uint16_t y) {
  tft.drawFastHLine(10, y, 240 - 2 * 10, 0x4228);
}

/***************************************************************************************
**                          Determine place to split a long string
***************************************************************************************/
// determine the "space" split point in a long string
int splitIndex(String text) {
  uint16_t index = 0;
  while ((text.indexOf(' ', index) >= 0) && (index <= text.length() / 2)) {
    index = text.indexOf(' ', index) + 1;
  }
  if (index) index--;
  return index;
}

/***************************************************************************************
**                          Right side offset to a character
***************************************************************************************/
// Calculate coord delta from end of text String to start of sub String contained within that text
// Can be used to vertically right align text so for example a colon ":" in the time value is always
// plotted at same point on the screen irrespective of different proportional character widths,
// could also be used to align decimal points for neat formatting
int rightOffset(String text, String sub) {
  int index = text.indexOf(sub);
  return tft.textWidth(text.substring(index));
}

/***************************************************************************************
**                          Left side offset to a character
***************************************************************************************/
// Calculate coord delta from start of text String to start of sub String contained within that text
// Can be used to vertically left align text so for example a colon ":" in the time value is always
// plotted at same point on the screen irrespective of different proportional character widths,
// could also be used to align decimal points for neat formatting
int leftOffset(String text, String sub) {
  int index = text.indexOf(sub);
  return tft.textWidth(text.substring(0, index));
}

/***************************************************************************************
**                          Get daily index at start of next day
***************************************************************************************/
int getNextDayIndex(void) {
  int index = 0;
  int today = day(weather->dt);
  for (index = 0; index < 9; index++) {
    if (day(weather->d_dt[index]) != today) break;
  }
  return index;
}

/***************************************************************************************
**                          Print the weather info to the Serial Monitor
***************************************************************************************/
void printWeather(void) {

  Serial.println("Wetter von Open-Meteo\n");
  if (weather) {
    Serial.println("Aktuelle Wetterwerte:");
    Serial.print("dt (time)        : ");
    Serial.println(strTime(weather->dt));
    Serial.print("dt (date)        : ");
    Serial.println(strDate(weather->dt));
    Serial.print("sunrise             : ");
    Serial.println(strTime(weather->d_sunrise[0]));  //diese Daten gibt es nur dort...
    Serial.print("sunset              : ");
    Serial.println(strTime(weather->d_sunset[0]));
    Serial.print("Latitude            : ");
    Serial.println(om.lat);
    Serial.print("Longitude           : ");
    Serial.println(om.lon);
    Serial.print("Höhe                : ");
    Serial.println(om.elev);
    Serial.print("humidity            : ");
    Serial.println(weather->relative_humidity_2m);
    Serial.print("code                : ");
    Serial.println(weather->weather_code);
    Serial.println();

    Serial.println("############### Stundenweise Vorhersage ###############\n");
    for (int i = 0; i < (MAX_HOURS); i++) {
      Serial.print("Stündliche Vorhersage, Stunde: ");
      if (i < 10) Serial.print(" ");
      Serial.print(i);
      Serial.println();
      Serial.print("dt (time)        : ");
      Serial.println(strTime(weather->h_dt[i]));
      Serial.print("dt (date)        : ");
      Serial.println(strDate(weather->h_dt[i]));
      Serial.print("temp             : ");
      Serial.println(weather->h_temperature_2m[i]);
      Serial.print("clouds           : ");
      Serial.println(weather->h_cloud_cover[i]);
      Serial.print("pop              : ");
      Serial.println(weather->h_precipitation_probability[i]);
      Serial.print("Niederschlag     : ");
      Serial.println(weather->h_precipitation[i]);
      Serial.print("code             : ");
      Serial.println(weather->h_weather_code[i]);
      Serial.println();
    }
    Serial.println("###############  Tageweise Vorhersage ###############\n");
    for (int i = 0; i < (MAX_DAYS); i++) {
      Serial.print("Tägliche Vorhersage, Tag:  ");
      if (i < 10) Serial.print(" ");
      Serial.print(i);
      Serial.println();
      Serial.print("dt (time)        : ");
      Serial.println(strTime(weather->d_dt[i]));
      Serial.print("dt (date)        : ");
      Serial.println(strDate(weather->d_dt[i]));
      Serial.print("sunrise          : ");
      Serial.println(strTime(weather->d_sunrise[i]));
      Serial.print("sunset           : ");
      Serial.println(strTime(weather->d_sunset[i]));
      Serial.print("temp_min         : ");
      Serial.println(weather->d_temperature_2m_min[i]);
      Serial.print("temp_max         : ");
      Serial.println(weather->d_temperature_2m_max[i]);
      Serial.print("pop              : ");
      Serial.println(weather->d_precipitation_probability_max[i]);
      Serial.print("id               : ");
      Serial.println(weather->d_weather_code[i]);
      Serial.println();
    }
  }
}
/***************************************************************************************
**             Convert Unix time to a "local time" time string "12:34"
***************************************************************************************/
String strTime(time_t unixTime) {
  time_t local_time = TIMEZONE.toLocal(unixTime, &tz1_Code);
  String localTime = "";
  if (hour(local_time) < 10) localTime += "0";
  localTime += hour(local_time);
  localTime += ":";
  if (minute(local_time) < 10) localTime += "0";
  localTime += minute(local_time);
  return localTime;
}

/***************************************************************************************
**             Convert UTC-time to a "local time" time string "12:34"
***************************************************************************************/
String strTimeLocal() {
  // Convert UTC (now is UTC?! to local time, returns zone code in tz1_Code, e.g "GMT"
  time_t local_time = TIMEZONE.toLocal(now(), &tz1_Code);
  String timeNow = "";
  if (hour(local_time) < 10) timeNow += "0";
  timeNow += hour(local_time);
  timeNow += ":";
  if (minute(local_time) < 10) timeNow += "0";
  timeNow += minute(local_time);
  return timeNow;
}

/***************************************************************************************
**  Convert Unix time to a local date + time string "Oct 16 17:18"
***************************************************************************************/
String strDate(time_t unixTime) {
  time_t local_time = TIMEZONE.toLocal(unixTime, &tz1_Code);
  String localDate = "";
  localDate += day(local_time); 
  localDate += ". ";
  localDate += shortMon[month(local_time)];             //damit es die Anzeige in deutschen Monats-Abkürzungen gibt (siehe All_Settings.h) 
  localDate += "  " + strTimeLocal();
  return localDate;
}

/**The MIT License (MIT)
  Copyright (c) 2015 by Daniel Eichhorn
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYBR_DATUM HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
  See more at http://blog.squix.ch
*/

//  Changes made by Bodmer:

//  Minor changes to text placement and auto-blanking out old text with background colour padding
//  Moon phase text added (not provided by OpenWeather)
//  Forecast text lines are automatically split onto two lines at a central space (some are long!)
//  Time is printed with colons aligned to tidy display
//  Min and max forecast temperatures spaced out
//  New smart splash startup screen and updated progress messages
//  Display does not need to be blanked between updates
//  Icons nudged about slightly to add wind direction + speed
//  Barometric pressure added

//  Adapted to use the OpenWeather library: https://github.com/Bodmer/OpenWeather
//  Moon phase/rise/set (not provided by OpenWeather) replace with and cloud cover humidity
//  Created and added new 100x100 and 50x50 pixel weather icons, these are in the
//  sketch data folder, press Ctrl+K to view
//  Add moon icons, eliminate all downloads of icons (may lose server!)
//  Adapted to use anti-aliased fonts, tweaked coords
//  Added forecast for 4th day
//  Added cloud cover and humidity in lieu of Moon rise/set
//  Adapted to be compatible with ESP32
//
//  Änderungen von br60-1
//  Umstellung auf Open-Meteo.com, dafür die Bibliothek geändert, bei Open-Meteo (bis zu 10000 Zugriffe pro Tag ohne Registrierung kostenlos), Wegfall eines "Keys"
//  nur Abruf und Auswertung benötigter Daten, das empfangene Objekt (JSON) ist kleiner.
//  Neuer splash-screen, wenn schon alles kosten- und anmeldefrei ist - werben wir auch ein wenig für Open-Meteo.com
//  Einführung einer zusätzlichen Ansicht/Bildschirm für die Daten der nächsten Stunden, Umschaltung der Bildschirme über Signal am analogem Eingang A0 (Touch)
//  Erhaltung der Daten in der Struktur, dadurch Möglichkeit zur Ansicht beider Bildschirme und bei Ausfall/nach Abschalten des WLAN
//  Anzeige so auch, wenn kein WLAN (mehr) verfügbar ist (ich schalte mein WLAN nachts ab!)
//  2 zusätzliche Wetter-Symbole zugefügt, die ich so schön finde (Danke dafür an Bodmer)
//  Ansichten, Datum und Wetteranzeigetexte komplett in deutsche Sprache konvertiert


