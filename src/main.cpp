// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define SerialAT Serial1
// Serial interface for GPS interface
#define SerialGPS Serial2

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb

#include <Arduino.h>
#include "main.h"
#include <Wire.h>
#include <TinyGsmClient.h>
#include <NMEAGPS.h>
#include <PubSubClient.h>
#include "time.h"
#include "logging.h"
#include "config.h"


TinyGsm modem(SerialAT);
TwoWire I2CPower = TwoWire(0);    // I2C for SIM800 (to keep it running when powered from battery)
NMEAGPS  gps;
gps_fix fix;
TinyGsmClient gsmClient(modem);   // TinyGSM Client for Internet connection
PubSubClient mqtt_client(gsmClient);
configuration cnfg;


#define uS_TO_S_FACTOR 1000000     /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  30//0        /* Time ESP32 will go to sleep (in seconds) 300 seconds = 5 minutes */

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

bool setPowerBoostKeepOn(int en){
  I2CPower.beginTransmission(IP5306_ADDR);
  I2CPower.write(IP5306_REG_SYS_CTL0);
  if (en) {
    I2CPower.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
  } else {
    I2CPower.write(0x35); // 0x37 is default reg value
  }
  return I2CPower.endTransmission() == 0;
}

char* build_topic(char* topic){
  return build_topic(topic, false);
}

char* build_topic(char* topic, bool is_publish){
  if(is_publish){
    if(strlen(cnfg.cnfg.owntracks_prefix)>0){
      sprintf(topic_buffer,"%s/%s/%s", cnfg.cnfg.owntracks_prefix, cnfg.cnfg.owntracks_user , cnfg.cnfg.dev);
    } else {
      sprintf(topic_buffer,"%s/%s", cnfg.cnfg.owntracks_user, cnfg.cnfg.dev);
    }
  } else {
    if(strlen(cnfg.cnfg.owntracks_prefix)>0){
      sprintf(topic_buffer,"%s/%s/%s", cnfg.cnfg.owntracks_prefix, cnfg.cnfg.dev, topic);
    } else {
      sprintf(topic_buffer,"%s/%s", cnfg.cnfg.dev, topic);
    }
  }
  return topic_buffer;
}

void callback(char * p_topic, byte * p_payload, uint16_t p_length){
  if (!strcmp(p_topic, build_topic(MQTT_CONF_MODE))){
    logln("mode update");
    p_payload[p_length] = 0x00;
    uint8_t t = atoi((char*)p_payload);
    if(!strcmp((char*)p_payload, "FAST")){
      report_status = REPORT_STATUS_FAST_SLEEP;
    } else if(!strcmp((char*)p_payload, "AWAKE")){
      report_status = REPORT_STATUS_AWAKE;
    } else if(!strcmp((char*)p_payload, "INSTANT")){
      report_status = REPORT_STATUS_INSTANT_SLEEP;
    } else if(t>=0 && t<=2){
      report_status = t;
    } 
    log_update_color(fix.valid.location,fix.satellites,gprs_online,gprs_rssi,getTime(),report_status,batt_voltage);
  } else if (!strcmp(p_topic, build_topic(MQTT_CONF_SKIP))){
    p_payload[p_length] = 0x00;
    skip_location_pub = atoi((char*)p_payload);
    log("skip update to ");
    sprintf(topic_buffer,"%i",skip_location_pub);
    pln(topic_buffer, COLOR_PURPLE);
  } else if (!strcmp(p_topic, build_topic(MQTT_CONF_LOCPUBLISHES))){
    p_payload[p_length] = 0x00;
    total_location_pub = atoi((char*)p_payload);
    log("update total location updates to ");
    sprintf(topic_buffer,"%i",total_location_pub);
    pln(topic_buffer, COLOR_PURPLE);
  } else if (!strcmp(p_topic, build_topic(MQTT_CONF_SLEEP))){
    p_payload[p_length] = 0x00;
    uint8_t sleep = atoi((char*)p_payload);
    log("sleep update to ");
    sprintf(topic_buffer,"%i",sleep);
    pln(topic_buffer, COLOR_PURPLE);
    esp_sleep_enable_timer_wakeup(sleep * uS_TO_S_FACTOR);
  };
}

