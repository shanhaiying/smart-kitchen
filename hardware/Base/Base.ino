#include <SoftwareSerial.h>
#include <Phant.h>

#define DEBUG FALSE

typedef unsigned int uint;

////////////// PER USER /////////////////
static const uint8_t UUID = 23;
/////////////////////////////////////////
const int BAUD = 9600;

const int COMMAND_TIMEOUT = 3000;  // ms
const int STARTUP_DELAY = 5000;
 
// Xbee globals.
const byte XBEE_RX = 2;
const byte XBEE_TX = 3;
SoftwareSerial xbee(XBEE_RX, XBEE_TX); 

const byte BLE_RX = 4;
const byte BLE_TX = 5;
SoftwareSerial scanner(BLE_RX, BLE_TX);

// Phant globals.
// const String destIP = "54.86.132.254";     // packet.sparkfun.com's IP address
// const String publicKey = "YDdNyD6Wymf9ayKlrDR3";
// const String privateKey = "RnB7GnXWG6S0ZkAwDM8B";

// Amazon ec2 acting as reverse proxy for Heroku App
const String domain = "ec2-52-55-175-106.compute-1.amazonaws.com";
const String destIP = "52.55.175.106";
const String endPoint = "countme/bitch";
const String privateKey = "";
const String file = "";
Phant phant(domain, endPoint, privateKey, file);

// HTTP Request POST field names
const String field_uuid = "uuid";
const String field_type = "type";
const String field_batt = "batt";
const String field_board_id = "board_id";
const String field_sensors[] = {"s0", "s1", "s2", "s3", "s4", 
                                "s5", "s6", "s7", "s8", "s9"};

const uint8_t NUM_SENSORS[] = {3, 1};
const uint MAX_SENSORS = 10;

bool valid_data = false;
struct {
  uint8_t board_type;
  uint8_t board_uuid;
  uint8_t sensors[MAX_SENSORS];
  uint8_t batt;
} packet;

// Phant limits you to 10 seconds between posts. Use this variable
// to limit the update rate (in milliseconds):
const unsigned long UPDATE_RATE = 15000;  // ms
unsigned long lastUpdate        = 0;     // Keep track of last update time

void setup() {
  delay(STARTUP_DELAY);

  // Set up serial ports:
  Serial.begin(BAUD);
  scanner.begin(BAUD);
  xbee.begin(BAUD);

  xbee.listen();

  // Set up WiFi network
  connectWiFi();
  Serial.println("Verified network connection!");
  setupHTTP(destIP);

  delay(STARTUP_DELAY);
}

char myChar;
uint8_t data_buffer[26];

void loop() {
  scanner.listen();
  bool ready = false;
  while(scanner.available()){
    ready = readScanner();
  }

  // If current time is UPDATE_RATE milliseconds greater than
  // the last update rate, send new packet.
  if (!ready || millis() <= (lastUpdate + UPDATE_RATE))
    return;

  xbee.listen();

  if (sendData()) {
    Serial.println("Update succeeded!");
  } else {
    Serial.println("Failed :-(");
  }

  lastUpdate = millis();
}

const uint MAX_DATA_BUFFER_LEN = 3*sizeof(packet) + 1;
uint8_t scannedDataBuffer[MAX_DATA_BUFFER_LEN];
const uint8_t DATA_TERMINATOR = 0xFF;
size_t scannedDataIdx = 0;
bool waitingForNullTerminator = true;

bool readScanner() {

  // Read in a char from the software serial channel.
  scannedDataBuffer[scannedDataIdx++] = scanner.read();

  // Just buffer if not end of sequence.
  if (scannedDataBuffer[scannedDataIdx-1] != DATA_TERMINATOR) {
    if (scannedDataIdx >= MAX_DATA_BUFFER_LEN) {
      scannedDataIdx = 0;
      waitingForNullTerminator = true;
    }
    return false;
  }

  // Reset if waiting.
  if (waitingForNullTerminator) {
    scannedDataIdx = 0;
    waitingForNullTerminator = false;
    return false;
  }

  // If we pass all those checks.
  deserializeScannedData();
  printPacket();
  scannedDataIdx = 0;
  return true;
}

void deserializeScannedData() {
  uint8_t* mutator = (uint8_t*) &packet;
  for (size_t i = 0; i < sizeof(packet); i++) {
    uint8_t decimal = scannedDataBuffer[i * 3] - '0';
    for (size_t j = 1; j < 3; j++) {
      decimal = decimal * 10 + (scannedDataBuffer[i * 3 + j] - '0');
    }
    mutator[i] = decimal;
  }
}

void printPacket() {
  Serial.print("Board Type: ");
  Serial.println(packet.board_type);
  Serial.print("Board uuid: ");
  Serial.println(packet.board_uuid);
  Serial.println("Sensor values: ");
  for (size_t i = 0; i < NUM_SENSORS[packet.board_type]; i++) {
    Serial.print(packet.sensors[i]);
    Serial.print(" ");
  }
  Serial.println();
}

// *****************************************************************************
// ****  FUNCTIONS     *********************************************************
// *****************************************************************************

void updatePhant() {
  phant.add(field_uuid, UUID);
  phant.add(field_type, packet.board_type);
  phant.add(field_board_id, packet.board_uuid);

  for (size_t i = 0; i < NUM_SENSORS[packet.board_type]; i++) {
    phant.add(field_sensors[i], packet.sensors[i]);
  }

  for (size_t i = NUM_SENSORS[packet.board_type]; i < MAX_SENSORS; i++) {
    phant.add(field_sensors[i], "None");
  }

  phant.add(field_batt, packet.batt);
}

