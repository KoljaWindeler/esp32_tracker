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
// battery
#define BATTERY              32
#define BAT2PER              (2250-1630)*100

#define MQTT_CONF_MODE "mode"
#define MQTT_CONF_SKIP "skip"
#define MQTT_CONF_LOCPUBLISHES "publishes"
#define MQTT_CONF_SLEEP "sleep"



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



char* build_topic(char* topic, bool is_publish);
char* build_topic(char* topic);
void callback(char * p_topic, byte * p_payload, uint16_t p_length);
uint32_t getTime();
int readBattery(bool reset);
uint32_t mktime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);