/**
 * Classe que representa um atuador.
 * Atuador é um dispositivo que converte energia (elétrica, pneumática ou hidráulica) em movimento, ou ação,
 * físico ou força mecânica.
 *
 * Nesse projeto irá representar bombas de transferencia, bombas dosadoras, solenoides e agitadores.
 */

#include "Atuador.h"

Atuador::Atuador(uint8_t pinoAtuador) {
  this->pino = pinoAtuador;
}

Atuador::Atuador(uint8_t pinoAtuador, uint8_t nivelAtuacao) : Atuador(pinoAtuador) {
  this->nivelAtuacao = nivelAtuacao;
}

// Retorna o estado atual do atuador. TRUE = ligado, FALSE = desligado
bool Atuador::getStatus() {
  return estado;
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

// Funcão responsável por configurar o pino de saida do atuador
void Atuador::begin() {
  pinMode(pino, OUTPUT);
}
