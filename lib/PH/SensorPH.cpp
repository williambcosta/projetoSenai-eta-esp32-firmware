/*
 * Classe responsável por ler o valor enviado pelo sensor de PH e interpretar o valor para determinar o PH do fluido.
 *
 * É importante a calibração do sensor antes da utilização. Para isso é necessário conectar o pino central do conector
 * BNC e sua parte externa. Esse procedimento força a saída do módulo a um valor fixo, que deve estar próximo de 2,5V.
 * Esse valor representa PH neutro (7). Com a conexão feita, basta ajustar o trimpot do módulo do sensor até que se consiga
 * um valor próximo de 2,5.
 *
 * A utilização de soluções tampão é recomendada para uma calibração mais precisa. Lendo o valor na entrada analógica do microcontrolador, para
 * solução tampão ácida (PH 4) e solução tampão alcalina (PH 10), assim como solução neutra. Esses valores devem ser referenciados na
 * criação do objeto da classe SensorPH, para que o cálculo do PH seja mais preciso.
 *
 * Outro ponto importante é ajustar a tensão para que o valor máximo enviado pelo sensor não seja superior a tensão máxima
 * esperada no pino do microcontrolador. Para isso é necessário um divisor de tensão, que pode ser feito com dois resistores.
 * A tensão máxima esperada é de 3,3V, que representa PH 0.
 *
 * Um valor de offset é calculado separadamente para os valores ácidos e alcalinos, pois o sensor E201-C possui uma curva de resposta não linear,
 * o que pode causar desvio significativo ao ler valores extremos de PH. O cálculo do offset é feito com base na diferença entre a leitura média
 * do sensor e a leitura de referência para PH neutro (7), multiplicada pelo fator de conversão, que é determinado pela diferença entre os valores
 * de referência para soluções tampão ácida e alcalina.
 *
 */

#include "SensorPH.h"

SensorPH::SensorPH(uint8_t pinoPH) {
  this->pinoPH = pinoPH;
}

SensorPH::SensorPH(uint8_t pinoPH, float valorPhAcido, float adcPhAcido, float adcPhNeutro, float valorPhAlcalino, float adcPhAlcalino) : SensorPH(pinoPH) {
  this->valorPhAcido = valorPhAcido;
  this->adcPhAcido = adcPhAcido;

  this->adcPhNeutro = adcPhNeutro;

  this->valorPhAlcalino = valorPhAlcalino;
  this->adcPhAlcalino = adcPhAlcalino;
}

// Calcula o valor de ph tendo como base o valor retornado pelo sensor
void SensorPH::calculaPH() {
  float adcRecebido = 0.0f;  // Varrável auxiliar que armazena o valor rebebido no pino do esp32
  float somatoria = 0.0f;    // Variável auxiliar que armazena a somatória dos valores para posteriormente calcular a média
  float offset = 0.0f;       // Variável auxiliar que é utilizado para calcular o valor de PH a partir do valor lido

  // Realiza várias leituras do sensor, somando os resultados, para estimar a média.
  // Isso ajuda a reduzir o ruído e obter uma leitura mais precisa.
  for (uint16_t i = 0; i < amostras; i++) {
    adcRecebido = (analogRead(pinoPH));  // Valor lido no pino do ESP32

    somatoria += adcRecebido;  // Soma o valor atual ao total
    delay(2);                  // Pequena pausa entre as leituras para estabilizar o sensor.
  }

  float leituraMedia = somatoria / amostras;  // Realiza a média entre os valores lidos

  // Calcula o offset com base na leitura média e nos valores de referência para soluções tampão ácida e alcalina.
  if (leituraMedia < adcPhNeutro) {  // Caso a solução seja alcalina
    offset = (valorPhAlcalino - 7.0f) / (adcPhAlcalino - adcPhNeutro);
  } else {  // Caso ácida
    offset = (7.0f - valorPhAcido) / (adcPhNeutro - adcPhAcido);
  }

  ph = 7.0f + (offset * (leituraMedia - adcPhNeutro));  // Atualiza o valor de ph
}

// Retorna o valor de PH calculado
float SensorPH::getPH() {
  calculaPH();

  return ph;
}
