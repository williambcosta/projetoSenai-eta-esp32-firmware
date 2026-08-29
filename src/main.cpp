/* ----- BIBLIOTECAS ----- */
#include <Arduino.h>

#include "SensorTemperatura.h"
#include "SensorTurbidez.h"

/* ----- DEFINICÕES ----- */
// Sensores de temperatura
#define PIN_TEMP_AGUA_BRUTA 1       // Pino do sensor de temperatura da água bruta
#define PIN_TEMP_TRAT 4             // Pino do sensor de temperatura da água tratada
#define PIN_TEMP_FINAL 7            // Pino do sensor de temperatura da água final

// Sensores de turbidez
#define PIN_TBDZ_AGUA_BRUTA 35       // Pino do sensor de turbidez da água bruta
#define PIN_TBDZ_TRAT 6             // Pino do sensor de turbidez da água tratada
#define PIN_TBDZ_FINAL 9            // Pino do sensor de turbidez da água final

// Sensores de pH
#define PIN_PH_AGUA_BRUTA 2         // Pino do sensor de pH da água bruta
#define PIN_PH_TRAT 5               // Pino do sensor de pH da água tratada
#define PIN_PH_FINAL 8              // Pino do sensor de pH da água final

// Sensores de nível
#define PIN_SNA_AGUA_BRUTA 20       // Pino do sensor de nível alto da água bruta
#define PIN_SNB_AGUA_BRUTA 21       // Pino do sensor de nível baixo da água bruta
#define PIN_SNA_TRAT 22             // Pino do sensor de nível alto da água tratada
#define PIN_SNB_TRAT 23             // Pino do sensor de nível baixo da água tratada
#define PIN_SNA_FINAL 24            // Pino do sensor de nível alto da água final
#define PIN_SNB_FINAL 25            // Pino do sensor de nível baixo da água final
#define PIN_SNA_EFLU 26             // Pino do sensor de nível alto do efluente
#define PIN_SNB_EFLU 27             // Pino do sensor de nível baixo do efluente

// Bombas de transferência
#define PIN_BOMBA_PT 10             // Pino da bomba de transferência para tratamento
#define PIN_BOMBA_PF 11             // Pino da bomba de transferência  para final
#define PIN_BOMBA_PE 12             // Pino da bomba de transferência para efluente
#define PIN_BOMBA_RE 13             // Pino da bomba de transferência para retorno

// Solenoides
#define PIN_SOLENOIDE 14            // Pino do solenoide de controle de fluxo

// Misturadores
#define PIN_MISTURADOR_TRAT 15      // Pino do misturador da água tratada
#define PIN_MISTURADOR_FINAL 16     // Pino do misturador da água final

// Dosadores
#define PIN_DOSADOR_CLORETO 17      // Pino do dosador de cloreto
#define PIN_DOSADOR_CARBONATO 18    // Pino do dosador de carbonato
#define PIN_DOSADOR_HIPOCLORITO 19  // Pino do dosador de hipoclorito

// Lâmpadas
#define PIN_LAMPADA_UV 28           // Pino da lâmpada UV

// Instancias dos sensores de temperatura
//SensorTemperatura tempArmazenamento = SensorTemperatura(PIN_TEMP_AGUA_BRUTA); // Sensor de temperatura da água bruta
//SensorTemperatura tempTrat = SensorTemperatura(PIN_TEMP_TRAT);                // Sensor de temperatura da água em tratamento
//SensorTemperatura tempFinal = SensorTemperatura(PIN_TEMP_FINAL);              // Sensor de temperatura da água final

SensorTurbidez ntu = SensorTurbidez(PIN_TBDZ_AGUA_BRUTA, 500);


/* ----- Configuração inicial ----- */
void setup() {
/*
    // TODO: Implementar o tanque de mistura
    // TODO: Criar instancias de tanque para cada um deles
    // Define os pinos dos sensores de nível como entrada
    pinMode(PIN_SNA_AGUA_BRUTA, INPUT_PULLDOWN);
    pinMode(PIN_SNB_AGUA_BRUTA, INPUT_PULLDOWN);
    pinMode(PIN_SNA_TRAT, INPUT_PULLDOWN);
    pinMode(PIN_SNB_TRAT, INPUT_PULLDOWN);
    pinMode(PIN_SNA_FINAL, INPUT_PULLDOWN);
    pinMode(PIN_SNB_FINAL, INPUT_PULLDOWN);
    pinMode(PIN_SNA_EFLU, INPUT_PULLDOWN);
    pinMode(PIN_SNB_EFLU, INPUT_PULLDOWN);

    // Define os pinos dos atuadores como saída
    pinMode(PIN_BOMBA_PE, OUTPUT);
    pinMode(PIN_BOMBA_PT, OUTPUT);
    pinMode(PIN_BOMBA_PF, OUTPUT);
    pinMode(PIN_BOMBA_RE, OUTPUT);

    pinMode(PIN_MISTURADOR_TRAT, OUTPUT);
    pinMode(PIN_MISTURADOR_FINAL, OUTPUT);

    pinMode(PIN_DOSADOR_CLORETO, OUTPUT);
    pinMode(PIN_DOSADOR_CARBONATO, OUTPUT);
    pinMode(PIN_DOSADOR_HIPOCLORITO, OUTPUT);

    pinMode(PIN_SOLENOIDE, OUTPUT);

    pinMode(PIN_LAMPADA_UV, OUTPUT);
*/

    Serial.begin(115200);  // Inicializa a comunicação serial com o computador
}


/* ----- Loop principal ----- */
void loop() {

    ntu.getTurbidez();

    delay(1000);

}
