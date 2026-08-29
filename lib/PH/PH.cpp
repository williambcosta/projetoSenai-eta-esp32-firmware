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
 *
 */

#include "PH.h"

PH::PH() {
    pinoPH = 0;
    tensaoCalibracao = 2.5f;
}

PH::PH(uint8_t pinoPH) {
    this->pinoPH = pinoPH;
}

// Calcula o valor de ph tendo como base o valor retornado pelo sensor
void PH::calculaPH() {
    float tensaoRecebida = ((float)analogRead(pinoPH) / 4095.0);  // Varrável auxiliar que armazena a tensão rebebida no pino do esp32
    float tensaoOriginal = tensaoRecebida * (5.0 / 3.3);          // Converte a tensão recebida pelo esp para a tensão original enviada pelo sensor
    ph = (tensaoOriginal * fatorConversao) + OFFSET;              // Converte a tensão para pH
}

// Retorna o valor de PH calculado
float PH::getPH() {
    // Log de eventos
    /*
        Config.log("PH medido no sensor conectado ao pino ");
        Config.log(pinoPH);
        Config.log(" : ");
        Config.logln(ph);
    */

    return ph;
}

// Determina a tensão de referencia para PH 7 após a calibração
void PH::setTensaoCalibracao(float tensaoCalibracao) {
    this->tensaoCalibracao = tensaoCalibracao;
}
