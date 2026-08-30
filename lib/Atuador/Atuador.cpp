/*
 * Classe que representa um atuador. Um atuador é um dispositivo que converte energia 
 * (elétrica, pneumática ou hidráulica) em movimento, ou ação, físico ou força mecânica.
 */

#include "Atuador.h"

Atuador::Atuador(uint8_t pinoAtuador) {
    this->pino = pinoAtuador;
    pinMode(pino, OUTPUT);
}

Atuador::Atuador(uint8_t pinoAtuador, uint8_t nivelAtuacao) : pino(pinoAtuador) {
    this->nivelAtuacao = nivelAtuacao;
}

// Função responsável por ligar o atuador
void Atuador::liga() {
    if (nivelAtuacao == HIGH) {
        digitalWrite(pino, HIGH);
    } else {
        digitalWrite(pino, LOW);
    }
}

// Função responsável por desligar o atuador
void Atuador::desliga() {
    if (nivelAtuacao == HIGH) {
        digitalWrite(pino, LOW);
    } else {
        digitalWrite(pino, HIGH);
    }
}

// Retorna o estado atual do atuador. TRUE = ligado, FALSE = desligado
bool Atuador::getStatus() {
    return estado;
}
