#ifndef DHT22_H
#define DHT22_H

#include <stdint.h>

typedef struct dht22Data
{
    float temp;
    float umi;
}dht22Data;


#define MAX_TIMINGS	85
#define WAIT_TIME 1000

int initWiringPi();
void printUsage();
int read_dht_data();
int read_sensor_value(int8_t pin);
void write_sensor_value(int8_t pin, int value);
dht22Data get_dht_data(int8_t pin);

#endif