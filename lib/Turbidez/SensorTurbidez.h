#ifndef SensorTurbidez_h
#define SensorTurbidez_h

#include <Arduino.h>

class SensorTurbidez {
   private:
    uint8_t pinoTurbidez;  // Pino do sensor de turbidez
    uint8_t amostras;      // Variável auxiliar para leitura do sensor.

    float ntu;  // Variável que armazena o valor de turbidez em NTU.

    void calculaNTU();  // Função que calcula a turbidez em NTU a partir da leitura do sensor.

   public:
    SensorTurbidez() {
        pinoTurbidez = 0;  // Valor padrão do sensor de turbidez
        amostras = 800;    // Valor padrão de amostras
        ntu = 0;           // Valor padrão de turbidez
    };
    SensorTurbidez(uint8_t pino);
    SensorTurbidez(uint8_t pino, uint8_t amostras);

    float getTurbidez();  // Retorna o valor de turbidez em NTU
};

#endif