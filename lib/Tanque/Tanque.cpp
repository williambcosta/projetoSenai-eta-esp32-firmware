/**
 * Classe responsável por gerenciar os sensores de temperatura, turbidez e pH do tanque de água, bem como os sensores de nível alto e baixo.
 *
 * O método begin deve ser chamado para configurar os pinos dos sensores de nível. Isso deve ser feito dentro, ou após, da função setup()
 * Isso garante que nenhuma operação com GPIOs seja executada antes da inicialização completa da placa.
 * 
 * Os pinos dos sensores são configurados como INPUT_PULLUP. Para garantir segurança o sensor deverá inverter a lógica de acionamento
 * caso o nível não seja atingido o sensor deve chavear o negativo enviando nivel lógico LOW no esp. Isso garante que, caso um fio rompa,
 * o esp sempre receberá HIGH indicando que o nível está alto, por exemplo, interrompendo a transferencia de um tanque para outro
 * evitando transbordo.
 * 
 * Como o esp já possue resistor de pull up na maioria das entradas, basta ligar o sensor no gnd e o retorno no pino específico.
 * 
 * GPIOs com resistor de Pull-up:
 *  0, 2, 4, 5, 12 até 23, 25, 26, 27, 32 e 33
 * 
 * GPIOs exclusivos para entrada analógica ou digital não possuem resistor de Pull-up, são eles:
 * 34, 35, 36 (VP) e 39 (VN)
 */

#include "Tanque.h"
Tanque::Tanque(const std::vector<SaidaDigital>& atuadores, uint8_t pinoNivelAlto, uint8_t pinoNivelBaixo)
    : atuadores(atuadores) {
  // Inicializa os membros da classe
  this->pinoNivelAlto = pinoNivelAlto;
  this->pinoNivelBaixo = pinoNivelBaixo;
}

// Função que indica se o nível da água do tanque está alto
bool Tanque::isNivelAlto() {
  return digitalRead(this->pinoNivelAlto) == HIGH && digitalRead(this->pinoNivelBaixo) == HIGH;
}

// Função que indica se o tanque está vazio
bool Tanque::isNivelBaixo() {
  return digitalRead(this->pinoNivelAlto) == LOW && digitalRead(this->pinoNivelBaixo) == LOW;
}

/**
 * Função que indica se exite falha nos sensores. Garante a falha apenas em uma situação, quando o sensor de nível alto
 * estiver acionado e o baixo não, mas pode ser útil para identificar problemas.
 */
bool Tanque::isFalhaSensores() {
  if (digitalRead(this->pinoNivelAlto) == HIGH && digitalRead(this->pinoNivelBaixo) == LOW) {
    return true;
  }

  return false;
}

/**
 * Função responsável por indicar o estado do atuador indicado.
 *
 * Retorna FALSE mesmo quando não existir o indice solicitado.
 */
bool Tanque::isAtuadorLigado(uint8_t indiceAtuador) {
  if (!hasAtuador(indiceAtuador)) {  // Verifica se o indice é maior ou igual ao tamanho do vetor
    return false;                   // caso seja retorna null pointer
  }

  return atuadores[indiceAtuador].isAtuado();  // Caso contrário retorna a referencia do atuador indicado
}

// Verifica se existe um atuador no indice indicado. Retorna True caso exista
bool Tanque::hasAtuador(uint8_t indiceAtuador) {
  return indiceAtuador < atuadores.size();
}

// Liga o atuador indicado. Caso o mesmo já esteja ligado, não faz nada.
void Tanque::ligaAtuador(uint8_t indiceAtuador) {
  if (!isAtuadorLigado(indiceAtuador)) {
    atuadores[indiceAtuador].liga();
  }
}

// Desliga o atuador indicado. Caso o mesmo já esteja desligado, não faz nada.
void Tanque::desligaAtuador(uint8_t indiceAtuador) {
  if (isAtuadorLigado(indiceAtuador)) {
    atuadores[indiceAtuador].desliga();
  }
}

// Funcão responsável por configurar os pinos de entrada de nível do tanque
void Tanque::begin() {
  pinMode(this->pinoNivelAlto, INPUT_PULLUP);
  pinMode(this->pinoNivelBaixo, INPUT_PULLUP);
};