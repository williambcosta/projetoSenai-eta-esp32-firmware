#ifndef PH_H
#define PH_H

#include <Arduino.h>

class PH {
   private:
    uint8_t pinoPH = 16;      // Pino do sensor
    uint16_t amostras = 100;  // Variável auxiliar para leitura do sensor. Indica quantas vezes ele será lido

    float ph = 7.0f;              // O valor de ph calculado
    float tensaoPhNeutro = 2.5f;  // Tensão obtida após a calibração do sensor. É utilizado para determinar o valor médio da escala de PH
    float fatorConversao = 3.5f;  // Regra matemática que transforma a eletricidade medida no sensor em um valor de pH.

    /*
     * O OFFSET é calculado para garantir que a tensão de calibração corresponda a pH 7.
     * A fórmula usada é: offset = pH_neutro - (tensaoCalibrada * fatorConversao)
     * O fator de conversão padrão do sensor é 3.5, podendo variar
     */
    float offset;

    void calculaPH();  // Calcula o valor de ph tendo como base o valor retornado pelo sensor

   public:
    PH(uint8_t pinoPH, float fatorConversao, float tensaoPhNeutro);

    float getPH();  // Retorna o valor de PH calculado

    void setFatorConversao(float fatorConversao);  // Determina o fator de conversão
    void setTensaoPhNeutro(float tensaoPhNeutro);  // Determina a tensão de referencia para PH 7 após a calibração
};

#endif