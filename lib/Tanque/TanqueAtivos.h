#ifndef TANQUE_ATIVOS_H
#define TANQUE_ATIVOS_H

#include <Arduino.h>

#include <vector>

#include "SaidaDigital.h"
#include "SensorPH.h"
#include "SensorTemperatura.h"
#include "SensorTurbidez.h"
#include "Tanque.h"

class TanqueAtivos: public Tanque {
 private:
  SensorTemperatura& temperatura;
  SensorTurbidez& turbidez;
  SensorPH& ph;

 public:
  TanqueAtivos(SensorTemperatura& sensorTemp, SensorPH& sensorPH,
         SensorTurbidez& sensorTurbidez, const std::vector<SaidaDigital>& atuadores,
         uint8_t pinoNivelAlto, uint8_t pinoNivelBaixo);

  float getTemperatura();  // Função que retornan a temperatura da água do tanque em C°
  float getTurbidez();     // Função que retorna a turbidez da água do tanque em NTU (Nephelometric Turbidity Units)
  float getPH();           // Função que retorna o ph da água do tanque
};

#endif