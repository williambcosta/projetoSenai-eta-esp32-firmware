#ifndef ATUADOR_H
#define ATUADOR_H

#include <Arduino.h>

class Atuador {
   private:
    bool estado = false;  // Armazena o estado do atuador. TRUE = ligado, FALSE = desligado

    uint8_t pino = 4;             // Pino responsável pelo sinal do atuador
    uint8_t nivelAtuacao = HIGH;  // Indica o nível lógico usado para indicar quando o atuador deve ligar. Se deve atuar em HIGH ou LOW

   public:
    Atuador(uint8_t pinoAtuador);
    Atuador(uint8_t pinoAtuador, uint8_t nivelAtuacao);

    void liga();       // Função responsável por ligar o atuador
    void desliga();    // Função responsável por desligar o atuador
    bool getStatus();  // Retorna o estado atual do atuador. TRUE = ligado, FALSE = desligado
};

#endif