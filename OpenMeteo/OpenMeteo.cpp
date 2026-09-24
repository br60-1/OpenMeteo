/*
Arduino client library for https://open-meteo.com/
Requires the JSON parse library here: https://github.com/Bodmer/JSON_Decoder
and the library for the TFT https://github.com/Bodmer/TFT_eSPI
*/

// Created by Bodmer 09/04/2020 as OpenWeather library: https://github.com/Bodmer/OpenWeather
// Updated by Bodmer 08/01/2021
// Updated by Bodmer 15/02/2023 to support free forecast API
// Change by br60-1 from openweathermap.org to open-meteo.com 01/02/2026 (freie API bis 10.000 Aufrufe pro Tag, kein api-key erforderlich)

// See license.txt in root folder of library
// Insecure mode added by ADAMSIN12

#if defined(ARDUINO_ARCH_MBED) || defined(ARDUINO_ARCH_RP2040)
  #if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    #include <WiFi.h>
  #else
    #include <WiFiNINA.h>
  #endif
#else
  #ifdef ESP8266
    #include <ESP8266WiFi.h>
  #else
    #include <WiFi.h>
  #endif
  #include <WiFiClientSecure.h>
#endif

#include "OpenMeteo.h"

/***************************************************************************************
** Function name:           getForecast
** Description:             Setup the weather forecast request
***************************************************************************************/
// The structures etc are created by the sketch and passed to this function.
// Pass a nullptr to exclude in response.
// ESP8266: Setting secure to false will invoke an insecure connection with AXTLS
//          for the connection, when set true BearSSL will be used.
// ESP32:   Secure parameter has no affect.

bool OM_Weather::getForecast(SOM_weather *weather, String latitude, String longitude, bool secure) {

  Secure = secure;

  // Local copie of structure pointer, the structures are filled during parsing
  this->weather = weather;

  // API now subscription
   
  String url = "https://api.open-meteo.com/v1/forecast?latitude=" + latitude + "&longitude=" + longitude + "&current=temperature_2m,relative_humidity_2m,is_day,rain,snowfall,weather_code,cloud_cover,pressure_msl,wind_speed_10m,wind_direction_10m&hourly=temperature_2m,precipitation_probability,precipitation,weather_code,cloud_cover&daily=weather_code,temperature_2m_max,temperature_2m_min,sunrise,sunset,precipitation_probability_max&timeformat=unixtime&timezone=Europe%2FBerlin&forecast_days=5&forecast_hours=6";
  
  // Es werden nur die Daten bei der API angefordert, die auch benötigt werden. Für Erweiterungen und Anpassungen der Bibliothek siehe die API-Dokumentation bei open-meteo.com. Dort besteht auch die Möglichkeit die URL zu erstellen
  // Bei Änderungen muss auch der Parser gegebenenfalls angepasst werden.
  
  
  /* Beispiel hier für Testzwecke
  https://api.open-meteo.com/v1/forecast?latitude=50.2&longitude=11.74&current=temperature_2m,relative_humidity_2m,is_day,rain,snowfall,weather_code,cloud_cover,pressure_msl,wind_speed_10m,wind_direction_10m&hourly=temperature_2m,precipitation_probability,precipitation,weather_code,cloud_cover&daily=weather_code,temperature_2m_max,temperature_2m_min,sunrise,sunset,precipitation_probability_max&timeformat=unixtime&timezone=Europe%2FBerlin&forecast_days=4&forecast_hours=6
  */

  // Send GET request and feed the parser
  bool result = parseRequest(url);

  // Null out pointers to prevent crashes
  this->weather = nullptr;

  return result;
}

#ifdef ESP32 // Decide if ESP32 or ESP8266 parseRequest available

