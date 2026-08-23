
 /* ----- DEPENDÊNCIAS ----- */
#include <DallasTemperature.h>
#include <OneWire.h>

class SensorTemperatura {
   private:
    OneWire oneWire;
    DallasTemperature sensor;
    DeviceAddress enderecoSensor = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

   public:
    SensorTemperatura(uint8_t pinoOneWire);
    float getTemperatura();
};