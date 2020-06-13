#include "logging.h"

bool _gps;
bool _gprs;
uint8_t _report_mode = 0;
uint8_t _sats = 0;
int _gprs_rssi = 0;
uint32_t _time=0;
uint32_t status_lastMillis = millis();
uint8_t _battery;
    

void header(){
    char msg[20];
    /////////////////// time ///////////////////
    sprintf(msg,"%10u",_time);
    Serial.print("[");
    addColor(COLOR_PURPLE);
    Serial.print(msg);
    remColor(COLOR_PURPLE);
    p(" | ",COLOR_NONE);
    /////////////////// battery ///////////////////
    if(_battery<30){
        addColor(COLOR_RED);
    } else if(_battery<70){
        addColor(COLOR_YELLOW);
    } else {
        addColor(COLOR_GREEN);
    }
    sprintf(msg,"%3i%%",_battery);
    Serial.print(msg);
    remColor(COLOR_PURPLE);
    p(" | ",COLOR_NONE);
    /////////////////// gps ///////////////////
    if(_gps){
        addColor(COLOR_GREEN);
        Serial.printf("%2i sat  ",_sats);
    } else {
        addColor(COLOR_RED);
        Serial.print("    GPS ");
    }
    remColor(COLOR_RED);
    p("| ",COLOR_NONE);
    /////////////////// GPRS ///////////////////
    if(_gprs){
        if(_gprs_rssi>14){
            addColor(COLOR_GREEN);
        } else if(_gprs_rssi >9){
            addColor(COLOR_YELLOW);
        } else {
               addColor(COLOR_RED);
        }
        Serial.printf("%2i dBm ",_gprs_rssi);
    } else {
        addColor(COLOR_RED);
        Serial.print("  GPRS ");
    }
    remColor(COLOR_RED);
    p("| ",COLOR_NONE);
    /////////////////// MODE ///////////////////
    if(_report_mode == REPORT_STATUS_AWAKE){
        Serial.print("AW");
    } else if(_report_mode == REPORT_STATUS_INSTANT_SLEEP){
        Serial.print("IS");
    } else if(_report_mode == REPORT_STATUS_FAST_SLEEP){
        Serial.print("FS");
    } else {
        Serial.print("UK");
    }
    p("] ",COLOR_NONE);
    
    status_lastMillis = millis() + STATUS_UPDATE;
}

void log_update_color(bool gps, uint8_t sats, bool gprs, int gprs_rssi, uint32_t time, uint8_t report_mode, int battery){
    _battery = battery;
    _gps = gps;
    _gprs = gprs;
    _time = time;
    _report_mode = report_mode;
    _sats = sats;
    _gprs_rssi = gprs_rssi;

    if(status_lastMillis < millis()){
        header();
        Serial.printf("\r\n");
    }
}

void log(const __FlashStringHelper* text){
    log(text, COLOR_NONE);
}   

void logln(const __FlashStringHelper* text){
    logln(text, COLOR_NONE);
}   

void log(char* text){
    log(text, COLOR_NONE);
}   

void logln(char* text){
    logln(text, COLOR_NONE);
}   

void log(const __FlashStringHelper* text, uint8_t color){
    header();
	addColor(color);
	Serial.print(text);
	remColor(color);
}

void logln(const __FlashStringHelper* text, uint8_t color){
    header();
	addColor(color);
	Serial.println(text);
	remColor(color);
}

void logln(char * text, uint8_t color){
    header();
	addColor(color);
	Serial.println(text);
	remColor(color);
};

void log(char * text, uint8_t color){
    header();
	addColor(color);
	Serial.print(text);
	remColor(color);
};

void pln(char * text, uint8_t color){
	addColor(color);
	Serial.println(text);
	remColor(color);
};

void p(char * text, uint8_t color){
	addColor(color);
	Serial.print(text);
	remColor(color);
};

void addColor(uint8_t color){
	/*
	RED='\033[0;31m'
	NC='\033[0m' # No Color
	Black        0;30     Dark Gray     1;30
	Red          0;31     Light Red     1;31
	Green        0;32     Light Green   1;32
	Brown/Orange 0;33     Yellow        1;33
	Blue         0;34     Light Blue    1;34
	Purple       0;35     Light Purple  1;35
	Cyan         0;36     Light Cyan    1;36
	Light Gray   0;37     White         1;37
	*/
	if(color==COLOR_RED){
		Serial.print((char*)"\033[0;31m");
	} else if(color==COLOR_GREEN){
		Serial.print((char*)"\033[0;32m");
	} else if(color==COLOR_YELLOW){
		Serial.print((char*)"\033[0;33m");
	} else if(color==COLOR_PURPLE){
		Serial.print((char*)"\033[0;35m");
	}

}
void remColor(uint8_t color){
	if(color!=COLOR_NONE){
		Serial.print((char*)"\033[0m");
	}
}
