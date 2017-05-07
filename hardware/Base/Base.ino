#include <SoftwareSerial.h>
#include <Phant.h>

#define DEBUG FALSE

const int COMMAND_TIMEOUT = 3000  // ms
const int STARTUP_DELAY = 5000;
const int SERIAL_BAUD = 9600;

// Xbee globals.
const int  XBEE_BAUD = 9600;
const byte XBEE_RX = 2;
const byte XBEE_TX = 3;
SoftwareSerial xbee(XBEE_RX, XBEE_TX); 

const byte BLE_RX = 4;
const byte BLE_TX = 5;
SoftwareSerial scanner(BLE_RX, BLE_TX);

// Phant globals.
const String destIP = "54.86.132.254";     // data.sparkfun.com's IP address
const String publicKey = "YDdNyD6Wymf9ayKlrDR3";
const String privateKey = "RnB7GnXWG6S0ZkAwDM8B";
Phant phant("data.sparkfun.com", publicKey, privateKey);

// HTTP Request POST field names
const String field_type = "type";
const String field_uuid = "uuid";
const String field_batt = "batt";
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
} data;

// Phant limits you to 10 seconds between posts. Use this variable
// to limit the update rate (in milliseconds):
const unsigned long UPDATE_RATE = 15000;  // ms
unsigned long lastUpdate        = 0;     // Keep track of last update time

void setup() {
  delay(STARTUP_DELAY);

  // Set up serial ports:
  Serial.begin(SERIAL_BAUD);
  xbee.begin(XBEE_BAUD);

  // Set up WiFi network
  Serial.println("Verifying network...");
  connectWiFi();
  Serial.println("Verified!");
  // TODO - maybe print network name
  setupHTTP(destIP);

  delay(STARTUP_DELAY);
}

void loop() {
  bool ready = readScanner();

  // If current time is UPDATE_RATE milliseconds greater than
  // the last update rate, send new data.
  if (ready && millis() > (lastUpdate + UPDATE_RATE)) {
    Serial.print("\nSending update...");
    if (sendData()) {
      Serial.println(" SUCCESS!");
    } else {
      Serial.println(" Failed :(");
    }
    lastUpdate = millis();
  }
  Serial.print(".");

  delay(2000);
}

const String WRAP = {0xFF,0x00,0xFE,0x01,0xFD, 0x02};
size_t verify_idx = 0;
typedef enum scanningStep {PRE, MAIN, POST};
enum scanningStep currentScanStep = PRE;

bool readScanner() {
  switch (scanningStep) {
    case PRE:
      if (scanner.read() != WRAP[verify_idx]) {
        verify_idx = 0;
      }
      if (verify_idx == WRAP.length() - 1) {
        verify_idx = WRAP.length() - 1;
        scanningStep= MAIN;
      } else {
        verify_idx++;
      }
      return false;

    case MAIN:
      scanner.readBytes((uint8_t*) &data, sizeof(data));
      currentScanStep = POST;
      return false;

    case POST:
      if (scanner.read() != WRAP[verify_idx]) {
        verify_idx = 0;
        return false;
      }
      if (verify_idx < 0) {
        verify_idx = 0;
        currentScanStep = PRE;
        return true;
      }
      verify_idx--;
      return false;
  }
}

// *****************************************************************************
// ****  FUNCTIONS     *********************************************************
// *****************************************************************************

void updatePhant() {
  phant.add(field_type, data.board_type);
  phant.add(field_uuid, data.board_uuid);

  for (size_t i = 0; i < data.num_sensors; i++) {
    phant.add(field_sensors[i], data.sensors[i]);
  }

  for (size_t i = data.num_sensors; i < MAX_SENSORS; i++) {
    phant.add(field_sensors[i], "None");
  }

  phant.add(field_batt, data.batt);
}

int sendData() {
  xbee.flush();
  xbee.readString();

  if (!data.valid)
    return -1;

  updatePhant();

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
    return -1;
}

/////////////////////////////////////////////////////////////////////////////////
// xbeeee WiFi Setup Stuff //
/////////////////////////////////////////////////////////////////////////////////
// setupHTTP() sets three important parameters on the xbeeee:
// 1. Destination IP -- This is the IP address of the server we want to send data to.
// 2. Destination Port -- We'll be sending data over port 80. The standard HTTP port a server listens to.
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

  // Get rid of any data that may have already been in the
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
