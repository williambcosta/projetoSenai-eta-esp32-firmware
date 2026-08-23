/*
 * Classe responsável por ler e calcular a turbidez da água em NTU (Nephelometric Turbidity Units) a partir de um sensor de turbidez conectado ao ESP32.
 * 
 * O valor NTU é calculado a partir da tensão obtida segindo a formula: NTU = -1120.4 * (tensão^2) + 5742.3 * tensão - 4352.9,
 * onde a tensão é a média das leituras do sensor. Essa média auxilia na eliminação de ruidos, garantindo uma medição mais precisa e confiável.
 *
 */

#include "SensorTurbidez.h"

SensorTurbidez::SensorTurbidez(uint8_t pino) {
    SensorTurbidez(); 
    pinoTurbidez = pino;
}

SensorTurbidez::SensorTurbidez(uint8_t pino, uint8_t amostras) {
    SensorTurbidez(pino);
    this->amostras = amostras;
}

// Função que calcula a turbidez em NTU a partir da leitura do sensor.
void SensorTurbidez::calculaNTU() {
    float tensaoRecebida = 0;  // Varrável auxiliar que armazena a tensão rebebida no pino do esp32
    float tensaoOriginal = 0;  // Variável auxiliar que armazena as tensões originais enviadas pelo sensor. O cálculo NTU depende da tensão original

    // Realiza várias leituras do sensor, somando os resultados, para estimar a média posteriormente.
    // Isso ajuda a reduzir o ruído e obter uma leitura mais precisa.
    for (uint8_t i = 0; i < amostras; i++) {
        tensaoRecebida = ((float)analogRead(pinoTurbidez) / 4095.0);  // Valor lido no pino do ESP32, convertido em V
        tensaoOriginal += tensaoRecebida * (5.0 / 3.3);               // Converte a tensão recebida pelo esp para a tensão original enviada pelo sensor

        delay(2);  // Pequena pausa entre as leituras para estabilizar o sensor.
    }

    // Realiza a média entre os valores lidos
    float leituraMedia = tensaoOriginal / amostras;

    if (leituraMedia < 2.5) {  // Se a leitura for menor que 2.5
        ntu = 3000;            // fixa o valor de ntu em 3000, indicando que a água esta turva

    } else if (leituraMedia > 4.2) {  // Caso contrário, se a leitura for maior que 4.2
        ntu = 0;                      // fixa o valor de ntu em 0, indicando que a água esta limpa

    } else {  // Senão, calcula o valor de ntu através da fórmula
        ntu = -1120.4 * (leituraMedia * leituraMedia) + 5742.3 * leituraMedia - 4352.9;
    }
}


// Retorna o valor de turbidez em NTU
float SensorTurbidez::getTurbidez() {
    calculaNTU();  // Chama a função que calcula a turbidez em NTU
    return ntu;    // Retorna o valor de turbidez em NTU
}
