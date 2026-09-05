#ifndef TANQUE_H
#define TANQUE_H

#include <Arduino.h>

#include <vector>

#include "Atuador.h"
#include "SensorPH.h"
#include "SensorTemperatura.h"
#include "SensorTurbidez.h"

class Tanque {
   private:
    SensorTemperatura& temperatura;
    SensorTurbidez& turbidez;
    SensorPH& ph;

    std::vector<Atuador> atuadores;

    uint8_t pinoNivelAlto = 13;
    uint8_t pinoNivelBaixo = 14;

   public:
    Tanque(SensorTemperatura& sensorTemp, uint8_t pinoPH, SensorTurbidez& sensorTurbidez, uint8_t pinoNivelAlto, uint8_t pinoNivelBaixo, const std::vector<Atuador>& atuadores);

    float getTemperatura();  // Função que retornan a temperatura da água do tanque em C°
    float getTurbidez();     // Função que retorna a turbidez da água do tanque em NTU (Nephelometric Turbidity Units)
    float getPH();           // Função que retorna o ph da água do tanque

    bool isNivelAlto();      // Função que indica se o nível da água do tanque está alto
    bool isNivelBaixo();     // Função que indica se o tanque está vazio
    bool isFalhaSensores();  // Função que indica se exite falha nos sensores.

    bool isAtuadorLigado(uint8_t indiceAtuador);  // Função responsável por indicar o estado do atuador indicado.
    bool hasAtuador(uint8_t indiceAtuador);       // Verifica se existe um atuador no indice indicado

    void ligaAtuador(uint8_t indiceAtuador);     // Liga o atuador indicado.
    void desligaAtuador(uint8_t indiceAtuador);  // Desliga o atuador indicado.
};

#endif