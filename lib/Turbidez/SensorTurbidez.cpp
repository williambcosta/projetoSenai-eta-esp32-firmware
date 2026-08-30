/*
 * Classe responsável por ler e calcular a turbidez da água em NTU (Nephelometric Turbidity Units) a partir de um sensor de turbidez conectado ao ESP32.
 *
 * O valor NTU é calculado a partir da tensão obtida segindo a formula: NTU = -1120.4 * (tensão^2) + 5742.3 * tensão - 4352.9,
 * onde a tensão é a média das leituras do sensor. Essa média auxilia na eliminação de ruidos, garantindo uma medição mais precisa e confiável.
 *
 * É necessário medir os valores máximo e minimo que o sensor envia, pois o sensor pode enviar várias faixas de tensão.
 * Para isso basta medir o valor enviado com o sensor sem obstrução nenhuma (o que indica água limpa), esse valor será
 * armazenado na variável maxTensao.
 *
 */

#include "SensorTurbidez.h"

SensorTurbidez::SensorTurbidez(uint8_t pinoTurbidez, uint16_t amostras, float maxTensao, float minTensao) {
    this->pinoTurbidez = pinoTurbidez;
    this->amostras = amostras;
    this->maxtensao = maxTensao;
    this->minTensao = minTensao;

    fatorConversao = 1000.0f / (maxtensao - minTensao);
}

// Função que calcula a turbidez em NTU a partir da leitura do sensor.
void SensorTurbidez::calculaNTU() {
    float tensaoRecebida = 0.0f;  // Varrável auxiliar que armazena a tensão rebebida no pino do esp32
    float somatoria = 0.0f;       // Variável auxiliar que armazena a somatória de tensões para uma posteriormente calcular a média

    // Realiza várias leituras do sensor, somando os resultados, para estimar a média posteriormente.
    // Isso ajuda a reduzir o ruído e obter uma leitura mais precisa.
    for (uint16_t i = 0; i < amostras; i++) {
        tensaoRecebida = (((float)analogRead(pinoTurbidez) * 3.53f) / 4095.0f);  // Valor lido no pino do ESP32, convertido em V

        somatoria += tensaoRecebida;  // Soma a tensão atual ao total
        delay(2);                     // Pequena pausa entre as leituras para estabilizar o sensor.
    }

    // Realiza a média entre os valores lidos
    float leituraMedia = somatoria / amostras;

    if (leituraMedia < minTensao) {  // Se a leitura for menor que 2.5
        ntu = 2000.0f;               // fixa o valor de ntu em 3000, indicando que a água esta turva

    } else if (leituraMedia >= maxtensao) {  // Caso contrário, se a leitura for maior que 4.2
        ntu = 0.0f;                          // fixa o valor de ntu em 0, indicando que a água esta limpa

    } else {  // Senão, calcula o valor de ntu através da fórmula
        ntu = 1000 - (leituraMedia * fatorConversao);
    }
}

// Retorna o valor de turbidez em NTU
float SensorTurbidez::getTurbidez() {
    calculaNTU();  // Chama a função que calcula a turbidez em NTU

    return ntu;  // Retorna o valor de turbidez em NTU
}