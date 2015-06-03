/*********************************************************************
Velocity sensor - to calculate velosity using photoresistors
*********************************************************************/
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
// Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

// Hardware SPI (faster, but must use certain hardware pins):
// SCK is LCD serial clock (SCLK) - this is pin 13 on Arduino Uno
// MOSI is LCD DIN - this is pin 11 on an Arduino Uno
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 3);
// Note with hardware SPI MISO and SS pins aren't used but will still be read
// and written to during SPI transfer.  Be careful sharing these pins!

#define SENSOR1_PIN 0
#define SENSOR2_PIN 1

#define SENSOR_NORMAL 0      // sensor is in normal state
#define SENSOR_SHADOWED 1    // sensor was put in shadow
#define SENSOR_CLOSED 2      // sensor was triggered
#define MAX_PULSE_DURATION 2000  // maximum shadow pulse

// sensor threshold to detect overshadow event
int light_threshold = 50;

char sensor_state[] = {'N', '*', 'X'};

short sensor1_state, sensor2_state;
int sensor1_level, sensor2_level;
int sensor1_level_prev, sensor2_level_prev;
int sensor1_level_delta, sensor2_level_delta;
unsigned long sensor1_time1, sensor1_time2;
unsigned long sensor2_time1, sensor2_time2;

void setup() {
  Serial.begin(9600);
  // init display
  display.begin();
  display.setContrast(50);
  display.display(); // show splashscreen
  delay(500);
  // init sensors
  sensor1_state = SENSOR_NORMAL;
  sensor1_level_prev = analogRead(SENSOR1_PIN);
  sensor2_state = SENSOR_NORMAL;
  sensor2_level_prev = analogRead(SENSOR2_PIN);
}

void loop(void) {
  // init display
  display.clearDisplay();   // clears the screen and buffer
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);

  unsigned long sensor_time = millis();
  String time_str = "Time: " + String(sensor_time) + String(' ');
  display.println(time_str);

  // ************* process sensor 1 *************
  sensor1_level = analogRead(SENSOR1_PIN);
  sensor1_level_delta = sensor1_level - sensor1_level_prev;

  switch (sensor1_state) {
    case SENSOR_NORMAL:
      // sensor is in normal state
      if (sensor1_level_delta >= light_threshold) {// was shadowed
        sensor1_state = SENSOR_SHADOWED;
        sensor1_time1 = sensor_time;
      }
      break;
    case SENSOR_SHADOWED:
      // sensor was put in shadow some time ago
      if ((sensor_time - sensor1_time1) > MAX_PULSE_DURATION) {
        // time is gone - go to NORMAL state without any reaction
        sensor1_state = SENSOR_NORMAL;
        sensor1_time1 = 0;
      } else {
        if (sensor1_level_delta <= -light_threshold) {// was unshadowed
          sensor1_state = SENSOR_CLOSED;
          sensor1_time2 = sensor_time;
        }
      }
      break;
    case SENSOR_CLOSED:
      // was put in the light some time ago
      break;
  }

  String s1_str1 = "Sens1: " + String(sensor1_level) + " " + String(sensor_state[sensor1_state]) + String(' ');
  display.println(s1_str1);
  String s1_str2 = String(sensor1_time1) + " " + String(sensor1_time2);
  display.println(s1_str2);
  sensor1_level_prev = sensor1_level;

  // ************* process sensor 2 *************
  sensor2_level = analogRead(SENSOR2_PIN);
  sensor2_level_delta = sensor2_level - sensor2_level_prev;

  switch (sensor2_state) {
    case SENSOR_NORMAL:
      // sensor is in normal state
      if (sensor2_level_delta >= light_threshold) {// was shadowed
        sensor2_state = SENSOR_SHADOWED;
        sensor2_time1 = sensor_time;
      }
      break;
    case SENSOR_SHADOWED:
      // sensor was put in shadow some time ago
      if ((sensor_time - sensor2_time1) > MAX_PULSE_DURATION) {
        // time is gone - go to NORMAL state without any reaction
        sensor2_state = SENSOR_NORMAL;
        sensor2_time1 = 0;
      } else {
        if (sensor2_level_delta <= -light_threshold) {// was unshadowed
          sensor2_state = SENSOR_CLOSED;
          sensor2_time2 = sensor_time;
        }
      }
      break;
    case SENSOR_CLOSED:
      // was put in the light some time ago
      break;
  }

  String s2_str1 = "Sens2: " + String(sensor2_level) + " " + String(sensor_state[sensor2_state]) + String(' ');
  display.println(s2_str1);
  String s2_str2 = String(sensor2_time1) + " " + String(sensor2_time2);
  display.println(s2_str2);
  sensor2_level_prev = sensor2_level;

  // logging
  Serial.print(time_str);
  Serial.print(s1_str1); Serial.print(s2_str1);
  Serial.println();

  display.display();
}

