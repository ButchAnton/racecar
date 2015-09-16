/****************************************************************
racecar.ino
Read data from an accelerometer and post to an EMS server on a
Replay HD camera.
Butch Anton @ SAP Labs, LLC.
16-Sep-2015
Based on some ADXL345 code from Adafruit.
****************************************************************/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

// Wi-Fi Shield
#include <SPI.h>
#include <SFE_CC3000.h>
#include <SFE_CC3000_Client.h>

// Pins for Wi-Fi Shield
#define CC3000_INT      2   // Needs to be an interrupt pin (D2/D3)
#define CC3000_EN       7   // Can be any digital pin
#define CC3000_CS       10  // Preferred is pin 10 on Uno

// Connection info data lengths
#define IP_ADDR_LEN     4   // Length of IP address in bytes
#define MAC_ADDR_LEN    6   // Length of MAC address in bytes

// "Constants" for the Wi-Fi Shield
// char ap_ssid[] = "DevRel2Go";                  // SSID of network
// char ap_ssid[] = "SAP-Guest";                  // SSID of network
char ap_ssid[] = "NETGEAR26";                  // SSID of network
// char ap_ssid[] = "Butch_ASUS";                  // SSID of network
// char ap_password[] = "21119709";          // Password of network
// char ap_password[] = "";          // Password of network
char ap_password[] = "clevertrain028";          // Password of network
// char ap_password[] = "4085297774";          // Password of network
unsigned int ap_security = WLAN_SEC_WPA2; // Security of network
// unsigned int ap_security = WLAN_SEC_UNSEC; // Security of network
unsigned int timeout = 30000;             // Milliseconds
// char *server = "ec2-52-7-151-201.compute-1.amazonaws.com";
// IPAddress serverIP(10,0,1,102);
IPAddress serverIP(192,168,1,3);
int port = 5555;

// Wi-Fi shield globals
SFE_CC3000 wifi = SFE_CC3000(CC3000_INT, CC3000_EN, CC3000_CS);
SFE_CC3000_Client client = SFE_CC3000_Client(wifi);

#define MAX_TCP_CONNECT_RETRIES 10
int tcpConnectFails = 0;

// HTTP headers
String content_type = "application/json";

// Time between reports
#define TIME_BETWEEN_REPORTS 1000 // miliseconds

// Create a sensor instance with a unique ID.
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(6466);

#if 0
void displaySensorDetails(void)
{
  sensor_t sensor;
  accel.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" m/s^2");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" m/s^2");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" m/s^2");
  Serial.println("------------------------------------");
  Serial.println();
  delay(500);
}

void displayDataRate(void)
{
  Serial.print  ("Data Rate:    ");

  switch(accel.getDataRate())
  {
    case ADXL345_DATARATE_3200_HZ:
      Serial.print  ("3200 ");
      break;
    case ADXL345_DATARATE_1600_HZ:
      Serial.print  ("1600 ");
      break;
    case ADXL345_DATARATE_800_HZ:
      Serial.print  ("800 ");
      break;
    case ADXL345_DATARATE_400_HZ:
      Serial.print  ("400 ");
      break;
    case ADXL345_DATARATE_200_HZ:
      Serial.print  ("200 ");
      break;
    case ADXL345_DATARATE_100_HZ:
      Serial.print  ("100 ");
      break;
    case ADXL345_DATARATE_50_HZ:
      Serial.print  ("50 ");
      break;
    case ADXL345_DATARATE_25_HZ:
      Serial.print  ("25 ");
      break;
    case ADXL345_DATARATE_12_5_HZ:
      Serial.print  ("12.5 ");
      break;
    case ADXL345_DATARATE_6_25HZ:
      Serial.print  ("6.25 ");
      break;
    case ADXL345_DATARATE_3_13_HZ:
      Serial.print  ("3.13 ");
      break;
    case ADXL345_DATARATE_1_56_HZ:
      Serial.print  ("1.56 ");
      break;
    case ADXL345_DATARATE_0_78_HZ:
      Serial.print  ("0.78 ");
      break;
    case ADXL345_DATARATE_0_39_HZ:
      Serial.print  ("0.39 ");
      break;
    case ADXL345_DATARATE_0_20_HZ:
      Serial.print  ("0.20 ");
      break;
    case ADXL345_DATARATE_0_10_HZ:
      Serial.print  ("0.10 ");
      break;
    default:
      Serial.print  ("???? ");
      break;
  }
  Serial.println(" Hz");
}