/***************************************************************************************
** Function name:           parseRequest (for ESP32)
** Description:             Fetches the JSON message and feeds to the parser
***************************************************************************************/
bool OM_Weather::parseRequest(String url) {

  uint32_t dt = millis();

  OM_STATUS_PRINTF("\n\nThe connection to server is secure (https). Certificate not checked.\n");
  WiFiClientSecure client;
  client.setInsecure(); // Certificate not checked

  const char*  host = "api.open-meteo.com";
  port = 443;

  if (!client.connect(host, port))
  {
    OM_STATUS_PRINTF("Connection failed.\n");
    return false;
  }

  JSON_Decoder parser;
  parser.setListener(this);

  uint32_t timeout = millis();
  char c = 0;
  parseOK = false;

#ifdef SHOW_JSON
  int ccount = 0;
#endif
  // Send GET request
  Serial.println();
  OM_STATUS_PRINT("Sending GET request to "); OM_STATUS_PRINT(host); OM_STATUS_PRINT(" port "); OM_STATUS_PRINT(port); OM_STATUS_PRINTF("\n");
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

  // Pull out any header, X-Forecast-API-Calls: reports current daily API call count
  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      OM_STATUS_PRINTF("Header end found\n");
      break;
    }

#ifdef SHOW_HEADER
    Serial.println(line);
#endif

    if ((millis() - timeout) > 5000UL)
    {
      OM_STATUS_PRINTF ("HTTP header timeout\n");
      client.stop();
      return false;
    }
  }

  OM_STATUS_PRINTF("\nParsing JSON\n");

  // Parse the JSON data, available() includes yields
  while (client.available() > 0 || client.connected())
  {
    while(client.available() > 0)
    {
      c = client.read();
      parser.parse(c);
	  
#ifdef SHOW_JSON
      if (c == '{' || c == '[' || c == '}' || c == ']') Serial.println();
      Serial.println(c); if (ccount++ > 100 && c == ',') {ccount = 0; Serial.println();}
#endif
    }

    if ((millis() - timeout) > 8000UL)
    {
      OM_STATUS_PRINTF("Client timeout during JSON parse\n");
      parser.reset();
      client.stop();
      return false;
    }
    yield();
  }

  OM_STATUS_PRINTF("\nDone in "); OM_STATUS_PRINT(millis()-dt); OM_STATUS_PRINTF(" ms\n");
  Serial.println();

  parser.reset();
  client.stop();
  
  // A message has been parsed, but the data-point correctness is unknown
  return parseOK;
}

#else // ESP8266 or Arduino RP2040 Nano Connect version

/***************************************************************************************
** Function name:           parseRequest (for ESP8266)
** Description:             Fetches the JSON message and feeds to the parser
***************************************************************************************/
bool OM_Weather::parseRequest(String url) {
  if (Secure) return parseRequestSecure(&url);
  else return parseRequestInsecure(&url);
}

bool OM_Weather::parseRequestSecure(String* url) {

  uint32_t dt = millis();

  const char*  host = "api.open-meteo.com";

  #if (defined(ARDUINO_ARCH_MBED) || defined(ARDUINO_ARCH_RP2040)) && !defined(ARDUINO_RASPBERRY_PI_PICO_W)
  WiFiSSLClient client;
  #else
  // Must use namespace:: to select BearSSL
  BearSSL::WiFiClientSecure client;
  client.setInsecure(); // Certificate not checked
  #endif
  port = 443;

  if (!client.connect(host, port))
  {
    OM_STATUS_PRINTF("Connection failed.\n");
    return false;
  }
  JSON_Decoder parser;
  parser.setListener(this);

  uint32_t timeout = millis();
  char c = 0;
  parseOK = false;

  #ifdef SHOW_JSON
  int ccount = 0;
  #endif

  #ifdef ESP8266
  OM_STATUS_PRINTF("\nThe connection to server is using BearSSL in insecure mode (certificates not checked).\n");
  #endif

  // Send GET request
  Serial.println();
  OM_STATUS_PRINTF("Sending GET request to api.open-meteo.com...\n");
  Serial.println();
  client.print(String("GET ") + *url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  Serial.println();

  // Pull out any header, X-Forecast-API-Calls: reports current daily API call count
  while (client.available() || client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      OM_STATUS_PRINTF("Header end found\n");
      break;
    }

    OM_STATUS_PRINT(line); OM_STATUS_PRINTF("\n");

    if ((millis() - timeout) > 5000UL)
    {
      OM_STATUS_PRINTF ("HTTP header timeout\n");
      client.stop();
      return false;
    }
  }

  // Parse the JSON data, available() includes yields
  while (client.available() || client.connected())
  {
    while (client.available())
    {
      c = client.read();
	  //Test:
//      if (c != ' ')              //Leerzeichen nicht verarbeiten
        parser.parse(c);
		
  #ifdef SHOW_JSON
      if (c == '{' || c == '[' || c == '}' || c == ']') Serial.println();
      Serial.print(c); if (ccount++ > 100 && c == ',') {ccount = 0; Serial.println();}
  #endif
    }

    if ((millis() - timeout) > 8000UL)
    {
      OM_STATUS_PRINTF ("JSON client timeout\n");
      parser.reset();
      client.stop();
      return false;
    }
  }

  Serial.println();
  OM_STATUS_PRINTF("\nDone in "); OM_STATUS_PRINT(millis()-dt); OM_STATUS_PRINTF(" ms\n");

  parser.reset();

  client.stop();
  
  // A message has been parsed without error but the data-point correctness is unknown
  return parseOK;
}

