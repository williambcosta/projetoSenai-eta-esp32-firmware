#ifndef SensorTurbidez_h
#define SensorTurbidez_h

#include <Arduino.h>

class SensorTurbidez {
   private:
    uint8_t pinoTurbidez;  // Pino do sensor de turbidez

    uint16_t amostras;  // Variável auxiliar para leitura do sensor.

    float ntu;  // Variável que armazena o valor de turbidez em NTU.

    void calculaNTU();  // Função que calcula a turbidez em NTU a partir da leitura do sensor.

   public:
    SensorTurbidez();
    SensorTurbidez(uint8_t pinoTurbidez);
    SensorTurbidez(uint8_t pinoTurbidez, uint16_t amostras);

    float getTurbidez();  // Retorna o valor de turbidez em NTU

};

#endif