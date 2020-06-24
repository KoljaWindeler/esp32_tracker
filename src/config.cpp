#include "config.h"

configuration::configuration(){
    m_size[0]=5;
    m_size[1]=48;
    m_size[2]=48;
    m_size[3]=48;
    m_size[4]=50;
    m_size[5]=48;
    m_size[6]=6;
    m_size[7]=48;
    m_size[8]=48;
    m_size[9]=48;
    m_size[10]=48;
    
    skip_last = 0; // skip capability
    inConfig = false;
};

configuration::~configuration(){}

bool configuration::SerialProgramming(){
    if (Serial.available()) {
        uint8_t char_buffer = Serial.read();
        // Serial.print(char_buffer);
        // start char for configuration
        if (char_buffer == '?') {
            // Serial.print("@");`
            p = (uint8_t *) &cnfg;;
            f_p         = 0;
            char_buffer = 255; // enter the "if" below
            f_start     = 0;
            inConfig = true;
        }

        // jump to next field
        if (char_buffer == 13 || char_buffer == 255) {
            // Serial.print("#");
            f_start = 0;
            for (uint8_t i = 0; i <= sizeof(m_size) / sizeof(m_size[0]) - skip_last; i++) { // 1.2.3.4.5.6.7
                if (i > 0) {
                    f_start += m_size[i - 1];
                }
                //Serial.printf("+%i %i\r\n",f_p,f_start);
                // no text entered but [enter], keep content
                if(f_p == f_start && char_buffer == 13){
                    if(strlen(getElement(i))){
                        strcpy((char*)p,getElement(i));
                        f_p+=strlen(getElement(i));
                        p+=strlen(getElement(i));
                    } else {
                        *p=0x00;
                        f_p+=1;
                        p+=1;
                    }
                    Serial.print(getElement(i));
                }
                // only one char entered
                if(f_p == f_start+1 && char_buffer == 13){
                    uint8_t* a=p;
                    a--;
                    //Serial.printf("%c = a, %c = p\r\n",*a,*p);
                    if(*a==' '){
                        //Serial.println("Input started with blank, set it to 0x00");
                        *(p-1)=0x00;
                    }
                }

                // seach for the field that starts closes to our current pos
                if (f_p <= f_start) {
                    for (int ii = 0; ii < f_start - f_p; ii++) { // add as many 0x00 to the configuration as required
                        *p = 0x00;
                        p++;
                    }
                    f_p = f_start; // set our new pos to the start of that field
                    // print some shiny output
                    if (i == 0) {
                        Serial.print(F("\r\n\r\nStart readig configuration"));
                    };
                    //Serial.printf("enter %i %i\r\n",i,sizeof(m_size) / sizeof(m_size[0]));
                    if (i >= 0 && i < sizeof(m_size) / sizeof(m_size[0]) - skip_last) {
                        Serial.println("");
                        explainStruct(i, false);
                        if(strlen(getElement(i))){
                            Serial.printf("[%s]\r\n",getElement(i));
                        } else {
                            Serial.println("[(blank)]");
                        }
                    } else if (i == sizeof(m_size) / sizeof(m_size[0]) - skip_last) { // last segement .. save and reboot
                        // fill the buffer
                        Serial.print(F("\r\n==========\r\nconfiguration stored\r\n"));
                        delay(1000);
                        explainFullStruct();
                        Serial.print(F("==========\r\n"));
                        // write to address 0 ++
                        f_start = 0;
                        for (uint8_t i = 0; i < sizeof(m_size) / sizeof(m_size[0]); i++) { // 1.2.3.4.5.6.7
                            f_start += m_size[i];
                        }
                        store();
                        delay(1000);
                        inConfig = false;
                        Serial.print(F("Restart now"));
                        delay(500);
                        ESP.restart();
                    }
                    break; // leave loop
                }     // if(fp<f_start)
            } // loop over struct size
                // set pointer to start of the field
            p  = (uint8_t *) &cnfg;
            p += f_p;
        } else if (char_buffer == 127) { // backspace
            // search lowerlimit of this field
            f_start = 0;
            for (uint8_t i = 0; i < sizeof(m_size) / sizeof(m_size[0]) - skip_last; i++) { // 0.1.2.3.4.5.6.7
                // Serial.print("+");
                if (f_start + m_size[i] > f_p) { // seach for the field that starts closes to our current pos
                    break;
                }
                f_start += m_size[i];
            }
            // Serial.printf("%i--%i\r\n",f_p,f_start);
            if (f_p > f_start) {
                p--; // limits?
                f_p--;
                Serial.print((char) 0x08); // geht das? ulkig aber ja
            }
        } else if (char_buffer != 10) { // plain char storing "\r"
            // Serial.print("&");
            // calc next segment

            f_start = 0;
            for (uint8_t i = 0; i < sizeof(m_size) / sizeof(m_size[0]) - skip_last; i++) { // 0.1.2.3.4.5.6.7
                // Serial.print("+");
                f_start += m_size[i];
                if (f_p < f_start) {  // seach for the field that starts closes to our current pos
                    break;
                }
            }
            // if(f_p<sizeof(*m_mqtt)-1){ // go on as long as we're in the structure
            if (f_p < f_start - 1) { // go on as long as we're in the structure
                // e.g.: first field is 16 byte long (f_start=16), we can use [0]..[14], [15] must be 0x00, so 13<16-1, 14<16-1; !! 15<16-1
                if (char_buffer != '\r' && char_buffer != '\n') {
                    Serial.print((char) char_buffer);
                }
                *p = char_buffer; // store incoming char in struct
                p++;
                f_p++;
                // } else {
                //  Serial.println("L");
            }
        }
    }
}


