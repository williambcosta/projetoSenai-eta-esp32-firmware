/*
 * Classe responsável por ler e calcular a turbidez da água em NTU (Nephelometric Turbidity Units) a partir de um sensor de turbidez conectado ao ESP32.
 *
 * O valor NTU é calculado a partir da tensão obtida segindo a formula: NTU = -1120.4 * (tensão^2) + 5742.3 * tensão - 4352.9,
 * onde a tensão é a média das leituras do sensor. Essa média auxilia na eliminação de ruidos, garantindo uma medição mais precisa e confiável.
 *
 */

#include "SensorTurbidez.h"

SensorTurbidez::SensorTurbidez() {
    pinoTurbidez = 0;  // Pino do sensor de turbidez
    amostras = 800;    // Variável auxiliar para leitura do sensor.
    ntu = 0.0f;        // Variável que armazena o valor de turbidez em NTU.
}

SensorTurbidez::SensorTurbidez(uint8_t pinoTurbidez) {
    Serial.println("Sensor pino");
    this->pinoTurbidez = pinoTurbidez;
}

SensorTurbidez::SensorTurbidez(uint8_t pinoTurbidez, uint16_t amostras) {
    Serial.println("sensor pino amostras");
    this->pinoTurbidez = pinoTurbidez;
    this->amostras = amostras;
}

// Função que calcula a turbidez em NTU a partir da leitura do sensor.
void SensorTurbidez::calculaNTU() {
    float tensaoRecebida = 0.0f;  // Varrável auxiliar que armazena a tensão rebebida no pino do esp32
    float tensaoOriginal = 0.0f;  // Variável auxiliar que armazena as tensão original enviada pelo sensor. O cálculo NTU depende da tensão original
    float somatoria = 0.0f;       // Variável auxiliar que armazena a somatória de tensões para uma posteriormente calcular a média

    // Realiza várias leituras do sensor, somando os resultados, para estimar a média posteriormente.
    // Isso ajuda a reduzir o ruído e obter uma leitura mais precisa.
    for (uint16_t i = 0; i < amostras; i++) {
        tensaoRecebida = ((float)analogRead(pinoTurbidez) / 4095.0f);  // Valor lido no pino do ESP32, convertido em V

        tensaoOriginal = tensaoRecebida * (5.0f / 3.3f);  // Converte a tensão recebida pelo esp para a tensão original enviada pelo sensor
        somatoria += tensaoOriginal;                      // Soma a tensão atual ao total
        delay(2);                                         // Pequena pausa entre as leituras para estabilizar o sensor.
    }

    // Realiza a média entre os valores lidos
    float leituraMedia = somatoria / amostras;
    Serial.println(leituraMedia);

    if (leituraMedia < 0.7f) {  // Se a leitura for menor que 2.5
        ntu = 1000.0f;          // fixa o valor de ntu em 3000, indicando que a água esta turva

    } else if (leituraMedia > 1.4f) {  // Caso contrário, se a leitura for maior que 4.2
        ntu = 0.0f;                    // fixa o valor de ntu em 0, indicando que a água esta limpa

    } else {  // Senão, calcula o valor de ntu através da fórmula
        ntu = -1120.4f * (leituraMedia * leituraMedia) + 5742.3f * leituraMedia - 4352.9f;
    }
}

// Retorna o valor de turbidez em NTU
float SensorTurbidez::getTurbidez() {
    calculaNTU();  // Chama a função que calcula a turbidez em NTU

    // Log de eventos
    /*
        Config.log("Turbidez medida no sensor conectado no pino ");
        Config.log(pinoTurbidez);
        Config.log(" : ");
        Config.logln(ntu);
    */

    return ntu;  // Retorna o valor de turbidez em NTU
}