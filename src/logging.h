#ifndef logging_h
	#define logging_h
    
    #define COLOR_NONE             0
	#define COLOR_RED              1
	#define COLOR_GREEN            2
	#define COLOR_YELLOW           3
	#define COLOR_PURPLE           4

    #define REPORT_STATUS_AWAKE 0
    #define REPORT_STATUS_FAST_SLEEP 1
    #define REPORT_STATUS_INSTANT_SLEEP 2

    #include <Arduino.h>
    void header(bool gps, bool gprs, uint32_t time);
    void log(const __FlashStringHelper* text, uint8_t color);
    void logln(const __FlashStringHelper* text, uint8_t color);
    void logln(char * text, uint8_t color);
    void log(char * text, uint8_t color);
    void addColor(uint8_t color);
    void remColor(uint8_t color);


    void log_update_color(bool gps, uint8_t sats, bool gprs, int gprs_rssi, uint32_t time, uint8_t report_mode, int battery);
    void log(const __FlashStringHelper* text);
    void logln(const __FlashStringHelper* text);
    void log(char* text);
    void logln(char* text);

    void pln(char * text, uint8_t color);
    void p(char * text, uint8_t color);

    #define STATUS_UPDATE 1000
    
#endif