bool OM_Weather::parseRequestInsecure(String* url) {

  uint32_t dt = millis();

  const char*  host = "api.open-meteo.com";

  // AXTLS used (insecure)
  WiFiClient client;
  port = 80;
 
  if (!client.connect(host, port))
  {
    OM_STATUS_PRINTF("Connection failed.\n");
    return false;
  }
  JSON_Decoder parser;
  parser.setListener(this);

  uint32_t timeout = millis();
  char c = 0;
  parseOK = false;

  #ifdef SHOW_JSON
  int ccount = 0;
  #endif

  OM_STATUS_PRINTF("\nThe connection to server is INSECURE (using AXTLS).\n");

  // Send GET request
  OM_STATUS_PRINTF("Sending GET request to api.open-meteo.com...\n");
  client.print(String("GET ") + *url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

  // Pull out any header, X-Forecast-API-Calls: reports current daily API call count
  while (client.available() || client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      OM_STATUS_PRINTF("Header end found\n");
      break;
    }

    OM_STATUS_PRINT(line); OM_STATUS_PRINTF("\n");

    if ((millis() - timeout) > 5000UL)
    {
      OM_STATUS_PRINTF("HTTP header timeout\n");
      client.stop();
      return false;
    }
  }


  // Parse the JSON data, available() includes yields
  while (client.available() || client.connected())
  {
    while (client.available())
    {
      c = client.read();
      parser.parse(c);
  #ifdef SHOW_JSON
      if (c == '{' || c == '[' || c == '}' || c == ']') Serial.println();
      Serial.print(c);
      if (ccount++ > 100 && c == ',') {ccount = 0; Serial.println();}
  #endif
    }

    if ((millis() - timeout) > 8000UL)
    {
      OM_STATUS_PRINTF("JSON client timeout\n");
      parser.reset();
      client.stop();
      return false;
    }
  }

  OM_STATUS_PRINTF("\nDone in "); OM_STATUS_PRINT(millis()-dt); OM_STATUS_PRINTF(" ms\n");

  parser.reset();
  client.stop();
  // A message has been parsed without error but the data-point correctness is unknown
  return parseOK;
}

 #endif // ESP32 or ESP8266 parseRequest


/***************************************************************************************
** Function name:           key etc
** Description:             These functions are called while parsing the JSON message
***************************************************************************************/
void OM_Weather::key(const char *key) {

  currentKey = key;

#ifdef SHOW_CALLBACK
  Serial.println("\n>>> Key >>> " + (String)key);
#endif
}

void OM_Weather::startDocument() {

  currentParent = currentKey = "";
  objectLevel = 0;
  valuePath = "";
  arrayIndex = 0;
  arrayLevel = 0;
  parseOK = true;

#ifdef SHOW_CALLBACK
  Serial.print("\n>>> Start document >>>");
#endif
}

