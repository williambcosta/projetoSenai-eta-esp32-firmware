/**
 * Implementação da classe ShiftRegister para controle de registradores de deslocamento.
 */

#include "ShiftRegister.h"

// Retorna a instância única da classe ShiftRegister
ShiftRegister& ShiftRegister::Instance() {
  static ShiftRegister instance;  // Instância única da classe ShiftRegister
  return instance;
}

// Inicializa os pinos do registrador de deslocamento
void ShiftRegister::begin(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin) {
  this->dataPin = dataPin;
  this->clockPin = clockPin;
  this->latchPin = latchPin;

  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
}

// Define se as saídas devem ser atualizadas automaticamente após cada alteração
void ShiftRegister::setAutoAtualizacao(bool autoAtualizacao) {
  this->autoAtualizacao = autoAtualizacao;
}

/**
 * Define o estado de uma saída específica no registrador de deslocamento
 *
 * Caso a atualização automática esteja habilitada, as saídas serão atualizadas imediatamente após a alteração do estado da saída.
 * Caso contrário, será necessário chamar a função atualizarSaidas() para que as alterações sejam refletidas.
 *
 */
void ShiftRegister::setSaida(uint8_t byteIndex, uint8_t bitIndex, bool valor) {
  bitWrite(dados[byteIndex], bitIndex, valor);  // Atualiza o bit específico no array de dados

  // Se a atualização automática estiver habilitada, atualiza os pinos de saída do registrador de deslocamento
  if (autoAtualizacao) {
    atualizarSaidas();
  }
}

// Define o estado de uma saída específica no registrador de deslocamento usando a notação "x.y" sendo x o byte e y o bit
void ShiftRegister::setSaida(std::string saida, bool valor) {
  uint8_t byteIndex = saida[0] - '0';           // Converte o caractere do byte para um índice numérico
  uint8_t bitIndex = saida[2] - '0';            // Converte o caractere do bit para um índice numérico
  bitWrite(dados[byteIndex], bitIndex, valor);  // Atualiza o bit específico no array de dados

  setSaida(byteIndex, bitIndex, valor);  // Chama a função setSaida para atualizar o estado da saída
}

// Atualiza os pinos de saída do registrador de deslocamento com os valores armazenados no array de dados
void ShiftRegister::atualizarSaidas() {
  digitalWrite(latchPin, LOW);  // Coloca o pino de latch em LOW para iniciar a atualização dos pinos de saída

  int8_t tamanho = sizeof(dados) / sizeof(dados[0]);  // Calcula o tamanho do array de dados

  // Envia os dados para o registrador de deslocamento atualizando os bytes de saída, começando pelo último byte
  for (int8_t i = tamanho - 1; i >= 0; i--) {
    shiftOut(dataPin, clockPin, MSBFIRST, dados[i]);  // Envia o byte atual para o registrador de deslocamento
  }

  digitalWrite(latchPin, HIGH);  // Coloca o pino de latch em HIGH para transferir os dados para as saídas
}

// Verifica se uma saída específica está atuada (ligada)
bool ShiftRegister::isAtuado(uint8_t byteIndex, uint8_t bitIndex) {
  return dados[byteIndex] >> bitIndex & 1;  // Retorna o estado do bit específico no array de dados
}

// Verifica se uma saída específica está atuada (ligada) usando a notação "x.y" sendo x o byte e y o bit
bool ShiftRegister::isAtuado(std::string saida) {
  uint8_t byteIndex = saida[0] - '0';  // Converte o caractere do byte para um índice numérico
  uint8_t bitIndex = saida[2] - '0';   // Converte o caractere do bit para um índice numérico

  return isAtuado(byteIndex, bitIndex);  // Chama a função isAtuado para verificar o estado da saída
}
