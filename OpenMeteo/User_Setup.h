
// Configuration settings for OpenMeteo library

// These parameters set the data point count stored in program memory (not the datapoint
// count sent by the server). So they determine the memory used during collection
// of the data points.

#define MAX_HOURS 5     // Maximum "hourly" forecast period, can be up 1 to 48
#define MAX_DAYS 5      // Maximum "daily" forecast periods can be 1 to 8 (Today + 7 days = 8 maximum)
                        
//#define SHOW_HEADER   // Debug only - for checking response header via serial message
//#define SHOW_JSON     // Debug only - simple serial output formatting of whole JSON message
//#define SHOW_CALLBACK // Debug only to show the decode tree
//#define OM_STATUS_ON  // Debug only - turn on/off progress and status messages


// ###############################################################################
// DO NOT tinker below, this is configuration checking that helps stop crashes:
// ###############################################################################

#ifdef OM_STATUS_ON
  #define OM_STATUS_PRINTF(C) Serial.print(F(C))
  #define OM_STATUS_PRINT(V) Serial.print(V)
#else
  #define OM_STATUS_PRINTF(C)
  #define OM_STATUS_PRINT(X)
#endif

// Check and correct bad setting
#if (MAX_HOURS > 48) || (MAX_HOURS < 1)
  #undef  MAX_HOURS
  #define MAX_HOURS 8
#endif

// Check and correct bad setting
#if (MAX_DAYS > 8) || (MAX_DAYS < 1)
  #undef  MAX_DAYS
  #define MAX_DAYS 6
#endif
