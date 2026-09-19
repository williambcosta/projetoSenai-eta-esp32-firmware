#ifndef SAIDA_DIGITAL_H
#define SAIDA_DIGITAL_H

#include <Arduino.h>

#include "ShiftRegister.h"

class SaidaDigital {
 private:
  bool estado = false;  // Armazena o estado da saida. TRUE = ligado, FALSE = desligado

  long tempo = 0;  // Armazena o tempo em que a saida foi ligada◘

  /*
   * Representação do registrador de deslocamento que controla a saida. O byte irá representar o registrador, caso mais de
   * um registrador seja utilizado e o bit irá representar a posição dentro do registrador.
   */
  uint8_t byteSaida = 0;  // Representa o registrador
  uint8_t bitSaida = 0;   // Representa a saída do registrador

  uint8_t nivelAtuacao = HIGH;  // Indica o nível lógico usado para ativar a saída. Se deve atuar em HIGH ou LOW

  ShiftRegister& shiftRegister = ShiftRegister::Instance();  // Instancia do Singleton ShiftRegister para controlar o registrador de deslocamento

 public:
  SaidaDigital(uint8_t byteSaida, uint8_t bitSaida);
  SaidaDigital(uint8_t byteSaida, uint8_t bitSaida, uint8_t nivelAtuacao);

  bool getStatus();  // Retorna o estado atual do SaidaDigital. TRUE = ligado, FALSE = desligado

  void liga(uint8_t byteSaida, uint8_t bitSaida);                            // Função responsável por ligar o SaidaDigital
  void desliga(uint8_t byteSaida, uint8_t bitSaida);                         // Função responsável por desligar o SaidaDigital
  void desligaApos(uint8_t byteSaida, uint8_t bitSaida, uint8_t minutos);    // Função responsável por desligar a saída após o tempo determinado em minutos. O valor será limitado a 255 minutos
  void desligaApos(uint8_t byteSaida, uint8_t bitSaida, uint16_t segundos);  // Função responsável por desligar a saída após o tempo determinado em segundos. O valor será limitado a 15300 segundos
  void loop(unsigned long currentTime);                                      // Função que deve ser chamada no loop principal para verificar se a saída deve ser desligada após o tempo determinado
  void begin();                                                              // Funcão responsável por configurar o pino de saida do SaidaDigital
};

#endif