#ifndef SENSOR_TEMPERATURA_H
#define SENSOR_TEMPERATURA_H

/* ----- DEPENDÊNCIAS ----- */
#include <DallasTemperature.h>
#include <OneWire.h>

class SensorTemperatura {
   private:
    OneWire oneWire;
    DallasTemperature sensor;
    DeviceAddress enderecoSensor = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    float temperatura = -999.9;  // Inicia com valor padrão.

   public:
    SensorTemperatura() {
        oneWire = OneWire(32);
        sensor = DallasTemperature(&oneWire);
        sensor.begin();
    };
    SensorTemperatura(uint8_t pinoOneWire);
    float getTemperatura();
};

#endif