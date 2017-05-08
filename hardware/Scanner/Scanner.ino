#include <BLE_API.h>

typedef unsigned int uint;

#define SCAN_INTERVAL 1000
#define SCAN_WINDOW   200
#define TIMEOUT       0
#define ACTIVE_SCANNING false

typedef enum board_t {FRIDGE, OVEN};
static const uint8_t NUM_SENSORS[] = {3, 1};
// NOTE: NUM_SENSORS is mapped by board_t

static const uint MAX_SENSORS = 10;
static const uint NUM_BYTES = 26;
static const uint START_INDEX = 5;
static const uint8_t HASH[] = {69,1,255,1,69}; 
static const size_t HASH_LEN = 5;

bool valid_data = false;
struct {
  uint8_t board_type;
  uint8_t board_uuid;
  uint8_t sensors[MAX_SENSORS];
  uint8_t batt;
} data;

BLE ble;

/**
 * @brief  Function to decode advertisement or scan response data
 *
 * @param[in]  type            The data type that you want to get
 * @param[in]  advdata_len     The length of advertisement or scan reponse data
 * @param[in]  *p_advdata      The pointer of advertisement or scan reponse data
 * @param[out] *len            If type exist, this is the length of field data
 * @param[out] *p_field_data   If type exist, this is the pointer of field data
 *
 * @return NRF_SUCCESS or NRF_ERROR_NOT_FOUND
 */
uint32_t bleAdvDataDecode(uint8_t type, uint8_t advdata_len, uint8_t *p_advdata, uint8_t *len, uint8_t *p_field_data) {
  uint8_t index=0;
  uint8_t field_length, field_type;

  while(index<advdata_len) {
    field_length = p_advdata[index];
    field_type   = p_advdata[index+1];
    if(field_type == type) {
      memcpy(p_field_data, &p_advdata[index+2], (field_length-1));
      *len = field_length - 1;
      return NRF_SUCCESS;
    }
    index += field_length + 1;
  }
  return NRF_ERROR_NOT_FOUND;
}

void printData() {
  Serial.print("Board Type: ");
  Serial.println(data.board_type);
  Serial.print("Board uuid: ");
  Serial.println(data.board_uuid);
  Serial.println("Sensor values: ");
  for (size_t i = 0; i < NUM_SENSORS[data.board_type]; i++) {
    Serial.print(data.sensors[i]);
    Serial.print(" ");
  }
  Serial.println();
}

const size_t WRAP_LEN = 6;
const uint8_t WRAP[WRAP_LEN] = {0xFF,0x00,0xFE,0x01,0xFD, 0x02};

void serializeData() {
  if (!valid_data)
    return;

  const uint8_t* iterator = (uint8_t*) &data;
  for (size_t i = 0; i < sizeof(data); i++) {
    char buf [4];
    sprintf (buf, "%03i", iterator[i]);
    for (size_t j = 0; j < 4 && buf[j] != '\0'; j++) {
      Serial.write(buf[j]);
    }
  }

  Serial.write((byte)0xFF);
}

bool valid(int checksum) {
  uint8_t sum = 0;
  for (size_t i = 0; i < HASH_LEN; i++) {
    sum += HASH[i];
  }
  sum *= 2;  // there are two hashes per packet
  sum += data.board_type;
  sum += data.board_uuid;
  sum += data.batt;
  for (size_t i = 0; i < NUM_SENSORS[data.board_type]; i++) {
    sum += data.sensors[i];
  }
  return sum == checksum;
}

/**
 * @brief  Callback handle for scanning device
 *
 * @param[in]  *params   params->peerAddr            The peer's BLE address
 *                       params->rssi                The advertisement packet RSSI value
 *                       params->isScanResponse      Whether this packet is the response to a scan request
 *                       params->type                The type of advertisement
 *                                                   (enum from 0 ADV_CONNECTABLE_UNDIRECTED,ADV_CONNECTABLE_DIRECTED,ADV_SCANNABLE_UNDIRECTED,ADV_NON_CONNECTABLE_UNDIRECTED)
 *                       params->advertisingDataLen  Length of the advertisement data
 *                       params->advertisingData     Pointer to the advertisement packet's data
 */
void scanCallback(const Gap::AdvertisementCallbackParams_t *params) {
  valid_data = false;
  uint8_t len;
  uint8_t adv_name[31];

  if (params->type != GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED)
    return;
  if (NRF_SUCCESS != bleAdvDataDecode(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA,
                                         params->advertisingDataLen,
                                         (uint8_t*) params->advertisingData, &len, adv_name))
  {
    return;
  }

  size_t idx = START_INDEX;

  for (size_t i = 0; i < HASH_LEN; i++) {
    if (params->advertisingData[idx] != HASH[i])
      return;
    idx++;
  }

  data.board_type = params->advertisingData[idx++];
  data.board_uuid = params->advertisingData[idx++];

  for (size_t i = 0; i < NUM_SENSORS[data.board_type]; i++) {
    data.sensors[i] = params->advertisingData[idx];
    idx++;
  }

  data.batt = params->advertisingData[idx++];

  for (size_t i = 0; i < HASH_LEN; i++) {
    if (params->advertisingData[idx] != HASH[i]) {
      return;
    }
    idx++;
  }

  uint8_t checksum = params->advertisingData[START_INDEX + NUM_BYTES-1];

  if (!valid(checksum))
    return;

  valid_data = true;
  serializeData();
}

void setup() {
  Serial.begin(9600);
  ble.init();
  ble.setScanParams(SCAN_INTERVAL, SCAN_WINDOW, TIMEOUT, ACTIVE_SCANNING);
  ble.setActiveScan(ACTIVE_SCANNING);
  ble.startScan(scanCallback);
  // Serial.println("Started scanning!!!");
}

void loop() {
  ble.waitForEvent();
}

