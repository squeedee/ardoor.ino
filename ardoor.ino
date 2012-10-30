/*
 Doornet
 
 Based on the Arduino Example for the ethernet shield by David A Mellis

 * Ethernet shield attached to pins 10, 11, 12, 13
 * Digital door pin is on pin n
 
 */

#define WEBDUINO_SERIAL_DEBUGGING 0
#define DEBUG 1

#include <SPI.h>
#include <Ethernet.h>
#include <WebServer.h>
#include <Streaming.h>

const int door_pin =  7; 
unsigned long time_when_on = 0; 
boolean on = false;
const long timeout = 1000;

byte mac[] = { 0x7B, 0x44, 0x4F, 0x4F, 0x52, 0x7D };
IPAddress ip(192,168,0,150);

#define PREFIX ""

WebServer webserver(PREFIX, 80);

P(open_template) =
  "<html>"
  "<head>"
  "<title>Ardoorino</title>"
  "<link href='css' rel='stylesheet' type='text/css'>"
  "</head>"
  "<body>"
  "<h1>Ardoorino</h1>";

P(close_template) = 
  "</body>";  

P(door_button_page_part_1) =
  "<div class='button-container'>"
  "<form action='' method='post'>";

P(door_button_page_part_2) =
  "</form>"
  "</div>";

P(css) =
  "h1 {"
  "text-align: center;"
  "text-transform: uppercase;"
  "font-size: 22px;"
  "font-family:Arial;"
  "color: #6B1C0E;"
  "margin-top: 22px;"
  "}"
  "body {"
  "background-color:#ffffff;"
  "}"
  ".button {"
  "-moz-box-shadow: 2px 2px 7px 0px #212e16;"
  "-webkit-box-shadow: 2px 2px 7px 0px #212e16;"
  "box-shadow: 2px 2px 7px 0px #212e16;"
  "background:-webkit-gradient( linear, left top, left bottom, color-stop(0.05, #9dce2c), color-stop(1, #85ad28) );"
  "background:-moz-linear-gradient( center top, #9dce2c 5%, #85ad28 100% );"
  "filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#9dce2c', endColorstr='#85ad28');"
  "background-color:#9dce2c;"
  "-moz-border-radius:10px;"
  "-webkit-border-radius:10px;"
  "border-radius:10px;"
  "display:inline-block;"
  "color:#3f5214;"
  "font-family:Arial;"
  "font-size:22px;"
  "font-weight:bold;"
  "padding:32px 21px;"
  "text-decoration:none;"
  "text-shadow:1px 1px 1px #e7f0d8;"
  "}.button:hover {"
  "background:-webkit-gradient( linear, left top, left bottom, color-stop(0.05, #85ad28), color-stop(1, #9dce2c) );"
  "background:-moz-linear-gradient( center top, #85ad28 5%, #9dce2c 100% );"
  "filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#85ad28', endColorstr='#9dce2c');"
  "background-color:#85ad28;"
  "}.button:active {"
  "position:relative;"
  "top:1px;"
  "}"
  ".button-container {"
  "text-align: center;"
  "}"
  ".button-off {"
  "border:2px solid #62940d;"
  "}"
  ".button-on {"
  "border:2px solid #ff0000;"
  "}";

void turn_on() {
  if (on) return;
  on = true;
    
  #if DEBUG > 0
    Serial << "Turned on at ";
    Serial << time_when_on;
    Serial << "\n";
  #endif  
  
  digitalWrite(door_pin, true);

  time_when_on = millis();
}

boolean timed_out() {
  if (time_when_on < millis() - timeout) return true;
  // some error here, but its the cheapest way to deal with 'millis()' wrapping to 0
  return (time_when_on > millis() + timeout); 
}

void turn_off_after_timeout() {
  if (!on) return;
  if (!timed_out) return; // this needs a patch for when millis cycles back to 0
  on = false;
  
  #if DEBUG > 0
    Serial << "Turned off\n";
  #endif  

  digitalWrite(door_pin, false);
}

void render_root_to_server(WebServer &server) {
  server.printP(open_template);

  server.printP(door_button_page_part_1);

  // Composed button
  server << "<input class='button";
  if (on) {
    server << " button-on";
  } else { 
    server << " button-off";
  }
  server << "' type='submit' value='Open Door'>";
  server.printP(door_button_page_part_2);
  server.printP(close_template);
}

void defaultCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  server.httpSuccess();  

  if (type == WebServer::HEAD)
    return;
  if (type == WebServer::POST)
    turn_on();
    
  render_root_to_server(server);  
}

void cssCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  server.httpSuccess();
  server.printP(css);
}

void setup() {

  #if DEBUG > 0
    Serial.begin(19200);
     while (!Serial) {
      ; // wait for serial port to connect. Needed for Leonardo only
    }
  #endif

  pinMode(door_pin, OUTPUT);      

  Ethernet.begin(mac, ip);
  webserver.begin();

  webserver.setDefaultCommand(&defaultCmd);
  webserver.addCommand("index", &defaultCmd);
  webserver.addCommand("css", &cssCmd);
}


void loop() {
  webserver.processConnection();

  turn_off_after_timeout();

  delay(100);
}