void configuration::explainFullStruct(){
	for(uint8_t i=0; i<11; i++){
		explainStruct(i, false);
        if(strlen(getElement(i))==0){
            Serial.println("(blank)");
        } else {
    		Serial.println(getElement(i));
        }
	}
}

void configuration::explainStruct(uint8_t i, boolean rn){
	if (rn) {
		Serial.print(F("\r\n"));
	}
    if (i == 0) {
		Serial.print(F("SIM Pin: "));
    } else if (i == 1) {
		Serial.print(F("APN url: "));
	} else if (i == 2) {
		Serial.print(F("APN user: "));
	} else if (i == 3) {
		Serial.print(F("APN pw: "));
	} else if (i == 4) {
		Serial.print(F("device name: "));
	} else if (i == 5) {
		Serial.print(F("MQTT server URL: "));
	} else if (i == 6) {
		Serial.print(F("MQTT server port: "));
	} else if (i == 7) {
		Serial.print(F("MQTT server user: "));
	} else if (i == 8) {
		Serial.print(F("MQTT server pw: "));
	} else if (i == 9) {
		Serial.print(F("owntracks prefix: "));
    } else if (i == 10) {
		Serial.print(F("owntracks user: "));
	}
	if (rn) {
		Serial.print(F("\r\n"));
	}
};

char* configuration::getElement(uint8_t i){
	if(i==0){
		return cnfg.simPIN;
    } else if(i==1){
		return cnfg.apn;
	} else if(i==2){
		return cnfg.apn_user;
	} else if(i==3){
		return cnfg.apn_pw;
	} else if(i==4){
		return cnfg.dev;
	} else if(i==5){
		return cnfg.mqtt_server;
	} else if(i==6){
		return cnfg.mqtt_port;
	} else if(i==7){
		return cnfg.mqtt_user;
    } else if(i==8){
		return cnfg.mqtt_pw;
    } else if(i==9){
		return cnfg.owntracks_prefix;
	} 
	return cnfg.owntracks_user;
}

boolean configuration::load(){
	EEPROM.begin(512); // can be up to 4096
	uint8_t checksum = 0xf1; // random
    char* p_cnfg = (char*)&cnfg;
	
    for (int i = 0; i < configuration_LENGTH; i++) {
		*p_cnfg = EEPROM.read(i);
		//Serial.printf(" (%i)",i);
		//Serial.print(char(*p_cnfg));
		checksum ^= *p_cnfg;
		p_cnfg++;
	}
	uint8_t c1 = EEPROM.read(configuration_LENGTH);
	uint8_t c2 = EEPROM.read(configuration_LENGTH+1);
    checksum ^= c1;
	if (checksum == 0x00 && c1 == c2) {
		// Serial.println("EEok");
		return true;
	} else {
        sprintf(cnfg.simPIN, "new");
		sprintf(cnfg.apn, "new");
		sprintf(cnfg.apn_user, "new");
		sprintf(cnfg.apn_pw, "new");
		sprintf(cnfg.dev, "new");
		sprintf(cnfg.mqtt_server, "new");
		sprintf(cnfg.mqtt_port, "1883");
		sprintf(cnfg.mqtt_user, "newUser");
		sprintf(cnfg.mqtt_pw, "newPW");
		sprintf(cnfg.owntracks_prefix, "owntracks");
        sprintf(cnfg.owntracks_user, "newUser");
		store();
		return false;
	}
}

boolean configuration::store(){
    return store((char*)&cnfg);
};

boolean configuration::store(char * temp){
	EEPROM.begin(512); // can be up to 4096
	uint8_t checksum = 0xf1;
	//return true; // debug first
	for (int i = 0; i < configuration_LENGTH; i++) {
		EEPROM.write(i, *temp);
		// Serial.print(int(*temp));
		checksum ^= *temp;
		temp++;
	}
	EEPROM.write(configuration_LENGTH, checksum);
	// double write check
	EEPROM.write(configuration_LENGTH+1, checksum);
	EEPROM.commit();
	Serial.printf("%i byte written\r\n",configuration_LENGTH);
	return true;
}