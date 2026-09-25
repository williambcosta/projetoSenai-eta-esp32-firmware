/* ----- BIBLIOTECAS ----- */
#include <Arduino.h>
#include <WiFi.h>

#include <vector>

#include "MqttManager.h"
#include "SaidaDigital.h"
#include "SensorPH.h"
#include "SensorTemperatura.h"
#include "SensorTurbidez.h"
#include "ShiftRegister.h"
#include "Tanque.h"
#include "TanqueTratamento.h"

/* ----- DEFINICÕES ----- */
/* ----- Configurações de Comunicação ----- */
#define MQTT_BROKER "90d42cec75d14181b23673d72f964713.s1.eu.hivemq.cloud"
#define MQTT_PORTA 8883
#define MQTT_SENHA "esp12345"
#define MQTT_USUARIO "espclient"
#define WIFI_SENHA "#Ws120912"
#define WIFI_SSID "Willian"

/* ----- Entradas Analógicas ----- */
// Sensores de turbidez
#define PIN_TBDZ_ATIVOS 34  // Pino do sensor de turbidez da água tratada
#define PIN_TBDZ_FINAL 35   // Pino do sensor de turbidez da água final

// Sensores de pH
#define PIN_PH_ATIVOS 36  // Pino do sensor de pH da água tratada
#define PIN_PH_FINAL 39   // Pino do sensor de pH da água final

/* ----- Entradas Digitais ----- */
// Sensores de temperatura
#define PIN_TEMP_ATIVOS 4  // Pino do sensor de temperatura da água tratada
#define PIN_TEMP_FINAL 13  // Pino do sensor de temperatura da água final

// Sensores de nível
#define PIN_SNA_ARMAZENAMENTO 14  // Pino do sensor de nível alto tanque água bruta
#define PIN_SNB_ARMAZENAMENTO 16  // Pino do sensor de nível baixo tanque água bruta
#define PIN_SNA_ATIVOS 17         // Pino do sensor de nível alto tanque ativos
#define PIN_SNB_ATIVOS 18         // Pino do sensor de nível baixo tanque ativos
#define PIN_SNA_FINAL 19          // Pino do sensor de nível alto tanque final
#define PIN_SNB_FINAL 21          // Pino do sensor de nível baixo tanque final
#define PIN_SNA_EFLU 22           // Pino do sensor de nível alto tanque efluente
#define PIN_SNB_EFLU 23           // Pino do sensor de nível baixo tanque efluente

/* ----- Saídas Digitais ----- */
// Comunicação 74HC595
#define CLK 25    // Clock do registrador de deslocamento. Cada pulso nesse pino lê o bit que está na entrada DS e o desloca internamente.
#define LATCH 26  // Clock de armazenamento ou Latch. Storage Register Clock. Um pulso aqui transfere os dados armazenados internamente para os pinos de saída (Q0–Q7) de uma só vez
#define DADOS 27  // Entrada de dados serial. É por onde os bits entram no chip, um de cada vez.

// Bombas de transferência
#define PIN_BOMBA_PM1 "0.0"  // Pino da bomba de transferência para tanque de ativos
#define PIN_BOMBA_PM2 "0.1"  // Pino da bomba de transferência para final/efluentes
#define PIN_BOMBA_PM3 "0.2"  // Pino da bomba de retorno para tanque de armazenamento

// Solenoides
#define PIN_SOL_EFLU "0.3"   // Pino da solenoide de controle de fluxo para o tanque de efluentes
#define PIN_SOL_FINAL "0.4"  // Pino da solenoide de controle de fluxo para o tanque final

// Misturadores
#define PIN_RM1_ATIV "0.5"   // Pino do misturador do tanque de ativos
#define PIN_RM2_FINAL "0.6"  // Pino do misturador do tanque final

// Dosadores
#define PIN_DOSADOR_CLORETO "0.7"      // Pino do dosador de cloreto
#define PIN_DOSADOR_CARBONATO "1.0"    // Pino do dosador de carbonato
#define PIN_DOSADOR_HIPOCLORITO "1.1"  // Pino do dosador de hipoclorito

// Lâmpadas
#define PIN_LAMPADA_UV "1.2"  // Pino da lâmpada UV

// Enumeração para representar as etapas do processo de tratamento de água
enum class EtapaProcesso {
  Inicial,
  VerificNivelAltoArmaz,
  EsvaziandoTqArmaz,
  VerificTransferenciaAtivos,
  DosandoCoagulante,
  VerificNivelBaixoAtivos,
  DosandoAlcalinizante,
  Coagulacao,
  RemovendoSedimentos,
  DosandoSanitizante,
  EsvaziandoTanqueAtivos,
  TratamentoUv,
  Finalizado
};

EtapaProcesso etapaAtual = EtapaProcesso::Inicial;  // Variável para armazenar a etapa atual do processo