void OM_Weather::endDocument() {

  currentParent = currentKey = "";
  objectLevel = 0;
  valuePath = "";
  arrayIndex = 0;
  arrayLevel = 0;

#ifdef SHOW_CALLBACK
  Serial.print("\n<<< End document <<<");
#endif
}

void OM_Weather::startObject() {

  if (arrayIndex == 0 && objectLevel == 1) currentParent = currentKey;
  objectLevel++;

#ifdef SHOW_CALLBACK
  Serial.print("\n>>> Start object level:" + (String) objectLevel + " array level:" + (String) arrayLevel + " array index:" + (String) arrayIndex +" >>>");
#endif
}

void OM_Weather::endObject() {

  if (arrayLevel == 0) currentParent = "";
  if (arrayLevel == 1  && objectLevel == 2) arrayIndex++;
  objectLevel--;
  

#ifdef SHOW_CALLBACK
  Serial.print("\n<<< End object <<<");
#endif
}

void OM_Weather::startArray() {

  arrayLevel++;
  valuePath = currentParent + "/" + currentKey; // aka = current Object, e.g. "daily:data"

#ifdef SHOW_CALLBACK
  Serial.print("\n>>> Start array " + valuePath + "/" + (String) arrayLevel + "/" + (String) arrayIndex +" >>>");
#endif
}

void OM_Weather::endArray() {
  if (arrayLevel > 0) arrayLevel--;
  if (arrayLevel == 0) arrayIndex = 0;
  valuePath = "";

#ifdef SHOW_CALLBACK
  Serial.print("\n<<< End array <<<");
#endif
}

void OM_Weather::whitespace(char c) {
  c = c; // Avoid warning
 
}

void OM_Weather::error( const char *message ) {
  Serial.print("\nParse error message: ");
  Serial.print(message);
  parseOK = false;
}


