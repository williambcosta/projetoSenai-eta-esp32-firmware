#ifndef SensorTurbidez_h
#define SensorTurbidez_h

#include <Arduino.h>

class SensorTurbidez {
   private:
    uint8_t pinoTurbidez = 0;  // Pino do sensor de turbidez

    uint16_t amostras = 800;      // Variável auxiliar para leitura do sensor.

    bool isLogAtivo = false;  // Indica se o log de eventos está ativo. Será utilizado o monitor serial para exibir os eventos

    float ntu = 0.0f;  // Variável que armazena o valor de turbidez em NTU.

    void calculaNTU();  // Função que calcula a turbidez em NTU a partir da leitura do sensor.

   public:
    SensorTurbidez(uint8_t pinoTurbidez);
    SensorTurbidez(uint8_t pinoTurbidez, uint16_t amostras);

    float getTurbidez();  // Retorna o valor de turbidez em NTU

    void setLogEventos(bool logAtivo);  // Indica se o log de eventos deve ser ativado
};

#endif