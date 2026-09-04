/*
 * Classe responsável por ler o valor enviado pelo sensor de PH e interpretar o valor para determinar o PH do fluido.
 *
 * É importante a calibração do sensor antes da utilização. Para isso é necessário conectar o pino central do conector
 * BNC e sua parte externa. Esse procedimento força a saída do módulo a um valor fixo, que deve estar próximo de 2,5V.
 * Esse valor representa PH neutro (7). Com a conexão feita, basta ajustar o trimpot do módulo do sensor até que se consiga
 * um valor próximo de 2,5.
 *
 * Outro ponto importante é ajustar a tensão para que o valor máximo enviado pelo sensor não seja superior a tensão máxima
 * esperada no pino do microcontrolador.
 *
 * Para calcular o fator de conversão é necessário medir a tensão (V1) que o sensor envia em solução neutra (pH7) e a
 * tensão (V2) em solução alcalina (pH10), ou solução ácida(pH4).
 *
 * Por fim calcular o fator de conversão = (pH4 - pH7) / V2 - V1
 */

#include "SensorPH.h"

PH::PH(uint8_t pinoPH, float fatorConversao, float tensaoPhNeutro) {
    this->pinoPH = pinoPH;
    this->tensaoPhNeutro = tensaoPhNeutro;
    this->fatorConversao = fatorConversao;

    offset = 7.0f - (tensaoPhNeutro * fatorConversao);
}

// Calcula o valor de ph tendo como base o valor retornado pelo sensor
void PH::calculaPH() {
    float tensaoRecebida = 0.0f;  // Varrável auxiliar que armazena a tensão rebebida no pino do esp32
    float somatoria = 0.0f;       // Variável auxiliar que armazena a somatória de tensões para uma posteriormente calcular a média

    // Realiza várias leituras do sensor, somando os resultados, para estimar a média posteriormente.
    // Isso ajuda a reduzir o ruído e obter uma leitura mais precisa.
    for (uint16_t i = 0; i < amostras; i++) {
        tensaoRecebida = (((float)analogRead(pinoPH) * 3.3f) / 4095.0f);  // Valor lido no pino do ESP32, convertido em V

        somatoria += tensaoRecebida;  // Soma a tensão atual ao total
        delay(2);                     // Pequena pausa entre as leituras para estabilizar o sensor.
    }

    // Realiza a média entre os valores lidos
    float leituraMedia = somatoria / amostras;

    ph = (leituraMedia * fatorConversao) + offset;  // Converte a tensão para pH
}

// Retorna o valor de PH calculado
float PH::getPH() {
    calculaPH();

    return ph;
}

// Determina a tensão de referencia para PH 7 após a calibração
void PH::setFatorConversao(float fatorConversao) {
    this->fatorConversao = fatorConversao;
}

// Determina a o fator de conversão calculado com a formula fC = (pH4 - pH7) / (VpH4 - VpH7)
void PH::setTensaoPhNeutro(float tensaoPhNeutro) {
    this->fatorConversao = fatorConversao;
}
