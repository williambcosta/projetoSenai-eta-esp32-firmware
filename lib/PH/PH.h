#ifndef PH_H
#define PH_H

#include <Arduino.h>

class PH {
   private:
    uint8_t pinoPH;  // Pino do sensor

    float ph;                // O valor de ph calculado
    float tensaoCalibracao;  // Tensão obtida após a calibração do sensor. É utilizado para determinar o valor médio da escala de PH
    float fatorConversao;    // Regra matemática que transforma a eletricidade medida no sensor em um valor de pH.

    void calculaPH();  // Calcula o valor de ph tendo como base o valor retornado pelo sensor

    /*
     * O OFFSET é calculado para garantir que a tensão de calibração corresponda a pH 7.
     * A fórmula usada é: OFFSET = pH_neutro - (tensaoCalibrada * fatorConversao)
     * O fator de conversão padrão do sensor é 3.5, podendo variar
     */
    const float OFFSET = 7.0f - (tensaoCalibracao * fatorConversao);

   public:
    PH();
    PH(uint8_t pinoPH);

    float getPH();  // Retorna o valor de PH calculado

    void setTensaoCalibracao(float tensaoCalibracao);  // Determina a tensão de referencia para PH 7 após a calibração
};

#endif