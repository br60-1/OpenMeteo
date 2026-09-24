// Use the OpenMeteo library 

// The weather icons and fonts are in the sketch data folder, press Ctrl+K
// to view.

// The ESP32 board support package 2.0.0 or later must be loaded in the
// Arduino boards manager to provide LittleFS support.

//            >>>       IMPORTANT TO PREVENT CRASHES      <<<
//>>>>>>  Set LittleFS to at least 2 MB before uploading files  <<<<<<

//            >>>           DON'T FORGET THIS             <<<
// Upload the fonts and icons to LittleFS using the "Tools" menu option.
// Beachte: Mit geöffneter Arduino IDE kann man mit der Tastenkombination [Strg] + [Shift] + [P] das Command-Menü aufrufen.
// Dort sollte ein Eintrag "Upload LittleFS to Pico/ESP8266/ESP32" zu finden sein. Die hochzuladenden Daten müssen in [Sketchordner]\data vorhanden sein.
// Das plugin muss aktuell sein.

//////////////////////////////
// Settings defined below

#define WIFI_SSID      ""      //hier SSID eingeben
#define WIFI_PASSWORD  ""      //hier Pwd eingeben

#define TIMEZONE euCET // See NTP_Time.h tab for other "Zone references", UK, usMT etc

// Update every 15 minutes, up to 10000 request per day are free
// Since weather models are updated only at intervals of 15 to 60 minutes, the example retrieves the forecast every 15 minutes.
const int UPDATE_INTERVAL_SECS = 15UL * 60UL;   

// Pins for the TFT interface are defined in the User_Config.h file inside the TFT_eSPI library

// Hier Standort eingeben
// Set the forecast longitude and latitude to at least 4 decimal places 
const String latitude =  "0.0000"; // 90.0000 to -90.0000 negative for Southern hemisphere
const String longitude = "0.0000"; // 180.000 to -180.000 negative for West

// Short day of week abbreviations used in 4 day forecast (change to your language)
const String shortDOW [8] = {"??", "So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
const String shortMon [13] = {"???", "Jan", "Feb", "Mrz", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"};

// Change the labels to your language here:
const char sunStr[]        = "Sonne";
const char cloudStr[]      = "Wolken";
const char humidityStr[]   = "Feuchte";
const String moonPhase [8] = {"Neumond", "Zunehmend", "1.Viertel", "Zunehmend", "Vollmond", "Abnehmend", "3.Viertel", "Abnehmend"};

// End of user settings
//////////////////////////////