void OM_Weather::value(const char *val) {

   String value = val;

  // Start of JSON
  if (currentParent == "") {
    if (currentKey == "latitude") lat = value.toFloat();
	else
    if (currentKey == "longitude") lon = value.toFloat();
    else
    if (currentKey == "elevation") elev = value.toFloat();
    else
    if (currentKey == "utc_offset_seconds") utc_offset = (uint32_t)value.toInt();
  }

  // Current forecast - no array index, short path
  if (currentParent == "current") {
    
	if (currentKey == "time") weather->dt = (uint32_t)value.toInt();
    else
    if (currentKey == "is_day") weather->is_day = value.toInt();
    else
    if (currentKey == "temperature_2m") weather->temperature_2m = value.toFloat();
    else
    if (currentKey == "pressure_msl") weather->pressure_msl = value.toFloat();
    else
    if (currentKey == "relative_humidity_2m") weather->relative_humidity_2m = value.toInt();
    else
    if (currentKey == "cloud_cover") weather->cloud_cover = value.toInt();
    else
    if (currentKey == "wind_speed_10m") weather->wind_speed_10m = value.toFloat();
    else
    if (currentKey == "wind_direction_10m") weather->wind_direction_10m = value.toInt();
    else
    if (currentKey == "rain") weather->rain = value.toFloat();
    else
    if (currentKey == "snowfall") weather->snowfall = value.toFloat();
    else
    if (currentKey == "weather_code") weather->weather_code = value.toInt();

    return;
  }
  if (currentParent == "current_units") {

    if (currentKey == "time") weather->utime = value;
    else
    if (currentKey == "interval") weather->uinterval = value;
    else
    if (currentKey == "temperature_2m") weather->utemperature_2m = value;
    else
    if (currentKey == "relative_humidity_2m") weather->urelative_humidity_2m = value;
    else
    if (currentKey == "is_day") weather->uis_day = value;
    else
    if (currentKey == "rain") weather->urain = value;
    else
    if (currentKey == "snowfall") weather->usnowfall = value;
    else
    if (currentKey == "weather_code") weather->uweather_code = value;
    else
    if (currentKey == "cloud_cover") weather->ucloud_cover = value;
    else
    if (currentKey == "pressure_msl") weather->upressure_msl = value;
    else
    if (currentKey == "wind_speed_10m") weather->uwind_speed_10m = value;
    else
    if (currentKey == "wind_direction_10m") weather->uwind_direction_10m = value;

    return;
  } 

  // Hourly forecast
  if (currentParent == "hourly") {
    
    if (arrayIndex >= MAX_HOURS) return;
    
    if (currentKey == "time") weather->h_dt[arrayIndex] = (uint32_t)value.toInt();
    else
    if (currentKey == "temperature_2m") weather->h_temperature_2m[arrayIndex] = value.toFloat();
    else
    if (currentKey == "precipitation") weather->h_precipitation[arrayIndex] = value.toFloat();
    else
    if (currentKey == "precipitation_probability") weather->h_precipitation_probability[arrayIndex] = value.toFloat();
    else
    if (currentKey == "weather_code") weather->h_weather_code[arrayIndex] = value.toInt();
    else
    if (currentKey == "cloud_cover") weather->h_cloud_cover[arrayIndex] = value.toInt();
    
	arrayIndex++;
    return;
  }

  if (currentParent == "hourly_units") {

    if (currentKey == "time") weather->h_utime = value;
    else
    if (currentKey == "temperature_2m") weather->h_utemperature_2m = value; 
    else
    if (currentKey == "precipitation_probability") weather->h_uprecipitation_probability = value; 
    else
    if (currentKey == "precipitation") weather->h_uprecipitation = value; 
    else
    if (currentKey == "weather_code") weather->h_uweather_code = value; 
    else
    if (currentKey == "cloud_cover") weather->h_ucloud_cover = value; 

    return;
  }

  // Daily forecast
  if (currentParent == "daily") {
    
    if (arrayIndex >= MAX_DAYS) return;
    
    if (currentKey == "time") weather->d_dt[arrayIndex] = (uint32_t)value.toInt();
    else
    if (currentKey == "temperature_2m_max") weather->d_temperature_2m_max[arrayIndex] = value.toFloat();
    else
    if (currentKey == "temperature_2m_min") weather->d_temperature_2m_min[arrayIndex] = value.toFloat();		
    else
    if (currentKey == "sunrise") weather->d_sunrise[arrayIndex] = (uint32_t)value.toInt();
    else
    if (currentKey == "sunset") weather->d_sunset[arrayIndex] = (uint32_t)value.toInt();
    else
    if (currentKey == "precipitation_probability_max") weather->d_precipitation_probability_max[arrayIndex] = value.toFloat();
    else 
    if (currentKey == "weather_code") weather->d_weather_code[arrayIndex] = value.toInt();

	arrayIndex++;
    return;
  }

  if (currentParent == "daily_units") {

    if (currentKey == "time") weather->d_utime = value;
	else
    if (currentKey == "weather_code") weather->d_uweather_code = value;
	else
    if (currentKey == "temperature_2m_max") weather->d_utemperature_2m_max = value;
	else
    if (currentKey == "temperature_2m_min") weather->d_utemperature_2m_min = value;
	else
    if (currentKey == "sunrise") weather->d_usunrise = value;
	else
    if (currentKey == "sunset") weather->d_usunset = value;
	else
    if (currentKey == "precipitation_probability_max") weather->d_uprecipitation_probability_max = value;

    return;
  }
  valuePath = currentParent + "/" + currentKey;
/*Test
  Serial.print("\nvaluePath     :"); Serial.println(valuePath);
  Serial.print("objectLevel   :"); Serial.println(objectLevel);
  Serial.print("currentParent :"); Serial.println(currentParent);
  Serial.print("currentKey    :"); Serial.println(currentKey);
  Serial.print("arrayLevel    :"); Serial.println(arrayLevel);
  Serial.print("arrayIndex    :"); Serial.println(arrayIndex);
  Serial.print("Value         :"); Serial.println(val);  
*/
return; 
 
}


