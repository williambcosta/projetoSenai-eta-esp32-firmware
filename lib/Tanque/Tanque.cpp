/*
 * Classe responsável por gerenciar os sensores de temperatura, turbidez e pH do tanque de água, bem como os sensores de nível alto e baixo.
 */

#include "Tanque.h"

Tanque::Tanque(SensorTemperatura& sensorTemp, uint8_t pinoPH, SensorTurbidez& sensorTurbidez, uint8_t pinoNivelAlto, uint8_t pinoNivelBaixo, const std::vector<Atuador>& atuadores) {
    // Inicializa os membros da classe
    this->pinoNivelAlto = pinoNivelAlto;
    this->pinoNivelBaixo = pinoNivelBaixo;
    this->atuadores = atuadores;

    temperatura = sensorTemp;
    turbidez = sensorTurbidez;

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
float Tanque::getPH() {
    return ph.getPH();
}

// Função que indica se o nível da água do tanque está alto
bool Tanque::isNivelAlto() {
    return digitalRead(this->pinoNivelAlto) == LOW && digitalRead(this->pinoNivelBaixo) == LOW;
}

// Função que indica se o tanque está vazio
bool Tanque::isNivelBaixo() {
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
}

/*
 * Função responsável por indicar o estado do atuador indicado.
 *
 * Retorna FALSE mesmo quando não existir o indice solicitado. Deve ser usada em conjunto com hasAtuador(indiceAtuador)
 */
bool Tanque::isAtuadorLigado(uint8_t indiceAtuador) {
    if (hasAtuador(indiceAtuador)) {  // Verifica se o indice é maior ou igual ao tamanho do vetor
        return false;                 // caso seja retorna null pointer
    }

    return atuadores[indiceAtuador].getStatus();  // Caso contrário retorna a referencia do atuador indicado
}

// Verifica se existe um atuador no indice indicado. Retorna True caso exista
bool Tanque::hasAtuador(uint8_t indiceAtuador) {
    return (indiceAtuador >= 0) && (indiceAtuador < atuadores.size());
}

// Liga o atuador indicado.
void Tanque::ligaAtuador(uint8_t indiceAtuador) {
    atuadores[indiceAtuador].liga();
}

// Desliga o atuador indicado.
void Tanque::desligaAtuador(uint8_t indiceAtuador) {
    atuadores[indiceAtuador].desliga();
};