void setup() {
  lwt_published = false;
  epoche_lastMillis = millis();
  five_second_update = millis();
  epoche_lastValue = 0;
  gprs_online = false;
  gps_online = false;
  report_status = REPORT_STATUS_AWAKE;
  pos_pub_counter = 0;
  skip_location_pub = 1;
  total_location_pub = 3;
  // battery
  pinMode(BATTERY,INPUT);
  analogReadResolution(20);
  analogSetPinAttenuation(BATTERY,ADC_11db);
  log_update_color(fix.valid.location,fix.satellites,gprs_online,gprs_rssi,getTime(),report_status,readBattery(true)); // with initial battery reading
  
  // Set serial monitor debugging window baud rate to 115200
  SerialMon.begin(115200);

  for (uint8_t i = 0; i < 10; i++) {
		SerialMon.print(i);
		SerialMon.print(F(".. "));
		delay(250);
	}
  SerialMon.println(" ");

  Serial.println("======= CONFIG ===========");
  cnfg.load();
  cnfg.explainFullStruct();
  Serial.println("======= CONFIG ===========");

  // set serial for GPS
  SerialGPS.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  // Start I2C communication
  I2CPower.begin(I2C_SDA, I2C_SCL, 400000);
  

  // Keep power when running from battery
  log(F("IP5306 KeepOn..."));
  if(setPowerBoostKeepOn(1)){
    pln(" OK",COLOR_GREEN);
  } else {
    pln(" FAILED",COLOR_RED);
  }

  // Set modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  pinMode(GPS_EN, OUTPUT);
  digitalWrite(GPS_EN, HIGH);

  // Set GSM module baud rate and UART pins
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  // Restart SIM800 module, it takes quite some time
  // To skip it, call init() instead of restart()
  if(strcmp(cnfg.cnfg.apn,"new")){
    logln(F("Initializing modem..."));
    //modem.restart();
    modem.init();
    // use modem.init() if you don't need the complete restart

    // Unlock your SIM card with a PIN if needed
    //if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
    //  modem.simUnlock(simPIN);
    //}
    mqtt_client.setServer((char*)cnfg.cnfg.mqtt_server,atoi(cnfg.cnfg.mqtt_port));
    mqtt_client.setCallback(callback); // in main.cpp

    // Configure the wake up source as timer wake up  
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  }
}



uint32_t mktime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second){
  int i;
  uint32_t seconds;
  static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  if(year > 1970){
    year -= 1970;
  }
  seconds= year*(SECS_PER_DAY * 365);

  for (i = 0; i < year; i++) {
    if (LEAP_YEAR(i)) {
      seconds +=  SECS_PER_DAY;   // add extra days for leap years
    }
  }
  
  // add days for this year, months start from 1
  for (i = 1; i < month; i++) {
    if ( (i == 2) && LEAP_YEAR(year)) { 
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
    }
  }
  seconds+= (day-1) * SECS_PER_DAY;
  seconds+= hour * SECS_PER_HOUR;
  seconds+= minute * SECS_PER_MIN;
  seconds+= second;

  epoche_lastMillis = millis();
  epoche_lastValue = seconds;

  return seconds; 
}

uint32_t getTime(){
  return epoche_lastValue+(millis()-epoche_lastMillis)/1000;
}


bool publish_lwt(){
  if(getTime()>1000){
    char msg[100];
    sprintf(msg, "{\"_type\":\"lwt\",\"tst\":%u}",getTime());
    if(mqtt_client.publish(build_topic("",true),msg,true)){
      logln(F("LWT published"));
      lwt_published = true;
    }
  } else {
    lwt_published = false;
  }
  return lwt_published;
}


bool check_connection(){
  gprs_online = false;
  if(!modem.isGprsConnected()){
    logln(F("Modem not connected, trying to reconnect..."));
    log(F("Connecting to APN: "));
    SerialMon.print(cnfg.cnfg.apn);
    if (!modem.gprsConnect(cnfg.cnfg.apn, cnfg.cnfg.apn_user, cnfg.cnfg.apn_pw)) {
      pln(" fail",COLOR_RED);
    } else {
      pln(" ok",COLOR_GREEN);
    }
  }

  if(modem.isGprsConnected()){
    if(!mqtt_client.connected()){
      log(F("MQTT not connected, trying to reconnect"));
      if(!mqtt_client.connect("test_id", cnfg.cnfg.mqtt_user, cnfg.cnfg.mqtt_pw, build_topic("",true), 0, true, "lost signal")){
        pln(" FAILED",COLOR_RED);
      } else {
        pln(" OK",COLOR_GREEN);
        logln(F("Subscription...  "));
        bool ret = true;
        //// mode //////
        ret = mqtt_client.subscribe(build_topic(MQTT_CONF_MODE)) & ret;
        log(build_topic(MQTT_CONF_MODE));
        if(ret){
          pln(" OK",COLOR_GREEN);
        } else {
          pln(" FAILED",COLOR_RED);
        };
        mqtt_client.loop();
        mqtt_client.loop();
        ////// skip //////
        ret = mqtt_client.subscribe(build_topic(MQTT_CONF_SKIP)) & ret;
        log(build_topic(MQTT_CONF_SKIP));
        if(ret){
          pln(" OK",COLOR_GREEN);
        } else {
          pln(" FAILED",COLOR_RED);
        };
        mqtt_client.loop();
        mqtt_client.loop();
        ////// sleep ///////////
        ret = mqtt_client.subscribe(build_topic(MQTT_CONF_SLEEP)) & ret;
        log(build_topic(MQTT_CONF_SLEEP));
        if(ret){
          pln(" OK",COLOR_GREEN);
        } else {
          pln(" FAILED",COLOR_RED);
        };
        mqtt_client.loop();
        mqtt_client.loop();
        //////////// amount of publishes ///////
        ret = mqtt_client.subscribe(build_topic(MQTT_CONF_LOCPUBLISHES)) & ret;
        log(build_topic(MQTT_CONF_LOCPUBLISHES));
        if(ret){
          pln(" OK",COLOR_GREEN);
        } else {
          pln(" FAILED",COLOR_RED);
        };
        mqtt_client.loop();
        mqtt_client.loop();
        
          
        if(ret){
          gprs_online = true;
        };
        if(!publish_lwt()){
          logln(F("no GSP data, so no timestamp"));
        }
      }
    } else {
      gprs_online = true;
    }
  }
  return gprs_online;
}