/*****  TANQUE DE ARMAZENAMENTO *****/
// Atuadores
std::vector<SaidaDigital> saidasArmazenamento = {SaidaDigital(PIN_BOMBA_PM1)};

// Instancia
Tanque tanqueArmazenamento = Tanque(saidasArmazenamento, PIN_SNA_ARMAZENAMENTO, PIN_SNB_ARMAZENAMENTO);

/***** TANQUE DE ATIVOS *****/
// Sensores
SensorTemperatura tempAtiv = SensorTemperatura(PIN_TEMP_ATIVOS);                     // Temperatura
SensorTurbidez ntuAtiv = SensorTurbidez(PIN_TBDZ_ATIVOS, 250, 2.0f, 0.0f);           // Turbidez
SensorPH phAtivos = SensorPH(PIN_PH_ATIVOS, 4.0f, 3705.5, 3105.0f, 10.0f, 2684.0f);  // Ph

// Atuadores
std::vector<SaidaDigital> saidasAtivos = {  // Indices
  SaidaDigital(PIN_BOMBA_PM2),              // 0
  SaidaDigital(PIN_RM1_ATIV),               // 1
  SaidaDigital(PIN_DOSADOR_CLORETO),        // 2
  SaidaDigital(PIN_DOSADOR_CARBONATO),      // 3
  SaidaDigital(PIN_DOSADOR_HIPOCLORITO),    // 4
  SaidaDigital(PIN_SOL_EFLU),               // 5
  SaidaDigital(PIN_SOL_FINAL)               // 6
};

// Instancia
TanqueTratamento tanqueAtivos = TanqueTratamento(tempAtiv, phAtivos, ntuAtiv, saidasAtivos, PIN_SNA_ATIVOS, PIN_SNB_ATIVOS);

/***** TANQUE FINAL *****/
// Sensores
SensorTemperatura tempFinal = SensorTemperatura(PIN_TEMP_FINAL);  // Temperatura
SensorTurbidez ntuFinal = SensorTurbidez(PIN_TBDZ_FINAL, 250, 2.0f, 0.0f);         // Turbidez
SensorPH phFinal = SensorPH(PIN_PH_FINAL, 4.0f, 3705.5, 3105.0f, 10.0f, 2684.0f);  // Ph

// Atuadores
std::vector<SaidaDigital> saidasFinal = {SaidaDigital(PIN_RM2_FINAL), SaidaDigital(PIN_LAMPADA_UV)};

// Instancia
Tanque tanqueFinal = Tanque(saidasFinal, PIN_SNA_FINAL, PIN_SNB_FINAL);

/***** TANQUE DE EFLUENTES *****/
// Atuadores
std::vector<SaidaDigital> saidasEfluentes = {SaidaDigital(PIN_BOMBA_PM3)};

// Instancia
Tanque tanqueEfluentes = Tanque(saidasEfluentes, PIN_SNA_EFLU, PIN_SNB_EFLU);

/* ----- Variáveis utilitárias e de controle do processo ----- */
int tempoDeVerificacao = 1000;         // Intervalo de tempo em milissegundos para verificar o nível dos tanques
unsigned long tempoArmazenamento = 0;  // Variável para armazenar o tempo anterior em milissegundos
unsigned long tempoAtivos = 0;         // Variável para armazenar o tempo anterior em milissegundos
unsigned long tempoFinal = 0;          // Variável para armazenar o tempo anterior em milissegundos

// Instancia do gerenciador MQTT para comunicação com o HiveMQ Cloud
MqttManager mqtt = MqttManager(WIFI_SSID, WIFI_SENHA, MQTT_BROKER, MQTT_PORTA, MQTT_USUARIO, MQTT_SENHA);

/* ----- Configuração inicial ----- */
void setup() {
  delay(1000);  // Aguarda 1 segundo para garantir que o sistema esteja estável antes de iniciar a configuração

  // Inicializa os tanques
  tanqueArmazenamento.begin();
  tanqueAtivos.begin();
  tanqueEfluentes.begin();
  tanqueFinal.begin();

  // Inicializa o registrador de deslocamento
  ShiftRegister::Instance().begin(DADOS, CLK, LATCH);

  // Inicializa a comunicação MQTT com os tópicos de comandos e dados
  mqtt.begin("espclient_Comandos", "espclient_Dados");

  Serial.begin(115200);  // Inicializa a comunicação serial
}

