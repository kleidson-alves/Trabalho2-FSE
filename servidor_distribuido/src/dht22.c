#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>

#include "dht22.h"


uint8_t dht_pin;

int data[5] = { 0, 0, 0, 0, 0 };
float temp_cels = -1;
float temp_fahr = -1;
float humidity = -1;

int read_dht_data() {
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0;
    uint8_t i;

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    pinMode(dht_pin, OUTPUT);
    digitalWrite(dht_pin, LOW);
    delay(18);
    pinMode(dht_pin, INPUT);

    for (i = 0; i < MAX_TIMINGS; i++) {
        counter = 0;
        while (digitalRead(dht_pin) == laststate) {
            counter++;
            delayMicroseconds(1);
            if (counter == 255) {
                break;
            }
        }
        laststate = digitalRead(dht_pin);

        if (counter == 255)
            break;

        if ((i >= 4) && (i % 2 == 0)) {
            data[j / 8] <<= 1;
            if (counter > 16)
                data[j / 8] |= 1;
            j++;
        }
    }

    if ((j >= 40) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))) {
        float h = (float)((data[0] << 8) + data[1]) / 10;
        if (h > 100) {
            h = data[0];
        }
        float c = (float)(((data[2] & 0x7F) << 8) + data[3]) / 10;
        if (c > 125) {
            c = data[2];
        }
        if (data[2] & 0x80) {
            c = -c;
        }
        temp_cels = c;
        temp_fahr = c * 1.8f + 32;
        humidity = h;
        return 0;
    }
    else {
        temp_cels = temp_fahr = humidity = -1;
        return 1;
    }
}

int read_sensor_value(int8_t pin) {
    return digitalRead(pin);
}

int initWiringPi() {
    if (wiringPiSetupGpio() == -1) {
        printf("Failed to initialize wiringPi\n");
        exit(1);
        return 1;
    }
    return 0;
}

void write_sensor_value(int8_t pin, int value) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, value);
}


dht22Data get_dht_data(int8_t pin) {
    int done = 0;
    dht22Data data;
    dht_pin = pin;

    while (!done) {
        done = !read_dht_data();
        delay(WAIT_TIME);
    }

    data.temp = temp_cels;
    data.umi = humidity;
    return data;

}