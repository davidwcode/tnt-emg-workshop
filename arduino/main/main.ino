#include <Servo.h>

Servo servo;

bool calibrated = false;
float threshold = 0.0;
int angle = 180;
int count_relax = 0;
int num_samples = 200;

const char *rest = "Rest";
const char *flex = "Flex";

// delay between servo movements
const unsigned long delta = 50;
unsigned long start_time;
unsigned long current_time;

void clearSerialBuffer() {
	while (Serial.available() > 0) {
		// discard old bytes
		Serial.read();
	}
}

void awaitSerial() {
	while (Serial.available() == 0) {
		// wait for fresh input
	}
}

void setup() {
	servo.attach(9); // servo on pin 9
	servo.write(0); // reset servo

	Serial.begin(9600);
	while (!Serial) {
		// wait for Serial to be ready
	}

	start_time = millis();
}

float calibrate(const char *type) {
	awaitSerial();
	clearSerialBuffer();

	float vals = 0;
	float cur = 0;

	Serial.println("Calibration Beginning...");
	delay(3000);
	Serial.print(type);
	Serial.println(" in 3...");
	delay(1000);
	Serial.println("2...");
	delay(1000);
	Serial.println("1...");
	delay(1000);
	Serial.println(type);

	clearSerialBuffer();

	for (int i = 0; i < num_samples; i++) {
		while (Serial.available() == 0) {
			// wait for fresh input
		}
		cur = Serial.readStringUntil('\n').toFloat();
		if (i == 0) {
			// skip first value to clear buffer
			continue;
		}
		vals += abs(cur);
		Serial.print(i);
		Serial.print(" ");
		Serial.println(vals);
	}

	return vals / num_samples - 1;
}

void loop() {
	current_time = millis();

	// calibrate threshold first
	if (!calibrated) {
		float restAvg = calibrate(rest);
		float flexAvg = calibrate(flex);

		Serial.print("Rest Average: ");
		Serial.println(restAvg);
		Serial.print("Flex Average: ");
		Serial.println(flexAvg);

		threshold = restAvg + 0.1 * (flexAvg - restAvg);
		Serial.print("Threshold: ");
		Serial.println(threshold);

		calibrated = true;
		
		return;
	}

	awaitSerial();
	clearSerialBuffer();

	float volt = Serial.readStringUntil('\n').toFloat();
	volt = abs(volt);

	
	if (volt < threshold && count_relax >= 20 && current_time - start_time >= delta) {
		start_time = millis();

		Serial.print("Relaxing: ");
		Serial.print(threshold);
		Serial.print(" > ");
		Serial.println(volt);

		angle += 10;

		angle = min(angle, 180);
		
		servo.write(angle);
	} else if (current_time - start_time >= delta) {
		start_time = millis();
		if (volt < threshold) {
			count_relax += 1;
		} else {
			count_relax = 0;
		}
		Serial.print("Flexing ");
		Serial.print(threshold);
		Serial.print(" < ");
		Serial.print(volt);
		Serial.print(" ");
		Serial.println(volt < threshold);

		angle -= 10;
		angle = max(angle, 0);

		servo.write(angle);
	}
}