void displayRange(void)
{
  Serial.print  ("Range:         +/- ");

  switch(accel.getRange())
  {
    case ADXL345_RANGE_16_G:
      Serial.print  ("16 ");
      break;
    case ADXL345_RANGE_8_G:
      Serial.print  ("8 ");
      break;
    case ADXL345_RANGE_4_G:
      Serial.print  ("4 ");
      break;
    case ADXL345_RANGE_2_G:
      Serial.print  ("2 ");
      break;
    default:
      Serial.print  ("?? ");
      break;
  }
  Serial.println(" g");
}
#endif // 0

void postToService(char *server, int port, String body) {
  // Make a TCP connection to remote host
  Serial.print("Performing HTTP POST to ");
  Serial.print(server);
  Serial.print (" on port ");
  Serial.println(port);

  if (!client.connect(server, port)) {
    Serial.println("Error: Could not make a TCP connection");
    tcpConnectFails++;
    if (tcpConnectFails > MAX_TCP_CONNECT_RETRIES) {
      Serial.println("Halting.");
      while(1);
    } else {
      tcpConnectFails = 0;
      Serial.println("Sending POST request ...");
      client.print("POST ");
      Serial.print("POST ");
      // client.print(baseService);
      // Serial.print(baseService);
      // client.print(service);
      // Serial.print(service);
      client.print("/");
      Serial.print("/");
      client.println(" HTTP/1.1");
      Serial.println(" HTTP/1.1");
      client.print("Host: ");
      Serial.print("Host: ");
      client.println(server);
      Serial.println(server);
      // client.print("Authorization: ");
      // Serial.print("Authorization: ");
      // client.println(authorization);
      // Serial.println(authorization);
      client.print("Content-Type: ");
      Serial.print("Content-Type: ");
      client.println(content_type);
      Serial.println(content_type);
      client.print("Content-Length: ");
      Serial.print("Content-Length: ");
      client.println(String(body.length()));
      Serial.println(String(body.length()));
      client.print("\r\n");
      Serial.print("\r\n");
      client.println(body);
      Serial.println(body);
      delay(500);

      while (client.available()) {
        char c = client.read();
        Serial.print(c);
      }
      client.close();
      // delay(500);   // Don't know why, but if we don't, things wedge.  I hate timing problems.
      Serial.println("\n************************* End of transaction *************************");
    }
  }
}

String createAccelerationBody(sensors_vec_t acceleration) {
  String body = "{ \"x\" : \"" + String(acceleration.x) + "\", \"y\" : \"" + String(acceleration.y) + "\", \"z\" : \"" + String(acceleration.z) + "\" }";
  Serial.print("createAccelerationBody: body: "); Serial.println(body);
  return body;
}

void postAccelerationData(sensors_vec_t acceleration, char *server, int port) {
  String body = createAccelerationBody(acceleration);
  postToService(server, port, body);
}

void postAccelerationData(sensors_vec_t acceleration, IPAddress server, int port) {
  String body = createAccelerationBody(acceleration);

  // This is so gross.  Convert the IPAddress into a character array so that we can have only one version
  // of the posting code.

  String serverString = String(server[0]) + "." + String(server[1]) + "." + String(server[2]) + "." + String(server[3]);
  int array_length = serverString.length() + 1;
  char serverChars[array_length];
  serverString.toCharArray(serverChars, array_length);
  postToService(serverChars, port, body);
}

