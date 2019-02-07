#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#define in_thermistor_PIN A0
#define out_thermistor_PIN A1

int x = 0;
unsigned long last_updatetime_display = 0;
unsigned long update_time_display = 100;

unsigned long last_updatetime = 0;
unsigned long update_time = 250;

//thermistortable[][ADC-arvo, lampotila(*C)]
float thermistortable[][2] = {
{ 240 , 0 },
{ 336 , 10 },
{ 384 , 15 },
{ 393 , 16 },
{ 411 , 18 },
{ 432 , 20 },
{ 453 , 22 },
{ 471 , 24 },
{ 490 , 26 },
{ 509 , 28 },
{ 528 , 30 },
{ 546 , 32 },
{ 562 , 34 },
{ 578 , 36 },
{ 596 , 38 },
{ 613 , 40 },
{ 630 , 42 },
{ 648 , 44 }
};

float temps[5][2] = {
	{ 0 , 0 },
	{ 0 , 0 },
	{ 0 , 0 },
	{ 0 , 0 },
	{ 0 , 0 },
};

void setup() {
	Serial.begin(9600);

	// by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
												// init done
	display.clearDisplay();
	pinMode(in_thermistor_PIN, INPUT);
	pinMode(out_thermistor_PIN, INPUT);
}

void loop() {
	if (millis() - last_updatetime_display > update_time_display) {
		last_updatetime_display = millis();
		update_display();
	}
}

float average_temperature(int pin) {
	int a = 0;
	if (pin == in_thermistor_PIN) {
		a = 0;
	}
	else if (pin == out_thermistor_PIN) {
		a = 1;
	}

	// Lukee nykyisen, talletta nykyisen ja 4 edellist�
	temps[4][a] = temps[3][a];
	temps[3][a] = temps[2][a];
	temps[2][a] = temps[1][a];
	temps[1][a] = temps[0][a];
	temps[0][a] = read_temprature(pin);

  /*
	if (pin == in_thermistor_PIN) {
		Serial.print((temps[0][a] + temps[1][a] + temps[2][a] + temps[3][a] + temps[4][a]) / 5.0);
    Serial.print(" ");
	}
	else if (pin == out_thermistor_PIN) {
		Serial.println((temps[0][a] + temps[1][a] + temps[2][a] + temps[3][a] + temps[4][a]) / 5.0);
	}
 */

	return (temps[0][a] + temps[1][a] + temps[2][a] + temps[3][a] + temps[4][a]) / 5.0;

}

float read_temprature(int pin) {
	int raw_temp = analogRead(pin);

  if (pin == in_thermistor_PIN) {
    Serial.print(raw_temp);
    Serial.print(" ");
  }
  else if (pin == out_thermistor_PIN) {
    Serial.println(raw_temp);
  }

	for (unsigned int i = 0; i < sizeof(thermistortable) - 1; i++) {
		if (raw_temp < thermistortable[i][0]) {
			continue;
		}
		else if (raw_temp >= thermistortable[i][0] && raw_temp <= thermistortable[i + 1][0]) {
			return float(mapfloat(raw_temp, thermistortable[i][0], thermistortable[i + 1][0], thermistortable[i][1], thermistortable[i + 1][1]));
		}
		else {
			//return 0;
		}
	}
	return 1;
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
	return(((out_max - out_min) * x + in_max * out_min - in_min * out_max) / (in_max - in_min));
}

void update_display() {
	// display.clearDisplay();

	// Temperature update
	if (millis() - last_updatetime > update_time) {
		last_updatetime = millis();
		update_temperature();
	}

	print_arrow(113, x);
	print_arrow(113, x + 32);
	display.display();

	x += 2;
	if (x > 63) {
		x -= 64;
	}
}

void update_temperature() {
	display.clearDisplay();
	print_temperature(0, 0, average_temperature(in_thermistor_PIN));
	print_temperature(0, 33, average_temperature(out_thermistor_PIN));
}

void print_temperature(int x_coordinate, int y_coordinate, float temperature) {
	int integer = (int) temperature;
	int decimal = (int) ((temperature - integer) * 10.0 + 0.5);
 
  if (decimal >= 10) {
    integer +=1;
    decimal = 0;
  }

	display.setTextSize(4);
	display.setTextColor(WHITE);

	display.setCursor(x_coordinate, y_coordinate);
	if (integer < 10) {
		display.print(' ');
	}
	display.print(integer);

	display.setTextSize(3);
	display.setCursor(x_coordinate + 44, y_coordinate + 7);
	display.print(',');

	display.setTextSize(4);
	display.setCursor(x_coordinate + 60, y_coordinate);
	display.print(decimal);

	display.setTextSize(2);
	display.setCursor(x_coordinate + 84, y_coordinate - 4);
	display.print('o');

	display.setTextSize(3);
	display.setCursor(x_coordinate + 96, y_coordinate);
	display.print('C');
}


void print_arrow(int x_coordinate, int y_coordinate) {
	int color = 0;
	for (int y = 0 ; y < 32 ; y++) {
		for (int x = 0 ; x < 15 ; x++) {
			if (y < 20) {
				if (x >= 5 && x < 10) {
					color = 1;
				}
				else {
					color = 0;
				}
			}
			else if (20 <= y && y < 28) {
				if (x >= y - 20 && x < 15 - (y - 20)) {
					color = 1;
				}
				else {
					color = 0;
				}
			}
			else if (y < 32) {
				color = 0;
			}
			if (y_coordinate + y > 63) {
				display.drawPixel(x_coordinate + x, y_coordinate + y - 64, color);
			}
			else {
				display.drawPixel(x_coordinate + x, y_coordinate + y, color);
			}
		}
	}
}
