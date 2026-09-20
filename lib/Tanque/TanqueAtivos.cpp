/**
 * Classe que representa o tanque de água em tratamento. É responsável por gerenciar os sensores e atuadores específicos desse tanque,
 * além dos comuns a todos os tanques, sendo eles: sensor de turbidez, sensor de pH e sensor de temperatura.
 */

#include "TanqueAtivos.h"

TanqueAtivos::TanqueAtivos(SensorTemperatura& sensorTemp, SensorPH& sensorPH,
                           SensorTurbidez& sensorTurbidez, const std::vector<SaidaDigital>& atuadores,
                           uint8_t pinoNivelAlto, uint8_t pinoNivelBaixo)
    : Tanque(atuadores, pinoNivelAlto, pinoNivelBaixo), temperatura(sensorTemp), turbidez(sensorTurbidez), ph(sensorPH) {
}

// Função que retornan a temperatura da água do tanque em C°
float TanqueAtivos::getTemperatura() {
  return temperatura.getTemperatura();
}

// Função que retorna a turbidez da água do tanque em NTU (Nephelometric Turbidity Units)
float TanqueAtivos::getTurbidez() {
  return turbidez.getTurbidez();
}

// Função que retorna o ph da água do tanque
float TanqueAtivos::getPH() {
  return ph.getPH();
}