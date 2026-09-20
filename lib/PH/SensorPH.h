#ifndef SENSOR_PH_H
#define SENSOR_PH_H

#include <Arduino.h>

class SensorPH {
 private:
  uint8_t pinoPH = 16;      // Pino do sensor
  uint16_t amostras = 100;  // Variável auxiliar para leitura do sensor. Indica quantas vezes ele será lido

  float ph = 7.0f;  // O valor de ph calculado

  float valorPhAcido = 4.0f;      // Valor de referência para solução tampão ácida.
  float valorPhAlcalino = 10.0f;  // Valor de referência para solução tampão alcalino.

  float adcPhAcido = 3705.5f;     // Leitura analógica obtida após leitura de solução tampão acida.
  float adcPhNeutro = 3105.0f;    // Leitura analógica obtida após leitura de solução tampão neutra.
  float adcPhAlcalino = 2684.0f;  // Leitura analógica obtida após leitura de solução tampão alcalina.

  void calculaPH();  // Calcula o valor de ph tendo como base o valor retornado pelo sensor

 public:
  SensorPH(uint8_t pinoPh);
  SensorPH(uint8_t pinoPH, float valorPhAcido, float adcPhAcido, float adcPhNeutro, float valorPhAlcalino, float adcPhAlcalino);

  float getPH();  // Retorna o valor de PH calculado
};

#endif