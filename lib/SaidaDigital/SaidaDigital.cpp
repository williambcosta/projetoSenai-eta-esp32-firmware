/**
 * Classe que representa um saida digital.
 *
 * As saídas digitais estarão ligadas ao CI74HC595, que é um registrador de deslocamento de 8 bits.
 *
 * Caso mais de um registrador seja utilizado, o byteSaida irá representar o registrador e o bitSaida
 * irá representar a posição dentro do registrador.
 *
 * Utiliza o Singleton ShiftRegister para controlar o registrador de deslocamento.
 *
 */

#include "SaidaDigital.h"

SaidaDigital::SaidaDigital(std::string saida) {
  this->byteSaida = saida[0] - '0';  // Converte o caractere do byte para um índice numérico
  this->bitSaida = saida[2] - '0';   // Converte o caractere do bit para um índice numérico
}

SaidaDigital::SaidaDigital(std::string saida, uint8_t nivelAtuacao) : SaidaDigital(saida) {
  setNivelAtuacao(nivelAtuacao);
}

SaidaDigital::SaidaDigital(uint8_t byteSaida, uint8_t bitSaida) {
  this->byteSaida = byteSaida;
  this->bitSaida = bitSaida;
}

SaidaDigital::SaidaDigital(uint8_t byteSaida, uint8_t bitSaida, uint8_t nivelAtuacao) : SaidaDigital(byteSaida, bitSaida) {
  setNivelAtuacao(nivelAtuacao);
}

// Retorna o estado atual do SaidaDigital. TRUE = ligado, FALSE = desligado
bool SaidaDigital::getStatus() {
  return ShiftRegister::Instance().isAtuado(this->byteSaida, this->bitSaida);
}

// Função responsável por verificar se a saída está atuada (ligada) levando em consideração o nível lógico de atuação definido.
bool SaidaDigital::isAtuado() {
  if(ShiftRegister::Instance().isAtuado(this->byteSaida, this->bitSaida) && nivelAtuacao == HIGH) {
    return true;
  } else if(!ShiftRegister::Instance().isAtuado(this->byteSaida, this->bitSaida) && nivelAtuacao == LOW) {
    return true;
  }

  return false;
}

// Função responsável por ligar o SaidaDigital
void SaidaDigital::liga() {
  if (nivelAtuacao == HIGH) {
    ShiftRegister::Instance().setSaida(this->byteSaida, this->bitSaida, HIGH);
  } else {
    ShiftRegister::Instance().setSaida(this->byteSaida, this->bitSaida, LOW);
  }
}

// Função responsável por desligar o SaidaDigital
void SaidaDigital::desliga() {
  if (nivelAtuacao == HIGH) {
    ShiftRegister::Instance().setSaida(this->byteSaida, this->bitSaida, LOW);
  } else {
    ShiftRegister::Instance().setSaida(this->byteSaida, this->bitSaida, HIGH);
  }
}

// Função responsável por definir o nível lógico usado para ativar a saída. Se deve atuar em HIGH ou LOW
void SaidaDigital::setNivelAtuacao(uint8_t nivelAtuacao) {
  this->nivelAtuacao = nivelAtuacao;
  if (nivelAtuacao == HIGH) {
    ShiftRegister::Instance().setSaida(this->byteSaida, this->bitSaida, LOW);
  } else {
    ShiftRegister::Instance().setSaida(this->byteSaida, this->bitSaida, HIGH);
  }
}

