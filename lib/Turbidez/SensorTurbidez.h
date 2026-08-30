#ifndef SensorTurbidez_h
#define SensorTurbidez_h

#include <Arduino.h>

class SensorTurbidez {
   private:
    uint8_t pinoTurbidez = 17;  // Pino do sensor de turbidez

    uint16_t amostras = 100;  // Variável auxiliar para leitura do sensor. Indica quantas vezes ele será lido

    float ntu = 0.0f;        // Variável que armazena o valor de turbidez em NTU.
    float maxtensao = 1.9f;  // Valor máximo medido que o sensor KIE-TS300B envia. Indica água cristalina.
    float minTensao = 0.2f;  // Valor mínimo medido que o sensor KIE-TS300B envia. Indica água muito turva.

    /*
     * O fatorConversao é calculado para garantir que o calculo do ntu seja preciso.
     */
    float fatorConversao;

    void calculaNTU();  // Função que calcula a turbidez em NTU a partir da leitura do sensor.

   public:
    SensorTurbidez(uint8_t pinoTurbidez, uint16_t amostras, float maxTensao, float minTensao);

    float getTurbidez();  // Retorna o valor de turbidez em NTU
};

#endif