int readBattery(bool reset){
   float voltage = analogRead(BATTERY); // 100k - 100k, amplifaction 0.5 so multiply by 3.3*2
  if(reset){
    batt_voltage = voltage;
  } else {
    //Serial.print(voltage);
    //Serial.print(" ");
    batt_voltage = batt_voltage + ((voltage-batt_voltage)/1000);
    //Serial.print(batt_voltage);
    //Serial.print(" ");
    //Serial.println(_min(map(batt_voltage, 2000, 2440.0, 0, 100), 100));
  }
  if(batt_voltage>2250){
    batt_voltage = 2250;
  } else if(batt_voltage<1630){
    batt_voltage = 1630;
  }
  return (batt_voltage-1630)/6.2;
}

void go_to_sleep(){
  mqtt_client.disconnect();
  logln(F("MQTT disconnected"));
  gsmClient.stop();
  logln(F("Server disconnected"));
  modem.gprsDisconnect();
  logln(F("GPRS disconnected"));
  digitalWrite(GPS_EN, HIGH);
  logln(F("GPS disabled"));
  logln(F("going to sleep"));
  esp_deep_sleep_start();
}

void loop() {
  while(SerialGPS.available()){
    char c = SerialGPS.read();
    //SerialMon.write(c);
    gps.handle( c );
  }

  if(five_second_update+5000<millis()){
    gprs_rssi = modem.getSignalQuality();
    five_second_update = millis();
  };

  cnfg.SerialProgramming();

  // update time
  if(!cnfg.inConfig){
    log_update_color(fix.valid.location,fix.satellites,gprs_online,gprs_rssi,getTime(),report_status,readBattery(false));
  }
    

  // only run the code below if we're online
  if(strcmp(cnfg.cnfg.apn,"new") && !cnfg.inConfig){
    if(check_connection()){
      mqtt_client.loop();
      if(gps.available()) {
        fix = gps.read();
        // update time whenever we get new data
        if(fix.valid.time){
          mktime(fix.dateTime.year+30,fix.dateTime.month,fix.dateTime.date,fix.dateTime.hours,fix.dateTime.minutes,fix.dateTime.seconds);
        }
        // publish new MQTT data if we have a location fix
        if (fix.valid.location){
          // update all fix related data
          if(!cnfg.inConfig){
            log_update_color(fix.valid.location,fix.satellites,gprs_online,gprs_rssi,getTime(),report_status,readBattery(false));
          };
          
          // make sure that we've signed up
          if(!lwt_published){
            publish_lwt();
          }

          

          // get battery data
          uint8_t bat_chargeState = -99;
          int8_t bat_percent = -99;
          uint16_t bat_milliVolts = -9999;
          modem.getBattStats(bat_chargeState, bat_percent, bat_milliVolts);

          // Publish
          if(pos_pub_counter % skip_location_pub == 0){
            log(F("Publish location update "));
            char msg[200];
            sprintf(msg,"%i .. ",pos_pub_counter);
            p(msg,COLOR_NONE);
            sprintf(msg,"{\"_type\":\"location\",\"acc\":%i,\"alt\":%i,\"batt\":%i,\"conn\":\"m\",\"lat\":%f,\"lon\":%f,\"t\":\"u\",\"tid\":\"cw\",\"tst\":%u,\"vac\":%i,\"vel\":%i}",
              1,int(fix.altitude()), readBattery(false), fix.latitude(),fix.longitude(),getTime(),1,int(fix.speed()));    // acc value required for HA    
            if(mqtt_client.publish(build_topic("",true),msg,true)){
              pln(" ok",COLOR_GREEN);
              pos_pub_counter++;
            } else {
              pln(" FAIL",COLOR_RED);
            }
          } else {
            pos_pub_counter++;
          }
          //SerialMon.println(msg);
        }
      }
    }
  }

  // sleeping
  if(report_status == REPORT_STATUS_INSTANT_SLEEP){
    go_to_sleep();
  } else if(report_status == REPORT_STATUS_FAST_SLEEP && pos_pub_counter >= total_location_pub * skip_location_pub){
    go_to_sleep();
  } 

}