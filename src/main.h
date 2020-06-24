// TTGO T-Call pins
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22
// GPS pins
#define GPS_RX               19
#define GPS_TX               18
#define GPS_EN               14
#define GPS_PPS              17
// led
#define LED_PIN              13
// battery
#define BATTERY              32
#define BATTERY_PIN          0
#define BAT2PER              (2250-1630)*100

#define MQTT_CONF_MODE "mode"
#define MQTT_CONF_SKIP "skip"
#define MQTT_CONF_LOCPUBLISHES "publishes"
#define MQTT_CONF_SLEEP "sleep"
#define MQTT_STATUS "status"

#define uS_TO_S_FACTOR 1000000     /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  30//0        /* Time ESP32 will go to sleep (in seconds) 300 seconds = 5 minutes */

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define SerialAT Serial1
// Serial interface for GPS interface
#define SerialGPS Serial2

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb




bool lwt_published = false;
uint32_t epoche_lastMillis;
uint32_t epoche_lastValue;
uint8_t report_status;
bool gprs_online = false;
int gprs_rssi = 0;
bool gps_online = false;
float batt_voltage;
uint32_t five_second_update;
uint16_t pos_pub_counter=0;
uint16_t skip_location_pub;
uint16_t total_location_pub;
char topic_buffer[100];
char dev[20];


bool setPowerBoostKeepOn(int en);

char* build_topic(char* topic, bool is_publish);
char* build_topic(char* topic);
void callback(char * p_topic, byte * p_payload, uint16_t p_length);

uint16_t get_accucary(bool height);
uint32_t getTime();
int readBattery(bool reset);
uint32_t mktime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
bool publish_lwt();
bool check_connection();
void go_to_sleep();
