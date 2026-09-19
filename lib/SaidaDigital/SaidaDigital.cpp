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
void SaidaDigital::liga(uint8_t byteSaida, uint8_t bitSaida) {
  if (nivelAtuacao == HIGH) {
    shiftRegister.setSaida(byteSaida, bitSaida, HIGH);
  } else {
    shiftRegister.setSaida(byteSaida, bitSaida, LOW);
  }
}

// Função responsável por desligar o SaidaDigital
void SaidaDigital::desliga(uint8_t byteSaida, uint8_t bitSaida) {
  if (nivelAtuacao == HIGH) {
    shiftRegister.setSaida(byteSaida, bitSaida, LOW);
  } else {
    shiftRegister.setSaida(byteSaida, bitSaida, HIGH);
  }
}

// TODO: Finalizar a implementação das funções desligaApos, loop e begin
// Função responsável por desligar a saída após o tempo determinado em minutos. O valor será limitado a 255 minutos
void SaidaDigital::desligaApos(uint8_t byteSaida, uint8_t bitSaida, uint8_t minutos) {
}

// Função responsável por desligar a saída após o tempo determinado em segundos. O valor será limitado a 15300 segundos
void SaidaDigital::desligaApos(uint8_t byteSaida, uint8_t bitSaida, uint16_t segundos) {
}

// Função que deve ser chamada no loop principal para verificar se a saída deve ser desligada após o tempo determinado
void SaidaDigital::loop(unsigned long currentTime) {
}

// Funcão responsável por configurar o pino de saida do SaidaDigital
void SaidaDigital::begin() {
}
