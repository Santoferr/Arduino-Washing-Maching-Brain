/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
*********/

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <EasyNTPClient.h>
#include <WiFiUdp.h>

// Replace with your network credentials
const char* ssid     = "Smart Home";
const char* password = "9852174563";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state

String washState = "off";


// Assign output variables to GPIO pins
const int output16 = 16; //D0
const int output5 = 5; //D1
const int output4 = 4; //D2
const int output0 = 0; //D3
const int output2 = 2; //D4
const int output14 = 14; //D5
const int output12 = 12; //D6


// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;



WiFiUDP udp;

EasyNTPClient ntpClient(udp, "pool.ntp.org", ((5 * 60 * 60)+(30 * 60))); // IST = GMT + 5:30

int test_coef = 1;
int test_coef_default = test_coef;

unsigned long fillTimeStart = 0;
unsigned long fillWithDetergentTimeStart = 0;
unsigned long heatTimeStart = 0;
unsigned long whashTimeStart = 0;
unsigned long rinseTimeStart = 0;
unsigned long drainTimeStart = 0;
unsigned long dryTimeStart = 0;
int endCycle = 0;

unsigned long fillDuration = 0;
unsigned long heatDuration = 0;
unsigned long fillWithDetergentDuration = 0;
unsigned long whashDuration = 0;
unsigned long rinseDuration = 0;
unsigned long drainDuration = 0;
unsigned long dryDuration = 0;
int step = -1;

char *steps[17] = { "lock", "fill", "detergent", "heat", "wash", "drain", "rinse","dry", "drain","rinse", "drain", "dry","drain", "dry", "drain", "unlock" };

//char *steps[3] = { "lock", "dry", "unlock" };
char * currentStep;

const int analogInPin = A0;  // ESP8266 Analog Pin ADC0 = A0

int sensorValue = 0;  // value read from the pot
int outputValue = 0;  // value to output to a PWM pin


int   gpio_fix = 1;


void initGPIOS(boolean boot) {

  // Set outputs to LOW
  digitalWrite(output16, HIGH);
  digitalWrite(output5, HIGH);
  digitalWrite(output4, HIGH);
  digitalWrite(output0, boot == true ? LOW : HIGH); // fix for normal boot
  digitalWrite(output2, boot == true ? LOW : HIGH); // fix for normal boot
  digitalWrite(output14, HIGH);
  digitalWrite(output12, HIGH);
  digitalWrite(15, HIGH); // fix for normal boot
}

void setupParameters() {
  fillDuration = 60 * 1.5 / test_coef;
  heatDuration = 60 * 1 / test_coef;
  fillWithDetergentDuration = 10 / test_coef;
  whashDuration = 60 * 20 / test_coef;
  rinseDuration = 60 * 1 / test_coef;
  drainDuration = 60 * 0.5 / test_coef;
  dryDuration = 60 * 3 / test_coef;
}


void setup() {
  Serial.begin(115200);



  // Initialize the output variables as outputs
  pinMode(output16, OUTPUT);
  pinMode(output5, OUTPUT);
  pinMode(output4, OUTPUT);
  pinMode(output0, OUTPUT);
  pinMode(output2, OUTPUT);
  pinMode(output14, OUTPUT);
  pinMode(output12, OUTPUT);


  initGPIOS(true);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
     
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  Serial.printf("Default hostname: %s\n", WiFi.hostname().c_str());
WiFi.hostname("ESPWASH");
Serial.printf("New hostname: %s\n", WiFi.hostname().c_str());
  server.begin();
}

long timetook(unsigned long stepTime) {
  return (ntpClient.getUnixTime() - stepTime) / 60;
}
void nextstep(){
  step++; 
  Serial.println(step);
  delay(2000);
}

