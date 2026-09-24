// The structures below are the repository for the data values extracted from the
// JSON message. The structures are populated with the extracted data by the "value()"
// member function in the main OpenWeather.cpp file.

/*
Einige Strukturen enthalten Arrays. Für den sparsamen Speicherverbrauch sorgt der Fakt, dass nur die benötigte Teilmenge des vollständigen Wetterberichts abgerufen wird.
Das (derzeitige) Limit von 10.000 kostenlosen Anfragen sollte somit keine Probleme bereiten.
Sollten weitere oder andere Wetterdaten benötigt werden, muss die Struktur hier angepasst werden.

Some structures contain arrays. Efficient memory usage is ensured by the fact that only the required subset of the full weather report is retrieved.
The (current) limit of 10,000 free requests should not pose any problems.
If additional or different weather data are required, the structure must be adjusted here.
*/

// The content is zero or "" when first created.

/***************************************************************************************
** Description:   Structure for weather
***************************************************************************************/
typedef struct SOM_weather {

  //current
  uint32_t dt = 0;
  uint8_t  is_day = 0;
  float    temperature_2m = 0;
  float    pressure_msl = 0;
  uint8_t  relative_humidity_2m = 0;
  uint8_t  cloud_cover = 0;
  float    wind_speed_10m = 0;
  uint16_t wind_direction_10m = 0;
  float    rain = 0;
  float    snowfall = 0;
  uint16_t weather_code = 0;

  //current_units
  String utime = "";
  String uinterval = "";
  String utemperature_2m = "";
  String urelative_humidity_2m = "";
  String uis_day = "";
  String urain = "";
  String usnowfall = "";
  String uweather_code = "";
  String ucloud_cover = "";
  String upressure_msl = "";
  String uwind_speed_10m = "";
  String uwind_direction_10m = "";

  // hourly
  uint32_t h_dt[MAX_HOURS] = { 0 };
  float    h_temperature_2m[MAX_HOURS] = { 0 };
  float    h_precipitation_probability[MAX_HOURS] = { 0 };
  float    h_precipitation[MAX_HOURS] = { 0 };
  uint16_t h_weather_code[MAX_HOURS] = { 0 };
  uint8_t  h_cloud_cover[MAX_HOURS] = { 0 };

  //hourly_units
  String h_utime = "";
  String h_utemperature_2m = "";
  String h_uprecipitation_probability = "";
  String h_uprecipitation = "";
  String h_uweather_code = "";
  String h_ucloud_cover = "";

  // daily
  uint32_t d_dt[MAX_DAYS] = { 0 };
  uint32_t d_sunset[MAX_DAYS] = { 0 };
  uint32_t d_sunrise[MAX_DAYS] = { 0 };
  float    d_precipitation_probability_max[MAX_DAYS] = { 0 };
  float    d_temperature_2m_max[MAX_DAYS] = { 0 };
  float    d_temperature_2m_min[MAX_DAYS] = { 0 };
  uint16_t d_weather_code[MAX_DAYS] = { 0 };

  // daily_units
  String d_utime = "";
  String d_uweather_code = "";
  String d_utemperature_2m_max = "";
  String d_utemperature_2m_min = "";
  String d_usunrise = "";
  String d_usunset = "";
  String d_uprecipitation_probability_max = "";

} SOM_weather;

