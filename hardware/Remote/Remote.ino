#include "SparkFunTMP102.h"
#include <Wire.h>
#include <BLE_API.h>

typedef unsigned int uint;

/* Constants. */
static const uint NUM_BYTES = 26;
static const uint8_t HASH[] = {69,1,255,1,69}; 
static const size_t HASH_LEN = 5;

typedef enum board_t {FRIDGE, OVEN};

static const uint DELAY = 1000000; // 10 seconds
static const uint ADVERTISEMENT_INTERVAL = 160;

static const char DEVICE_NAME[] = "EDLSENSE";

static const float BATT_NORMALIZER = 3.3;
static const uint TEMP_NORMALIZER = 500,
                  PRES_NORMALIZER = 1023;

// Sensors.
static const int REED_PIN = D1;
static const int FSR_PIN  = A3;
static const int VBAT_PIN = A4;
TMP102 temp_sensor(0x48);


////// UPDATE PER BOARD //////////

const enum board_t BOARD_TYPE = FRIDGE;
const uint8_t BOARD_UID  = 0x40;

//////////////////////////////////


/* Utilities. */
uint8_t checksum8(const unsigned char* buff, size_t len)
{
  uint sum;
  for (sum = 0; len != 0; len--) {
    sum += *(buff++);
  }
  return (uint8_t) sum;
}

/* Globals. */
BLE ble;
Ticker ticker;

static uint8_t AdvData[26];
bool initialized = false;


void update_data_packet() {
  for (int i = 0; i < NUM_BYTES; i++) {
    AdvData[i] = (int8_t) i;
  }
}

void update_data_packet_oven() {
  for (int i = 0; i < NUM_BYTES; i++) {
    AdvData[i] = (int8_t) i*2;
  } 
}

void update_data_packet_fridge(float temp, int pres, float batt, bool reed) {
  const uint8_t normalized_temp = (uint8_t) (temp * 255 / TEMP_NORMALIZER);
  const uint8_t normalized_pres = (uint8_t) (pres * 255 / PRES_NORMALIZER);
  const uint8_t normalized_batt = (uint8_t) (batt * 255 / BATT_NORMALIZER);
  const uint8_t normalized_reed = reed ? 0xFF: 0x00;

  uint counter = 0;

  // Set begining hash.
  for (size_t i = 0; i < HASH_LEN; i++) {
    AdvData[i] = HASH[i];
    counter++;
  }

  AdvData[counter++] = (uint8_t) BOARD_TYPE;
  AdvData[counter++] = BOARD_UID;

  AdvData[counter++] = normalized_temp;
  AdvData[counter++] = normalized_pres;
  AdvData[counter++] = normalized_reed;

  AdvData[counter++] = normalized_batt;

  // Set end hash.
  for (size_t i = 0; i < HASH_LEN; i++) {
    AdvData[counter + i] = HASH[i];
  }
  counter += HASH_LEN;

  // Pad the end with zeroes (but leave room for checksum).
  for (size_t i = counter; i < NUM_BYTES - 1; i++) {
    AdvData[i] = 0;
  }

  // Compute checksum.
  AdvData[NUM_BYTES-1] = checksum8(AdvData, NUM_BYTES-1);
}

void read_sensors(float* temp, int* pressure, bool* reed, float* batt) {
  temp_sensor.wakeup();
  *temp = temp_sensor.readTempF();
  temp_sensor.sleep();

  *pressure = analogRead(FSR_PIN);
  *batt     = analogRead(VBAT_PIN);
  *reed     = (bool) digitalRead(REED_PIN);
}

void send_advertisement() {
  // NOTE: not sure if this will work for initial broadcast
  if (!initialized) {
    ble.stopAdvertising();
    ble.clearAdvertisingPayload();
    initialized = true;
  }

  //float temp = 80.89;
  //int pressure = 987;
  //bool reed = true;
  //float batt = 3.13;

  switch (BOARD_TYPE) {
    case FRIDGE:
      float temp;
      int pressure;
      bool reed;
      float batt;
      read_sensors(&temp, &pressure, &reed, &batt);
      update_data_packet_fridge(temp, pressure, batt, reed);
      break;

    case OVEN:
      update_data_packet_oven();
      break;
  }

  ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED
                                   | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
  ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
  // NOTE: we can use ADV_NON_CONNECTABLE_UNDIRECTED but then iPhone testing app wont work with it

  // TODO - read the sensors...?

  // Serial.println(len(AdvData));
  ble.accumulateAdvertisingPayload(GapAdvertisingData::MANUFACTURER_SPECIFIC_DATA, 
                                   (const uint8_t*) AdvData,
                                   sizeof(AdvData));

  // ble.setTxPower(4);
  ble.setDeviceName((const uint8_t *) DEVICE_NAME);
  ble.setAdvertisingInterval(ADVERTISEMENT_INTERVAL);
  ble.startAdvertising();
}


void setup() {

  Serial.begin(9600);
  delay(1000);
  Serial.println("Starting advertising!");

  ble.init();

  ticker.attach_us(send_advertisement, (DELAY));
}

void loop() {
  // NOTE: not sure if we need this!
  ble.waitForEvent();
}