/* ----- Loop principal ----- */
void loop() {
  // TODO: Implementar alertas
  // mqtt.handle();

  // mqtt.publish(String(random(20, 35)).c_str());

  // * Nível baixo sempre manda 1
  // * Dosadores 2 ml/s

  /* Processo Tanque Armazenamento */
  //  1.1 - Se nível alto && Nível baixo ...
  //  1.2 - ...por 1s liga bomba PM1
  //  1.3 - Se nível baixo por 1s tanque armazenamento em qualquer momento do processo...
  //  1.4 - ...desliga PM1
  //  1.5 - PM1 ativo até tanque ativos nível alto por 1s

  /* Processo Tanque Ativos */
  //  4 - Nível alto && nível baixo tanque ativos por 1s
  //  5 - Inicia agitação RM1
  //  6 - Calcula o tempo de dosagem do coagulante (formula ? vai considerar o ph)
  //  7 - Dosar um pouco por vez e medir o ph após pausa na dosagem de ?s
  //  8 - Ph em nível X interromper dosagem de coagulante
  //  9 - Inicia dosagem de alcalinizante
  // 10 - Calcula o tempo de dosagem do alcalinizante (formula ? vai considerar o ph)
  // 11 - Dosar um pouco por vez e medir o ph após pausa na dosagem de ?s
  // 12 - Ph em nível X interromper dosagem de alcalinizante
  // 13 - Mantém a agitação por mais 5s
  // 14 - Pausa a agitação por 5m
  // 15 - Aciona HVK1, Descarrega efluentes (acionando PM2) após verificação de NTU
  // 16 - Manter PM2 por +/- 30s
  // 17 - Desaciona HVK1
  // 18 - Iniciar agitação (RM1)
  // 19 - Calcula o tempo de dosagem do sanitizante (formula ? vai considerar o volume)
  // 20 - Dosar um pouco por vez e medir o ph após pausa na dosagem de ?s
  // 21 - Ph em nível 7 interromper dosagem de sanitizante
  // 22 - Mantém a agitação até PH interrupção de dosagem sanitizante
  // 23 - Aciona HVK2
  // 24 - Aciona PM2 se nível baixo
  // 25 - PM2 desaciona quando Armazenamento Nivel alto || Ativos Nível Baixo

  /* Processo Tanque Final */
  // 26 - Se !Nível alto && nível baixo Aciona H1 e RM2
  // 27 - Se Nível alto Desaciona PM2
  // 28 - Se !Nível baixo && !Nível alto para RM2 e H1

  /* Efluentes */
  // 29 - Quando nível alto && (!Nível alto  && Nível baixo Armazenamento)
  // 30 - Aciona PM3
  // 31 - Se Armazenamento Nível alto || Efluentes Nível baixo
  // 32 - Para PM3

  /* Processo Tanque Armazenamento */
  // Tanque de armazenamento Enchendo
  if (etapaAtual == EtapaProcesso::Inicial) {
    if (tanqueArmazenamento.isNivelAlto()) {              // (* 1.1) Se nível alto
      etapaAtual = EtapaProcesso::VerificNivelAltoArmaz;  // Muda a etapa do processo para "VerificNivelAltoArmaz"
      tempoArmazenamento = millis();                      // Armazena o tempo atual em milissegundos
    }
  }

  // Verificando se o tanque de armazenamento está cheio e controlando a bomba PM1
  if (etapaAtual == EtapaProcesso::VerificNivelAltoArmaz) {
    if (tanqueArmazenamento.isNivelAlto()) {
      if (millis() - tempoArmazenamento >= tempoDeVerificacao) {  // (* 1.2) Verifica se se passou 1 segundo desde o último acionamento
        etapaAtual = EtapaProcesso::EsvaziandoTqArmaz;            // Muda a etapa do processo para "EsvaziandoTqArmaz"
        tanqueArmazenamento.ligaAtuador(0);                       // (* 1.2)Liga a bomba PM1 para transferir água para o tanque de ativos
      }
    }
  }

  // Esvaziando o tanque de armazenamento e controlando a bomba PM1
  if (etapaAtual == EtapaProcesso::EsvaziandoTqArmaz) {
    if (tanqueArmazenamento.isNivelBaixo() || tanqueAtivos.isNivelAlto()) {  // (* 1.3) Se nível baixo
      etapaAtual = EtapaProcesso::VerificTransferenciaAtivos;                // Muda a etapa do processo para "VerificTransferenciaAtivos"
      tempoArmazenamento = millis();                                         // Armazena o tempo atual em milissegundos para o tanque de ativos
    }
  }

  // Verificando se o tanque de armazenamento está vazio e controlando a bomba PM1
  if (etapaAtual == EtapaProcesso::VerificTransferenciaAtivos) {
    if (tanqueArmazenamento.isNivelBaixo() || tanqueAtivos.isNivelAlto()) {  // Se nível alto
      if (millis() - tempoArmazenamento >= tempoDeVerificacao) {             // (* 1.3) Verifica se se passou 1 segundo desde o último acionamento
        tanqueArmazenamento.desligaAtuador(0);                               // (* 1.4) Desliga a bomba PM1 para interromper a transferência de água para o tanque de ativos
        etapaAtual = EtapaProcesso::DosandoCoagulante;                       // Muda a etapa do processo para "DosandoCoagulante"
      }
    }
  }

  // TODO: Verificar se caso o armazenamento chegue no nível baixo antes de encher o ativos e vice versa o que fazer
}
