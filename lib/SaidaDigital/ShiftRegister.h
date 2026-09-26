/**
 * Declaração da classe ShiftRegister para controle de registradores de deslocamento. Ela será responsável
 * por gerenciar a comunicação com os registradores de deslocamento 74HC595, permitindo o controle de múltiplas
 * saídas digitais a partir de um número limitado de pinos do microcontrolador.
 * 
 * Ligação dos terminais do 74HC595:
 * - VCC (Pin 16): Conectar ao pino de 5V do microcontrolador.
 * - GND (Pin 8): Conectar ao pino de terra do microcontrolador.
 * - DS (Pin 14): Conectar ao pino de dados (dataPin) do microcontrolador.
 * - SHCP (Pin 11): Conectar ao pino de clock (clockPin) do microcontrolador.
 * - STCP (Pin 12): Conectar ao pino de latch (latchPin) do microcontrolador.
 * - OE (Pin 13): Conectar ao pino de terra do microcontrolador para habilitar a saída.
 * - MR (Pin 10): Conectar ao pino de 5V do microcontrolador para desabilitar o reset.
 * - Q0 a Q7 (Pins 15, 1, 2, 3, 4, 5, 6, 7): Pinos de saída do registrador de deslocamento.
 * - Q7' (Pin 9): Pino de saída serial para encadear múltiplos registradores de deslocamento.
 */
#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#include <Arduino.h>

class ShiftRegister {
 private:
  uint8_t clockPin = 25;  // Pino de clock do registrador de deslocamento
  uint8_t latchPin = 26;  // Pino de latch do registrador de deslocamento
  uint8_t dataPin = 27;   // Pino de dados do registrador de deslocamento

  bool autoAtualizacao = true;  // Indica se as saídas devem ser atualizadas automaticamente após cada alteração

  // Armazena o estado atual dos pinos de saída dos registradores
  byte dados[2] = {
      0b00000000,  // Registrador 1
      0b00000000   // Registrador 2
  };

  // Construtor padrão, privado para impedir a criação de instâncias.
  ShiftRegister() = default;

  // Remove construtores de cópia e operadores de atribuição
  ShiftRegister(const ShiftRegister&) = delete;
  ShiftRegister& operator=(const ShiftRegister&) = delete;
  ShiftRegister(ShiftRegister&&) = delete;

 public:
  static ShiftRegister& Instance();  // Retorna a instância única da classe ShiftRegister

  void begin(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin);  // Inicializa os pinos do registrador de deslocamento
  void setAutoAtualizacao(bool autoAtualizacao);                    // Define se as saídas devem ser atualizadas automaticamente após cada alteração
  void setSaida(uint8_t byteIndex, uint8_t bitIndex, bool valor);   // Define o estado de uma saída específica no registrador de deslocamento
  void setSaida(std::string saida, bool valor);                     // Define o estado de uma saída específica no registrador de deslocamento usando a notação "x.y" sendo x o byte e y o bit
  void atualizarSaidas();                                           // Atualiza os pinos de saída do registrador de deslocamento com os valores armazenados no array de dados
  bool isAtuado(uint8_t byteIndex, uint8_t bitIndex);               // Verifica se uma saída específica está atuada (ligada)
  bool isAtuado(std::string saida);                                 // Verifica se uma saída específica está atuada (ligada) usando a notação "x.y" sendo x o byte e y o bit
};

#endif