void loop() {
  if (gpio_fix == 1) {
    digitalWrite(output0, HIGH);
    digitalWrite(output2, HIGH);
    gpio_fix = 0;
  }
  WiFiClient client = server.available();   // Listen for incoming clients

  sensorValue = analogRead(analogInPin);

  // map it to the range of the PWM out
  outputValue = map(sensorValue, 0, 1023, 0, 255);



//char *steps[1] = {"test"};

  if (step > -1) {
    currentStep = steps[step];
  if (currentStep == "test") {
     digitalWrite(output2, LOW); // Lock the door
     
        digitalWrite(output0, LOW); // turn on motor
      delay(10000);
       digitalWrite(output0, HIGH); // turn on motor
      delay(5000);
      /* Serial.println("revers motor");
       *  digitalWrite(output5, LOW);   // >Right   
      digitalWrite(output16, LOW); // turn on motor
      delay(5000); //rotate for 5 seconds
      digitalWrite(output16, HIGH); //turn off motor
      delay(3000); //wait for motor to stop
      digitalWrite(output5, HIGH);   // >LEFT
       digitalWrite(output16, LOW); // turn on motor
        delay(5000); //rotate for 5 seconds
*/
        
     // nextstep();
    }
    else if (currentStep == "lock") {
      Serial.println("Locking the door");
      digitalWrite(output2, LOW);
      nextstep();
      fillTimeStart = 0;
    }
    else if (currentStep == "fill") {
      if (fillTimeStart == 0) {
        Serial.println("Filling up with water");
        digitalWrite(output16, LOW);
        fillTimeStart = ntpClient.getUnixTime();
      } else if ((ntpClient.getUnixTime() - fillTimeStart) > fillDuration) {
        Serial.print("End Filling water : ");
        Serial.print(timetook(fillTimeStart));
        Serial.println(" minutes");
        digitalWrite(output16, HIGH);
        fillTimeStart = 0;
        nextstep();

      }
    }
    else if (currentStep == "detergent") {
      if (fillWithDetergentTimeStart == 0 ) {
        Serial.println("Adding water with detergent");
        digitalWrite(output14, LOW);
        fillWithDetergentTimeStart = ntpClient.getUnixTime();
      } else if ((ntpClient.getUnixTime() - fillWithDetergentTimeStart) > fillWithDetergentDuration) {
        Serial.print("End Adding water with detergent : ");
        Serial.print(timetook(fillWithDetergentTimeStart));
        Serial.println(" minutes");
        digitalWrite(output14, HIGH);
        fillWithDetergentTimeStart = 0;
        //start heating the water to reach the temperature required by the wash program
        nextstep();
      }
    }
    else if (currentStep == "heat") {

      if (heatTimeStart == 0) {
        Serial.println("Heating water");
        digitalWrite(output12, LOW);
        heatTimeStart = ntpClient.getUnixTime();
      } else if ((ntpClient.getUnixTime() - heatTimeStart) > heatDuration) {
        Serial.print("End heating water : ");
        Serial.print(timetook(heatTimeStart));
        Serial.println(" minutes");
        digitalWrite(output12, HIGH);
        heatTimeStart = 0;
        nextstep();
      }

    }
    else if (currentStep == "wash") {
      if (whashTimeStart == 0) {
        whashTimeStart = ntpClient.getUnixTime();
        Serial.println("Start wash timer");
      } else if ((ntpClient.getUnixTime() - whashTimeStart) > whashDuration) {
        Serial.print("End spin at low speed : ");
        Serial.print(timetook(whashTimeStart));
        Serial.println(" minutes");
        digitalWrite(output5, HIGH);
        whashTimeStart = 0;
        nextstep();
      }else{
         Serial.println("Start spin at low speed");
        digitalWrite(output5, LOW);
        delay(5000);
        digitalWrite(output5, HIGH);
        delay(5000);
      }
    }
    else if (currentStep == "drain") {
      if (drainTimeStart == 0) {
        //start Drain after washing
        drainTimeStart = ntpClient.getUnixTime();
        Serial.println("Start Drain to evacuate detergent");
        digitalWrite(output0, LOW);
      } else if ((ntpClient.getUnixTime() - drainTimeStart) > drainDuration) {
        digitalWrite(output0, HIGH);
        Serial.print("End Drain to evacuate detergent : ");
        Serial.print(timetook(drainTimeStart));
        Serial.println(" minutes");
        drainTimeStart = 0;
       nextstep();
      }
    }
    else if (currentStep == "rinse") {

      if ( rinseTimeStart == 0) {
        rinseTimeStart = ntpClient.getUnixTime();
        digitalWrite(output16, LOW);
        Serial.println("Start Rinsing filing with water again");
      } else if ((ntpClient.getUnixTime() - rinseTimeStart) > rinseDuration) {
        Serial.println("End Rinsing : ");
        Serial.print(timetook(rinseTimeStart));
        Serial.println(" minutes");
        digitalWrite(output16, HIGH);
        rinseTimeStart = 0;
       nextstep();

      }
    }
    else if (currentStep == "dry") {

      if (dryTimeStart == 0) {
      
        //start Drain after washing
        dryTimeStart = ntpClient.getUnixTime();

        for (int i=0;i<20; i++){
          digitalWrite(output4, LOW);
          delay(300);
           digitalWrite(output4, HIGH);
          delay(500);
          }
        
      } else   if ((ntpClient.getUnixTime() - dryTimeStart) > dryDuration) {
        digitalWrite(output4, HIGH);
        Serial.println("End Drying : ");
        Serial.print(timetook(dryTimeStart));
        Serial.println(" minutes");
        dryTimeStart = 0;
       nextstep();
      }else{
               Serial.println("Start Drying");
          digitalWrite(output4, LOW);
          delay(500);
           digitalWrite(output4, HIGH);
          delay(500);
      }
    }
    else if (currentStep == "unlock") {
      Serial.println("Unlocking the door");
      digitalWrite(output2, HIGH);
     nextstep();
    }

  }




  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /wash") >= 0) {
              washState = "on";
              //Start fill water
              step = 0;
              test_coef = test_coef_default;
              setupParameters();
            }

            if (header.indexOf("GET /halt") >= 0) {
              washState = "off";
              step = -1;
              initGPIOS(false);
              test_coef = test_coef_default;
            }

            if (header.indexOf("GET /test") >= 0) {
              washState = "on";
              //Start fill water
              step = 0;
              test_coef = 36;
              setupParameters();
            }

            // test_coef


            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.4.1/css/bootstrap.min.css\" integrity=\"sha384-Vkoo8x4CGsO3+Hhxv8T/Q5PaXtkKtu6ug5TOeNV6gBiFeWPGFN9MuhOf23Q9Ifjh\" crossorigin=\"anonymous\">");

            // Web Page Heading
            client.println("<body>");
            client.println("<div class=\"container-sm\">");


            client.println("<h1>ESP8266 Washing Brain</h1>");




            client.println("<p>Start Cycle " + washState + "</p>");


            client.println("<p><a href=\"/wash\"><button class=\"btn btn-success\">Wach</button></a></p>");

            client.println("<p><a href=\"/halt\"><button class=\"btn btn-danger\">Halt</button></a></p>");

            client.println("<p><a href=\"/test\"><button class=\"btn btn-primary\">Test</button></a></p>");

            client.println("<textarea cols='200' rows='10' >");
            client.print("sensor = ");
            client.println(sensorValue);
            client.print("output = ");
            client.println(outputValue);
            client.println("</textarea>");
            client.println("</div>");

            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");

  }
  delay(1000);
}
