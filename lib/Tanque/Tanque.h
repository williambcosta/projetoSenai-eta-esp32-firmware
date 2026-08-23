#ifndef TANQUE_H
#define TANQUE_H

#include <Arduino.h>

#include "SensorTemperatura.h"
#include "SensorTurbidez.h"

class Tanque {
   private:
    SensorTemperatura temperatura;
    SensorTurbidez turbidez;

    // TODO: Implementar classe motor

    uint8_t pinoNivelAlto;
    uint8_t pinoNivelBaixo;

    float ph;


   public:
    Tanque() {
        ph = 7.0;
        pinoNivelAlto = 0;
        pinoNivelBaixo = 0;
    };
    Tanque(uint8_t pinoTemp, uint8_t pinoPH, uint8_t pinoTurbidez, uint8_t pinoNivelAlto, uint8_t pinoNivelBaixo);

    float getTemperatura();
    float getTurbidez();
    float getPH();

    bool isNivelAlto();
    bool isNivelBaixo();
    bool isVazio();
    bool isFalhaSensores();
};

#endif