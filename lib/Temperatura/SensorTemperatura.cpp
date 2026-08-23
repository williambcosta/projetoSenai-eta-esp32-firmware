/*
 * Esta classe auxilia na leitura dos sensores de temperatura.
 *
 * Ela contém uma referencia de OneWire, que é responsável pela comunicação com os sensores.
 * Cada instancia de SensorTemperatura terá sua propria referencia de OneWire.
 *  isso não é necessário, porém caso um dos sensores se desconectar o indice do mesmo será alterado, o que forçaria
 *  a implementação de uma lógica de setup do endereço unico do sensor. Colocando cada sensor em um pino diferente
 *  facilita a implementação.
 *
 * Uma insntancia é criada indicando o pino ao qual o sensor está conectado. O mesmo deve ser um pino digital.
 * Não é necessário indicar o pino como OUTPUT. Isso pode travar a comunicação e até queimar a entrada do microprocessador.
 */

#include "SensorTemperatura.h"

// Construtor do objeto. Configura e inicia a comunicação com o sensor.
SensorTemperatura::SensorTemperatura(uint8_t pinoOneWire) {
    oneWire = OneWire(pinoOneWire);
    sensor = DallasTemperature(&oneWire);
    sensor.begin();
}

// Retorna a temperatura em  graus Celsios
float SensorTemperatura::getTemperatura() {
    sensor.requestTemperatures();

    if (sensor.getAddress(enderecoSensor, 0)) {  // Caso exista algum sensor conectado
        return sensor.getTempC(enderecoSensor);  // Retorna a temperatura em °C
    }

    return -999.9;  // Caso contrário é retornado o valor -999.9 para indicar falha 
}