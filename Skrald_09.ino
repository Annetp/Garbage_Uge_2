
#include <ESP8266WiFi.h>

// FOR LDR
int sensorPin = A0; // select the input pin for LDR
int sensorValue = 0; // variable to store the value coming from the sensor
boolean DoorOpen = true; // initialise door to open

// FOR DISTANCE
int pingPin = D6; // Pin that sends and recives ultrasonic waves
float binTop = 5; // Distance from USM to top of rubbish bin
float binBottom = 100; // Distance from USM to bottom of rubbish bin
float full = 0; // initialising the variable full
int roundup = 0; // initialising the variable, which adjust full to specific intervals


// WiFi setup
const char* ssid = "DTU-Networks"; // Insert SSID for the accesspoint
const char* password = "Fotonik343"; // Insert Wifi key/password

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
String HTTP_req;          // stores the HTTP request
boolean LED_status = 0;   // state of LED, off by default
void ProcessCheckbox(WiFiClient cl);  // declaring function

  long microsecondsToCentimeters(long microseconds) {
    // The speed of sound is 340 m/s or 29 microseconds per centimeter.
    // The ping travels out and back, so to find the distance of the object we
    // take half of the distance travelled.
    return microseconds / 29 / 2;
  }

void setup() {
  Serial.begin(9600);
  delay(10);


  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  // FOR LDR:
  sensorValue = analogRead(sensorPin); // read the value from the sensor
  // Serial.println(sensorValue); //prints the values coming from the sensor on the screen
  delay(100);

  int sensorReading = analogRead(A0);


  if (sensorReading < 800) {
    DoorOpen = true;
    //Serial.print("eow der lyst");
  }
  else {
    DoorOpen = false;
    //Serial.print ("der er mørkt");
  }
  Serial.println(DoorOpen);


  // FOR DISTANCE
  // establish variables for duration of the ping, and the distance result in cencimeters:
  float duration, cm;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH pulse
  // whose duration is the time (in microseconds) from the sending of the ping
  // to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);

  // convert the time into a distance
  cm = microsecondsToCentimeters(duration);

  Serial.print(cm);
  Serial.print("cm");
  Serial.println();

  delay(100);



  full = float((cm / (binBottom - binTop)) * 100);

  Serial.println(full, 5);
  Serial.println(cm);
  Serial.println(binBottom);
  Serial.println(binTop);


  // Check if a client has connected
  WiFiClient client = server.available();

  if (client) {  // got client?
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {   // client data available to read
        char c = client.read(); // read 1 byte (character) from client
        HTTP_req += c;  // save the HTTP request 1 char at a time
        // last line of client request is blank and ends with \n
        // respond to client only after last line received
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println("Refresh: 2"); // Refresh webpage
          client.println();
          // send web page
          client.println("<!DOCTYPE html>");
          client.println("<html>");
          client.println("<head>");
          client.println("<title>Arduino Garbage Control</title>");
          client.println("</head>");
          client.println("<body>");
          client.println("<h1>Skraldespand </h1>");


          // Print Cupboard open or closed and garbage height
          if (DoorOpen == 0) {
            client.println("The cupboard is closed");
            client.println("<br />");
            client.println("<br />");
            full = cm / (binBottom - binTop) * 100;
         
            roundup = (int) full; // Makes full an integer
            switch (roundup) {
              case 0 > roundup < 20 :
                client.println("The garbagecan is 10 % full");
                break;
              case 21 < roundup < 40:
                client.println("The garbagecan is 30 % full");
                break;
              case 40 < roundup < 60:
                client.println("The garbagecan is 50 % full");
                break;
              case 60 < roundup < 80:
                client.println("The garbagecan is 70 % full");
                break;
              case roundup > 90:
                client.println("The garbagecan is full, please empty!");
                break;
            }

            /* client.print("The garbagecan is ");
              client.print(full);
              client.print("% full");
              client.println("<br />");
              // Mål højde og send beskreder
              if (full > 90) {
               client.println("<br />");
               client.println("The garbagecan is full, please empty!");
              } */

            else {
              client.println("The cupboard is open!");
              client.println("<br />");
              // Fremtidig udvilking:
              // LED indeni skab eller noget.
            }

            /*client.println("");
              client.println("analog input ");
              client.print("A0");
              client.print(" is ");
              client.print(sensorReading);
              client.println("<br />");

              client.print("Afstand fra top af skraldespand ");
              client.print(cm);
              client.print(" cm ");
              //client.print(sensorReading);
              client.println("<br />");*/


            // client.println("<p>Click to switch LED on and off.</p>");
            // client.println("<form method=\"get\">");
            //ProcessCheckbox(client);
            client.println("</form>");
            client.println("</body>");
            client.println("</html>");
            Serial.print(HTTP_req);
            HTTP_req = "";    // finished with request, empty string
            break;
          }
          // every line of text received from the client ends with \r\n
          if (c == '\n') {
            // last character on line of received text
            // starting new line with next character read
            currentLineIsBlank = true;
          }
          else if (c != '\r') {
            // a text character was received from client
            currentLineIsBlank = false;
          }
        } // end if (client.available())
      } // end while (client.connected())
      delay(1);      // give the web browser time to receive the data
      client.stop(); // close the connection
    } // end if (client)
  }