// Print out an IP Address in human-readable format
#if 0
void printIPAddr(unsigned char ip_addr[])
{
  int i;

  for (i = 0; i < IP_ADDR_LEN; i++) {
    Serial.print(ip_addr[i]);
    if ( i < IP_ADDR_LEN - 1 ) {
      Serial.print(".");
    }
  }
}
#endif // o

void setup(void)
{
  ConnectionInfo connection_info;
  int i = 0;

  // Initialize Serial port
  Serial.begin(115200);

#if 1
  Serial.println();
  Serial.println("---------------------------------");
  Serial.println("Race Car Camera and Accelerometer");
  Serial.println("---------------------------------");
#endif // 0

  // Initialize CC3000 (configure SPI communications)
  if (wifi.init()) {
    // Serial.println("CC3000 initialization complete");
  } else {
    // Serial.println("Something went wrong during CC3000 init!  Halting.");
    while(1);
  }

  /*
    // Start SmartConfig and wait for IP address from DHCP
    Serial.println("Starting SmartConfig");
    Serial.println("Send connection details from app now!");
    Serial.println("Waiting to connect...");
    if ( !wifi.startSmartConfig(timeout) ) {
      Serial.println("Error: Could not connect with SmartConfig");
    }
    */

  // Connect using DHCP
  // Serial.print("Connecting to SSID: ");
  // Serial.println(ap_ssid);
  if (!wifi.connect(ap_ssid, ap_security, ap_password, timeout)) {
    // Serial.println("Error: Could not connect to AP.  Halting.");
    while(1);
  }

  // Print out connection details
  if (!wifi.getConnectionInfo(connection_info)) {
    // Serial.println("Error: Could not obtain connection details.  Halting.");
    while(1);
  } else {
    // Serial.print("Connected to SSID: ");
    // Serial.println(ap_ssid);
  }

#if 0
  // Print MAC address
  Serial.print("CC3000 MAC Address: ");
  for ( i = 0; i < MAC_ADDR_LEN; i++ ) {
    if ( connection_info.mac_address[i] < 0x10 ) {
      Serial.print("0");
    }
    Serial.print(connection_info.mac_address[i], HEX);
    if ( i < MAC_ADDR_LEN - 1 ) {
      Serial.print(":");
    }
  }
  Serial.println();

  // Print IP Address
  Serial.print("IP Address: ");
  printIPAddr(connection_info.ip_address);
  Serial.println();
#endif // 0

#if 0
  // Print subnet mask
  Serial.print("Subnet Mask: ");
  printIPAddr(connection_info.subnet_mask);
  Serial.println();

  // Print default gateway
  Serial.print("Default Gateway: ");
  printIPAddr(connection_info.default_gateway);
  Serial.println();

  // Print DHCP server address
  Serial.print("DHCP Server: ");
  printIPAddr(connection_info.dhcp_server);
  Serial.println();

  // Print DNS server address
  Serial.print("DNS Server: ");
  printIPAddr(connection_info.dns_server);
  Serial.println();

  // Print SSID
  Serial.print("SSID: ");
  Serial.println(connection_info.ssid);
  Serial.println();
#endif // 0

  /* Initialise the sensor */
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while(1);
  }

  /* Set the range to whatever is appropriate for your project */
  accel.setRange(ADXL345_RANGE_16_G);
  // displaySetRange(ADXL345_RANGE_8_G);
  // displaySetRange(ADXL345_RANGE_4_G);
  // displaySetRange(ADXL345_RANGE_2_G);

  // Display some basic information on this sensor
  // displaySensorDetails();

  // Display additional settings (outside the scope of sensor_t)
  // displayDataRate();
  // displayRange();
  // Serial.println();

  // Sleep five seconds to let the sensor warm up.
  delay(5000);
}

void loop(void)
{
  // Get a new sensor event
  sensors_event_t event;
  accel.getEvent(&event);

  postAccelerationData(event.acceleration, serverIP, port);

  // Display the results (acceleration is measured in m/s^2)
  // Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  ");
  // Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
  // Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  "); Serial.println("m/s^2 ");
  delay(TIME_BETWEEN_REPORTS);
}