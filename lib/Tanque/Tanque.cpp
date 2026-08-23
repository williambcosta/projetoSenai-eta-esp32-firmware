/*
 * Classe responsável por gerenciar os sensores de temperatura, turbidez e pH do tanque de água, bem como os sensores de nível alto e baixo.
 */

#include "Tanque.h"

Tanque::Tanque(uint8_t pinoTemp, uint8_t pinoPH, uint8_t pinoTurbidez, uint8_t pinoNivelAlto, uint8_t pinoNivelBaixo) {
    // Inicializa os membros da classe
    this->pinoNivelAlto = pinoNivelAlto;
    this->pinoNivelBaixo = pinoNivelBaixo;
    temperatura = SensorTemperatura(pinoTemp);
    turbidez = SensorTurbidez(pinoTurbidez);

    // Configura os pinos de nível como pull-down. Caso acionados o nível lógico será LOW.
    pinMode(this->pinoNivelAlto, INPUT_PULLUP);
    pinMode(this->pinoNivelBaixo, INPUT_PULLUP);
}

// Função que retornan a temperatura da água do tanque em C°
float Tanque::getTemperatura() {
    return temperatura.getTemperatura();
}

// Função que retorna a turbidez da água do tanque em NTU (Nephelometric Turbidity Units)
float Tanque::getTurbidez() {
    return turbidez.getTurbidez();
}

// Função que retorna o ph da água do tanque
// TODO: Implementar a leitura do pH do tanque, utilizando o pino de pH fornecido no construtor da classe Tanque.
float Tanque::getPH() {
    return ph;
}

// Função que indica se o nível da água do tanque está alto
bool Tanque::isNivelAlto() {
    return digitalRead(this->pinoNivelAlto) == LOW && digitalRead(this->pinoNivelBaixo) == LOW;
}

// Função que indica se o nível da água do tanque está baixo
bool Tanque::isNivelBaixo() {
    return digitalRead(this->pinoNivelAlto) == HIGH && digitalRead(this->pinoNivelBaixo) == LOW;
}

// Função que indica se o tanque está vazio
bool Tanque::isVazio() {
    return digitalRead(this->pinoNivelAlto) == HIGH && digitalRead(this->pinoNivelBaixo) == HIGH;
}

/* Função que indica se exite falha nos sensores. Garante a falha apenas em uma situação, quando o sensor de nível alto
 * estiver acionado e o baixo não, mas pode ser útil para identificar problemas.
 */
bool Tanque::isFalhaSensores() {
    if (digitalRead(this->pinoNivelAlto) == LOW && digitalRead(this->pinoNivelBaixo) == HIGH) {
        return true;
    }

    return false;
};