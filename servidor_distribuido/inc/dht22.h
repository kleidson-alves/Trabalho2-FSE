#ifndef DHT22_H
#define DHT22_H

#include <stdint.h>

#define MAX_TIMINGS	85
#define WAIT_TIME 2000

int initWiringPi();
void printUsage();
int read_dht_data();
int read_sensor_value(int8_t pin);
void write_sensor_value(int8_t pin, int value);
float get_dht_data(char mode, int8_t pin);

#endif