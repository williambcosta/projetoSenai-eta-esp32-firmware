#ifndef ATUADOR_H
#define ATUADOR_H

#include <Arduino.h>

// TODO: Adicionar temporização para permanecer ligado por x tempo e desligar após x tempo
class Atuador {
 private:
  bool estado = false;  // Armazena o estado do atuador. TRUE = ligado, FALSE = desligado

  uint8_t pino = 4;             // Pino responsável pelo sinal do atuador
  uint8_t nivelAtuacao = HIGH;  // Indica o nível lógico usado para indicar quando o atuador deve ligar. Se deve atuar em HIGH ou LOW

 public:
  Atuador(uint8_t pinoAtuador);
  Atuador(uint8_t pinoAtuador, uint8_t nivelAtuacao);

  bool getStatus();  // Retorna o estado atual do atuador. TRUE = ligado, FALSE = desligado
  void liga();       // Função responsável por ligar o atuador
  void desliga();    // Função responsável por desligar o atuador
  void begin();      // Funcão responsável por configurar o pino de saida do atuador
};

#endif