bool sendData() {
  xbee.flush();
  xbee.readString();

  // if (!valid_data)
  //   return false;

  updatePhant();

  // Serial.println();
  // Serial.println(phant.post());
  // return false;

  xbee.print(phant.post());

  // Check the response to make sure we receive a "200 OK". If 
  // we were good little programmers we'd check the content of
  // the OK response. If we were good little programmers...
  char response[12];

  if (waitForAvailable(12) > 0) {
    for (int i=0; i<12; i++) {
      response[i] = xbee.read();
    }
    if (memcmp(response, "HTTP/1.1 200", 12) == 0)
      return true;
    else {
      Serial.print("Server response ---- ");
      Serial.println(response);
      return false; // Non-200 response
    }
  }
  else // Otherwise timeout, no response from server
    return false;
}

/////////////////////////////////////////////////////////////////////////////////
// xbeeee WiFi Setup Stuff //
/////////////////////////////////////////////////////////////////////////////////
// setupHTTP() sets three important parameters on the xbeeee:
// 1. Destination IP -- This is the IP address of the server we want to send packet to.
// 2. Destination Port -- We'll be sending packet over port 80. The standard HTTP port a server listens to.
// 3. IP protocol -- We'll be using TCP (instead of default UDP).

void setupHTTP(String address) {
  while (!commandMode(1))
    ;

  // Set IP (1 - TCP)
  command("ATIP1", 2);                    // RESP: OK
  // Set DL (destination IP address)
  command("ATDL" + address, 2);           // RESP: OK
  // Set DE (0x50 - port 80)
  command("ATDE50", 2);                   // RESP: OK

  commandMode(0);                         // Exit command mode when done
}

void printIP() {
  while (!commandMode(1))
    ;

  // Get rid of any packet that may have already been in the
  // serial receive buffer:
  xbee.flush();
  // Send the ATMY command. Should at least respond with "0.0.0.0\r" (7 characters):
  command("ATMY", 7);
  // While there are characters to be read, read them and throw
  // them out to the serial monitor.
  while (xbee.available() > 0) {
    Serial.write(xbee.read());
  }
  
  commandMode(0);
}

/////////////////////////////////////////////////////////////////////////////////
// Assumes you've already configured your xbeeee module's WiFi settings using X-CTU
// and simply blinks the LED while connecting or if there is a problem. We also
// turn on the LED once connected.
void connectWiFi() {
  const String CMD_SSID = "ATID";
  const String CMD_ENC = "ATEE";
  const String CMD_PSK = "ATPK";
  // Check if we're connected. If so, sweet! We're done.
  // Otherwise, time to configure some settings, and print
  // some status messages:
  int status;
  while ((status = checkConnect()) != 0)
  {
    // Print a status message. If `status` isn't 0 (indicating
    // "connected"), then it'll be one of these 
    //  (from xbeeee WiFI user's manual):
    // 0x01 - WiFi transceiver initialization in progress. 
    // 0x02 - WiFi transceiver initialized, but not yet scanning 
    //        for access point. 
    // 0x13 - Disconnecting from access point. 
    // 0x23 – SSID not configured. 
    // 0x24 - Encryption key invalid (either NULL or invalid 
    //        length for WEP) 
    // 0x27 – SSID was found, but join failed. 0x40- Waiting for 
    //        WPA or WPA2 Authentication 
    // 0x41 – Module joined a network and is waiting for IP 
    //        configuration to complete, which usually means it is
    //        waiting for a DHCP provided address. 
    // 0x42 – Module is joined, IP is configured, and listening 
    //        sockets are being set up. 
    // 0xFF– Module is currently scanning for the configured SSID.
    //
    // We added 0xFE to indicate connected but SSID doesn't match the provided id.
    Serial.print("Waiting to connect: ");
    Serial.println(status, HEX);
    delay(1000);
  }
}


// Check if the xbeeee is connected to a WiFi network.
// This function will send the ATAI command to the xbeeee.
// That command will return with either a 0 (meaning connected)
// or various values indicating different levels of no-connect.
byte checkConnect() {
  byte i=0xFF;
  char temp[2];
  commandMode(0);
  while (!commandMode(1))
    ;
  command("ATAI", 2);
  temp[0] = hexToInt(xbee.read());
  temp[1] = hexToInt(xbee.read());
  xbee.flush();
  
  #ifdef DEBUG
    Serial.print("temp[0] = ");   Serial.println(temp[0], HEX);
    Serial.print("temp[1] = ");   Serial.println(temp[1], HEX);
  #endif
  
  if (temp[0] == 0) {
    return 0;
  }
  else {
    return (temp[0]<<4) | temp[1];
  }
}

void command(String atcmd, int rsplen)
{
  xbee.flush();
  #ifdef DEBUG
    Serial.print("Entering command: ");
  #endif
  
  xbee.print(atcmd);
  
  #ifdef DEBUG
    Serial.println(atcmd);
  #endif 
  
  xbee.print("\r");
  waitForAvailable(rsplen);
}

int commandMode(boolean enter) {
  xbee.flush();
  
  if (enter) {
      #ifdef DEBUG
         Serial.println("Entering Command Mode");
      #endif
    char c;
    xbee.print("+++");   // Send CMD mode string
    waitForAvailable(1);
    String s = xbee.readString();
    #ifdef DEBUG
      Serial.println(s);
    #endif
    return s[0] == 'O';
  }
  else {
   #ifdef DEBUG
    Serial.println("Exiting Command Mode");
   #endif
  
    command("ATCN", 2);
    return true;
  }
}

int waitForAvailable(int qty) {
  int timeout = COMMAND_TIMEOUT;

  while ((timeout-- > 0) && (xbee.available() < qty))
    delay(1);

  return timeout;
}

byte hexToInt(char c) {
  if (c >= 0x41) // If it's A-F
    return c - 0x37;
  else
    return c - 0x30;
}
