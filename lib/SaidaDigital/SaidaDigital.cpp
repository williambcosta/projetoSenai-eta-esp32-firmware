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
 * É importante chamar a funcão loop passando millis() como parametro para que o tempo seja atualizado
 *
 */

#include "SaidaDigital.h"

SaidaDigital::SaidaDigital(std::string saida) {
  this->byteSaida = saida[0] - '0';           // Converte o caractere do byte para um índice numérico
  this->bitSaida = saida[2] - '0';            // Converte o caractere do bit para um índice numérico
}

SaidaDigital::SaidaDigital(std::string saida, uint8_t nivelAtuacao) : SaidaDigital(saida) {
  this->nivelAtuacao = nivelAtuacao;
}

SaidaDigital::SaidaDigital(uint8_t byteSaida, uint8_t bitSaida) {
  this->byteSaida = byteSaida;
  this->bitSaida = bitSaida;
}

SaidaDigital::SaidaDigital(uint8_t byteSaida, uint8_t bitSaida, uint8_t nivelAtuacao) : SaidaDigital(byteSaida, bitSaida) {
  this->nivelAtuacao = nivelAtuacao;
}

// Retorna o estado atual do SaidaDigital. TRUE = ligado, FALSE = desligado
bool SaidaDigital::getStatus() {
  return estado;
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

// TODO: Finalizar a implementação das funções desligaApos
// TODO: Na verdade não vou fazer mais. Vai aumentar muito a complexidade, não tenho tempo pra isso agora
/*
// Função responsável por desligar a saída após o tempo determinado em minutos. O valor será limitado a 255 minutos
void SaidaDigital::desligaApos(uint8_t byteSaida, uint8_t bitSaida, uint8_t minutos) {
  liga(byteSaida, bitSaida);
}

// Função responsável por desligar a saída após o tempo determinado em segundos. O valor será limitado a 15300 segundos
void SaidaDigital::desligaApos(uint8_t byteSaida, uint8_t bitSaida, uint16_t segundos) {
}
*/
