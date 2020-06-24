#ifndef configuration_h
	#define configuration_h
    #include <Arduino.h>
    #include <EEPROM.h>
    #define configuration_LENGTH 5+48+48+48+50+48+6+48+48+48+48

struct configuration_data { //
    char simPIN[5];
    char apn[48];
    char apn_user[48];
    char apn_pw[48];
    char dev[50];
    char mqtt_server[48];
    char mqtt_port[6];
    char mqtt_user[48];
    char mqtt_pw[48];
    char owntracks_prefix[48];
    char owntracks_user[48];
};


class configuration {
    public:
        configuration();
        ~configuration();
        boolean load();
        boolean store();
        boolean store(char * temp);
        bool SerialProgramming();

        void explainFullStruct();
        void explainStruct(uint8_t i, boolean rn);
        char* getElement(uint8_t i);

        boolean inConfig;
        configuration_data cnfg;

    private:
        uint8_t* p;
        uint16_t f_p;
        uint16_t f_start;
        uint8_t m_size[11];
       	uint8_t skip_last;
};

#endif