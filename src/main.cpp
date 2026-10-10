/*******************************************************************************************************************************/
/***** BIBLIOTECAS *************************************************************************************************************/
/*******************************************************************************************************************************/
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <time.h>

#include <vector>

#include "MqttManager.h"
#include "SaidaDigital.h"
#include "SensorPH.h"
#include "SensorTemperatura.h"
#include "SensorTurbidez.h"
#include "ShiftRegister.h"
#include "Tanque.h"
#include "TanqueTratamento.h"
#include "configuracoes.h"  // Configurações de comunicação e Wi-Fi
#include "processo.h"       // Declaração da estrutura do processo

/*******************************************************************************************************************************/
/***** DEFINICÕES **************************************************************************************************************/
/*******************************************************************************************************************************/
/***** Entradas Analógicas *****/
// Sensores de turbidez
#define PIN_TBDZ_ATIVOS 34  // Pino do sensor de turbidez da água inicial
#define PIN_TBDZ_FINAL 35   // Pino do sensor de turbidez da água final

// Sensores de pH
#define PIN_PH_ATIVOS 36  // Pino do sensor de pH da água tratada
#define PIN_PH_FINAL 39   // Pino do sensor de pH da água final

/***** Entradas Digitais *****/
// Sensores de temperatura
#define PIN_TEMP_INICIAL 4  // Pino do sensor de temperatura da água inicial
#define PIN_TEMP_ATIVOS 13  // Pino do sensor de temperatura da água tratada
#define PIN_TEMP_FINAL 32   // Pino do sensor de temperatura da água final

// Sensores de nível
#define PIN_SNA_ARMAZENAMENTO 14  // Pino do sensor de nível alto tanque água bruta
#define PIN_SNB_ARMAZENAMENTO 16  // Pino do sensor de nível baixo tanque água bruta
#define PIN_SNA_ATIVOS 17         // Pino do sensor de nível alto tanque ativos
#define PIN_SNB_ATIVOS 18         // Pino do sensor de nível baixo tanque ativos
#define PIN_SNA_FINAL 19          // Pino do sensor de nível alto tanque final
#define PIN_SNB_FINAL 21          // Pino do sensor de nível baixo tanque final
#define PIN_SNA_EFLU 22           // Pino do sensor de nível alto tanque efluente
#define PIN_SNB_EFLU 23           // Pino do sensor de nível baixo tanque efluente

/***** Saídas Digitais *****/
// Comunicação 74HC595
#define CLK 25    // Clock do registrador de deslocamento. Cada pulso nesse pino lê o bit que está na entrada DS e o desloca internamente um a um.
#define LATCH 26  // Clock de armazenamento ou Latch. Um pulso aqui transfere os dados armazenados internamente para os pinos de saída (Q0–Q7) de uma só vez
#define DADOS 27  // Entrada de dados serial. É por onde os bits entram no chip, um de cada vez.

// Bombas de transferência
#define PIN_BOMBA_PM1 "0.0"  // Pino da bomba de transferência para tanque de ativos
#define PIN_BOMBA_PM2 "0.1"  // Pino da bomba de transferência para final/efluentes
#define PIN_BOMBA_PM3 "0.2"  // Pino da bomba de retorno para tanque de armazenamento

// Solenoides
#define PIN_SOL_EFLU_SV1 "0.3"   // Pino da solenoide de controle de fluxo para o tanque de efluentes
#define PIN_SOL_FINAL_SV2 "0.4"  // Pino da solenoide de controle de fluxo para o tanque final

// Misturadores
#define PIN_RM1_ATIV "0.5"   // Pino do misturador do tanque de ativos
#define PIN_RM2_FINAL "0.6"  // Pino do misturador do tanque final

// Dosadores
#define PIN_DOSADOR_FeCl3_DM1 "0.7"  // Pino do dosador de cloreto
#define PIN_DOSADOR_CaCO3_DM2 "1.0"  // Pino do dosador de carbonato
#define PIN_DOSADOR_NaClO_DM3 "1.1"  // Pino do dosador de hipoclorito

// Lâmpadas
#define PIN_LAMPADA_H1 "1.2"  // Pino da lâmpada UV

// Instancia da estrutura do processo
Processo processo;

/*******************************************************************************************************************************/
/*****  TANQUE DE ARMAZENAMENTO ************************************************************************************************/
/*******************************************************************************************************************************/
// Sensores
SensorTemperatura tempArmaz = SensorTemperatura(PIN_TEMP_INICIAL);  // Temperatura

// Atuadores
std::vector<SaidaDigital> saidasArmazenamento = {SaidaDigital(PIN_BOMBA_PM1)};  // Bomba de Transferencia de agua

// Instancia
Tanque tanqueArmazenamento = Tanque(saidasArmazenamento, PIN_SNA_ARMAZENAMENTO, PIN_SNB_ARMAZENAMENTO);  // Instancia do tanque

/*******************************************************************************************************************************/
/******* TANQUE DE EFLUENTES ***************************************************************************************************/
/*******************************************************************************************************************************/
// Atuadores
std::vector<SaidaDigital> saidasEfluentes = {SaidaDigital(PIN_BOMBA_PM3)};

// Instancia
Tanque tanqueEfluentes = Tanque(saidasEfluentes, PIN_SNA_EFLU, PIN_SNB_EFLU);

/*******************************************************************************************************************************/
/********* TANQUE DE ATIVOS ****************************************************************************************************/
/*******************************************************************************************************************************/
// Sensores
SensorTemperatura tempAtiv = SensorTemperatura(PIN_TEMP_ATIVOS);                        // Temperatura
SensorTurbidez ntuAtiv = SensorTurbidez(PIN_TBDZ_ATIVOS, 5, 2.0f, 0.0f);                // Turbidez
SensorPH phAtivos = SensorPH(PIN_PH_ATIVOS, 5, 4.0f, 3705.5, 3105.0f, 10.0f, 2684.0f);  // Ph

// Atuadores
std::vector<SaidaDigital> saidasAtivos = {
    // Indices
    SaidaDigital(PIN_BOMBA_PM2),          // 0 - Bomba de teansferencia do tanque de ativos para tanque final/efluentes
    SaidaDigital(PIN_RM1_ATIV),           // 1 - Mexedor
    SaidaDigital(PIN_DOSADOR_FeCl3_DM1),  // 2 - Coagulante
    SaidaDigital(PIN_DOSADOR_CaCO3_DM2),  // 3 - Alcalinizante
    SaidaDigital(PIN_DOSADOR_NaClO_DM3),  // 4 - Sanitizante
    SaidaDigital(PIN_SOL_EFLU_SV1),       // 5 - Solenoide de controle de fluxo para efluentes
    SaidaDigital(PIN_SOL_FINAL_SV2)       // 6 - Solenoide de controle de fluxo para tanque final
};

// Instancia
TanqueTratamento tanqueAtivos = TanqueTratamento(tempAtiv, phAtivos, ntuAtiv, saidasAtivos, PIN_SNA_ATIVOS, PIN_SNB_ATIVOS);

/*******************************************************************************************************************************/
/*********** TANQUE FINAL ******************************************************************************************************/
/*******************************************************************************************************************************/
// Sensores
SensorTemperatura tempFinal = SensorTemperatura(PIN_TEMP_FINAL);                      // Temperatura
SensorTurbidez ntuFinal = SensorTurbidez(PIN_TBDZ_FINAL, 5, 2.0f, 0.0f);              // Turbidez
SensorPH phFinal = SensorPH(PIN_PH_FINAL, 5, 4.0f, 3705.5, 3105.0f, 10.0f, 2684.0f);  // Ph

// Atuadores
std::vector<SaidaDigital> saidasFinal = {SaidaDigital(PIN_RM2_FINAL), SaidaDigital(PIN_LAMPADA_H1)};

// Instancia
TanqueTratamento tanqueFinal = TanqueTratamento(tempFinal, phFinal, ntuFinal, saidasFinal, PIN_SNA_FINAL, PIN_SNB_FINAL);

/*******************************************************************************************************************************/
/***** VARIÁVEIS UTILITÁRIAS E DE CONTROLE DO PROCESSO *************************************************************************/
/*******************************************************************************************************************************/
// Parâmetros de tempo (em milisegundos)
int tempoDeVerificacao = 1000;         // Tempo para verificar o nível dos tanques
int tempoDosagemCoagulante = 5000;     // Tempo total de dosagem do coagulante
int tempoDosagemAlcalinizante = 5000;  // Tempo de dosagem do alcalinizante
int tempoDosagemSanitizante = 5000;    // Tempo de dosagem do sanitizante
int tempoPausaDosagem = 5000;          // Tempo que determina quando a dosagem será pausada para homogeneizar os quimicos com a agua
int tempoHomogeneizacao = 5000;        // Tempo após dosagem de quimica para homogeneização da agua antes de nova medição de parâmetros
int tempoDescargaEfluentes = 30000;    // Tempo que a bomba de transferencia do tanque de ativos fica ligada ao esvaziar o decantado para o tanque de efluentes
int tempoCoagulacao = 300000;          // Tempo de espera para que a coagulação aconteça
int tempoTratamentoUv = 300000;        // Tempo mínimo de espera para que o tratamento UV seja efetivo
int tempoUpdate = 2000;                // Tempo de atualização da telemetria enviada ao broker MQTT

// Temporizadores
unsigned long dtDosagemCoagulante = 0;     // Tempo percorrido de dosagem de coagulante
unsigned long dtDosagemAlcalinizante = 0;  // Tempo percorrido de dosagem de alcalinizante
unsigned long dtDosagemSanitizante = 0;    // Tempo percorrido de dosagem de sanitizante
unsigned long dtDescargaEfluentes = 0;     // Tempo percorrido de descarga dos efluentes
unsigned long dtArmazenamento = 0;         // Tempo percorrido de verificação do tanque de armazenamento
unsigned long dtAtivos = 0;                // Tempo percorrido de verificação do tanque de ativos
unsigned long dtFinal = 0;                 // Tempo percorrido de verificação do tanque de final
unsigned long dtHomogeneizacao = 0;        // Tempo percorrido de homogeneização de quimicos
unsigned long dtCoagulacao = 0;            // Tempo percorrido de coagulação dos residuos do tanque de ativos
unsigned long dtPausaDosagem = 0;          // Tempo percorrido de pausa de dosagem para homogeneização da água
unsigned long dtUpdate = 0;                // Tempo percorrido de atualização da telemetria
// unsigned long dt = 0;                      // Tempo de duração de uma interação inteira de loop

// Parâmetros das caracteristicas da água
float histerese = 0.3f;            // Histerese para verificação de ph
float alvoPhCoagulante = 7.0f;     // Alvo de pH após dosagem do Coagulante
float alvoPhAlcalinizante = 7.0f;  // Alvo de pH após dosagem do Alcalinizante
float alvoPhSanitizante = 7.0f;    // Alvo de pH após dosagem do Sanitizante
float alvoNtu = 5.0f;              // NTU alvo após coagulação

// Instancia do gerenciador MQTT para comunicação com o HiveMQ Cloud
MqttManager mqtt = MqttManager(WIFI_SSID, WIFI_SENHA, MQTT_BROKER, MQTT_PORTA, MQTT_USUARIO, MQTT_SENHA);

JsonDocument telemetria;  // Objeto Json que será enviado ao broker mqtt com os dados do processo

char mensagem[128];           //  Buffer para armazenar a mensagem recebida do broker MQTT
char bufferTelemetria[1025];  // Buffer para armazenar a telemetria em formato JSON antes de enviar ao broker MQTT
char dataHoraAtual[25];       // Buffer para armazenar a data e hora atual em formato de string, static garante que a variável permaneça na memória

/*******************************************************************************************************************************/
/***** SETUP *******************************************************************************************************************/
/*******************************************************************************************************************************/
// Configuração inicial
void setup() {
  delay(1000);  // Aguarda 1 segundo para garantir que o sistema esteja estável antes de iniciar a configuração

  Serial.begin(115200);  // Inicializa a comunicação serial

  // Inicializa os tanques
  tanqueArmazenamento.begin();
  tanqueAtivos.begin();
  tanqueEfluentes.begin();
  tanqueFinal.begin();

  // Inicializa o registrador de deslocamento
  ShiftRegister::Instance().begin(DADOS, CLK, LATCH);

  // Inicializa a comunicação MQTT com os tópicos de comandos, dados e alertas alem de conectar ao broker e wifi
  mqtt.begin("eta/comandos", "eta/telemetria", "eta/alertas");

  // Esqueleto do objeto json que será enviado ao broker mqtt
  String esqueletoJson = R"({
    "data_hora": "2026-10-02T14:30:00Z",
    "modo_operacao": "Automatico",
    "etapa_processo": "Inicial",
    "tanque_armazenamento": {
      "nivel_alto" : false,
      "nivel_baixo" : false,
      "bomba_pm1" : false,
      "temperatura" : 0.0
    },
    "tanque_ativos": {
      "nivel_alto" : false,
      "nivel_baixo" : false,
      "bomba_pm2" : false,
      "mexedor_rm1" : false,
      "dosador_fecl3_dm1" : false,
      "dosador_caco3_dm2" : false,
      "dosador_naclo_dm3" : false,
      "solenoide_hvk1" : false,
      "solenoide_hvk2" : false,
      "temperatura" : 0.0,
      "turbidez" : 0.0,
      "ph" : 0.0
    },
    "tanque_efluentes": {
      "nivel_alto" : false,
      "nivel_baixo" : false,
      "bomba_pm3" : false
    },
    "tanque_final": {
      "nivel_alto" : false,
      "nivel_baixo" : false,
      "mexedor_rm2" : false,
      "lampada_uv" : false,
      "temperatura" : 0.0,
      "turbidez" : 0.0,
      "ph" : 0.0
    }
  })";

  // Verifica se existe erro na formatação do json
  DeserializationError erro = deserializeJson(telemetria, esqueletoJson);
  if (erro) {
    Serial.print("Falha ao processar JSON: ");
    Serial.println(erro.f_str());
    Serial.println("");
  }

  delay(5000);  // Aguarda 5 segundos para garantir que o sistema esteja estável antes de sincronizar a hora com o servidor NTP

  // Configuração de data e hora
  configTime(-3 * 3600, 0, "pool.ntp.org");  // Configura o fuso horário para o horário de Brasília (UTC-3) e define o servidor NTP para sincronização do tempo

  struct tm horaLocal;  // Estrutura para armazenar a data e hora atual
  // Aguarda a sincronização do horário de São Paulo antes de prosseguir
  if (WiFi.status() == WL_CONNECTED) {
    unsigned long inicioNtp = millis();
    // Tenta sincronizar durante no máximo 10 segundos
    while (!getLocalTime(&horaLocal) && (millis() - inicioNtp < 10000)) {
      Serial.println("Aguardando sincronização NTP...");
      delay(500);
    }
  }

  dtUpdate = millis();  // Inicializa o temporizador de atualização da telemetria
}

/*******************************************************************************************************************************/
/***** ATUALIZA DATA E HORA ****************************************************************************************************/
/*******************************************************************************************************************************/
// Atualiza a data e hora atual em formato de (YYYY-MM-DDTHH:MM:SSZ)
void atualizaDataHoraAtual() {
  struct tm infoData;
  if (getLocalTime(&infoData)) {
    strftime(dataHoraAtual, sizeof(dataHoraAtual), "%Y-%m-%dT%H:%M:%SZ", &infoData);
  }
}

/*******************************************************************************************************************************/
/***** ATUALIZA TELEMETRIA *****************************************************************************************************/
/*******************************************************************************************************************************/
// Atualiza os dados no documento json e o buffer para facilitar o envio da mensagem
void atualizaTelemetria() {
  atualizaDataHoraAtual();
  // Dados do processo
  telemetria["data_hora"] = dataHoraAtual;
  telemetria["modo_operacao"] = processo.modoOperacaoToString(processo.modoOperacao);
  telemetria["etapa_processo"] = processo.etapaProcessoToString(processo.etapaAtual);

  // Tanque de armazenamento
  telemetria["tanque_armazenamento"]["nivel_alto"] = tanqueArmazenamento.getEstadoNivelAlto();
  telemetria["tanque_armazenamento"]["nivel_baixo"] = tanqueArmazenamento.getEstadoNivelBaixo();
  telemetria["tanque_armazenamento"]["bomba_pm1"] = tanqueArmazenamento.isAtuadorLigado(0);
  telemetria["tanque_armazenamento"]["temperatura"] = tempArmaz.getTemperatura();

  // Tanque de ativos
  telemetria["tanque_ativos"]["nivel_alto"] = tanqueAtivos.getEstadoNivelAlto();
  telemetria["tanque_ativos"]["nivel_baixo"] = tanqueAtivos.getEstadoNivelBaixo();
  telemetria["tanque_ativos"]["bomba_pm2"] = tanqueAtivos.isAtuadorLigado(0);
  telemetria["tanque_ativos"]["mexedor_rm1"] = tanqueAtivos.isAtuadorLigado(1);
  telemetria["tanque_ativos"]["dosador_fecl3_dm1"] = tanqueAtivos.isAtuadorLigado(2);
  telemetria["tanque_ativos"]["dosador_caco3_dm2"] = tanqueAtivos.isAtuadorLigado(3);
  telemetria["tanque_ativos"]["dosador_naclo_dm3"] = tanqueAtivos.isAtuadorLigado(4);
  telemetria["tanque_ativos"]["solenoide_hvk1"] = tanqueAtivos.isAtuadorLigado(5);
  telemetria["tanque_ativos"]["solenoide_hvk2"] = tanqueAtivos.isAtuadorLigado(6);
  telemetria["tanque_ativos"]["temperatura"] = tanqueAtivos.getTemperatura();
  telemetria["tanque_ativos"]["turbidez"] = tanqueAtivos.getTurbidez();
  telemetria["tanque_ativos"]["ph"] = tanqueAtivos.getPH();

  // Tanque de efluentes
  telemetria["tanque_efluentes"]["nivel_alto"] = tanqueEfluentes.getEstadoNivelAlto();
  telemetria["tanque_efluentes"]["nivel_baixo"] = tanqueEfluentes.getEstadoNivelBaixo();
  telemetria["tanque_efluentes"]["bomba_pm3"] = tanqueEfluentes.isAtuadorLigado(0);

  // Tanque final
  telemetria["tanque_final"]["nivel_alto"] = tanqueFinal.getEstadoNivelAlto();
  telemetria["tanque_final"]["nivel_baixo"] = tanqueFinal.getEstadoNivelBaixo();
  telemetria["tanque_final"]["mexedor_rm2"] = tanqueFinal.isAtuadorLigado(0);
  telemetria["tanque_final"]["lampada_uv"] = tanqueFinal.isAtuadorLigado(1);
  telemetria["tanque_final"]["temperatura"] = tanqueFinal.getTemperatura();
  telemetria["tanque_final"]["turbidez"] = tanqueFinal.getTurbidez();
  telemetria["tanque_final"]["ph"] = tanqueFinal.getPH();

  serializeJson(telemetria, bufferTelemetria);
}

/*******************************************************************************************************************************/
/***** LOOP PRINCIPAL **********************************************************************************************************/
/*******************************************************************************************************************************/
void loop() {
  // dt = millis();
  /******************************************/
  /***** Tratamento de comandos *************/
  /******************************************/

  // Processa os comandos recebidos do broker MQTT
  // Verifica se existe alguma mensagem recebida do broker MQTT e processa a mensagem
  if (mqtt.handle()) {
    mqtt.getMensagem(mensagem, 128);
    //Serial.println(strcmp(mensagem, "alerta"));
    // Alterando o modo de operação de acordo com a mensagem recebida do broker MQTT
    if (strcmp(mensagem, "MANUAL") == 0) {
      Serial.println("Alterando para o modo manual");
      processo.modoOperacao = Processo::ModoOperacao::Manual;
    } else if (strcmp(mensagem, "AUTOMATICO") == 0) {
      Serial.println("Alterando para o modo automático");
      processo.modoOperacao = Processo::ModoOperacao::Automatico;
    } else if (strcmp(mensagem, "EMERGENCIA") == 0) {
      Serial.println("Alterando para o modo emergência");
      processo.modoOperacao = Processo::ModoOperacao::Emergencia;
    } else if (strcmp(mensagem, "alerta") == 47) {
      mqtt.desativarAlerta(atoi(strchr(mensagem, '/') + 1));
    }

    /**
     * Manual
     * O modo de Manual vai apenas interpretar os comandos enviádos pelo supervisório
     */
    if (processo.modoOperacao == Processo::ModoOperacao::Manual) {
      // TODO: Garantir que o processo continue dependendo do comando. Ex: caso o comando seja ligar PM2 com SV2 ligado etapa vai ser esvaziando tanque Ativos.
      // Isso vai ser dificil de testar, provávelmente para o protótipo seja mais interessante continuar de onde estava antes do modo manual

      if (strcmp(mensagem, "PM1") == 0) {                              // Caso comando PM1
        if (tanqueArmazenamento.isAtuadorLigado(0)) {                  // Se a bomba PM1 estiver ligada, indica que o comando é para desligar a bomba
          tanqueArmazenamento.desligaAtuador(0);                       // Desliga a bomba
          processo.etapaAtual = Processo::Etapa::Inicial;              // Atualiza o controle do processo para voltar ao inicio caso o modo de operação volte a automatico
        } else {                                                       // Caso esteja desligada, indica que o comando é para ligar
          if (tanqueArmazenamento.isNivelBaixo()) {                    // Se tanque inicial em nível baixo
            mqtt.ativarAlerta(16);                                     // Envia alerta
          } else if (tanqueAtivos.isNivelAlto()) {                     // Caso Tanque de ativos em nível alto
            mqtt.ativarAlerta(17);                                     // Envia alerta
          } else {                                                     // Caso condições permitam
            tanqueArmazenamento.ligaAtuador(0);                        // Aciona bomba PM1
            processo.etapaAtual = Processo::Etapa::EsvaziandoTqArmaz;  // Atualiza o controle da etapa para que o processo retorne de onde parou caso o modo de operação volte a automático
          }
        }
      } else if (strcmp(mensagem, "PM2") == 0) {                         // Caso comando PM2
        if (tanqueAtivos.isAtuadorLigado(0)) {                           // Verifica se a bomba está ligada, indicando que o comando é para desligar
          tanqueAtivos.desligaAtuador(0);                                // Desliga bomba PM2
          if (tanqueAtivos.isAtuadorLigado(5)) {                         // Se a solenoide SV1 estiver ligada, indicando que o processo está esvaziando o tanque de ativos para o tanque de efluentes
            tanqueAtivos.desligaAtuador(5);                              // Desliga solenoide SV1
            processo.etapaAtual = Processo::Etapa::RemovendoSedimentos;  // Atualiza o controle da etapa para que o processo retorne de onde parou caso o modo de operação volte a automático
          }

          if (tanqueAtivos.isAtuadorLigado(6)) {                  // Se a solenoide SV2 estiver ligada, indicando que o processo está esvaziando o tanque de ativos para o tanque final
            tanqueAtivos.desligaAtuador(6);                       // Desliga solenoide SV2
            processo.etapaAtual = Processo::Etapa::TratamentoUv;  // Atualiza o controle da etapa para que o processo retorne de onde parou caso o modo de operação volte a automático
          }

        } else {                                                                            // Caso a bomba esteja desligada, indicando que é para ligar
          if (!tanqueAtivos.isAtuadorLigado(5) && !tanqueAtivos.isAtuadorLigado(6)) {       // Se ambas solenoides estão desligadas, indicando que não há fluxo de água para nenhum dos tanques
            mqtt.ativarAlerta(18);                                                          // Envia alerta
          } else if (tanqueAtivos.isAtuadorLigado(5) && tanqueAtivos.isAtuadorLigado(6)) {  // Se ambas solenoides estão ligadas, indicando que há fluxo de água para ambos os tanques
            mqtt.ativarAlerta(19);                                                          // Envia alerta
          } else if (tanqueAtivos.isAtuadorLigado(5)) {                                     // Se solenoide SV1 esteja ligada, indica que o processo está esvaziando o tanque de ativos para o tanque de efluentes
            if (tanqueEfluentes.isNivelAlto()) {                                            // E tanque final esteja em nível alto
              mqtt.ativarAlerta(20);                                                        // Envia alerta
            } else {                                                                        // Caso as condições permitam
              tanqueAtivos.ligaAtuador(0);                                                  // Liga bomba PM2
              processo.etapaAtual = Processo::Etapa::RemovendoSedimentos;                   // Atualiza o controle da etapa para que o processo retorne de onde parou caso o modo de operação volte a automático
            }
          } else if (tanqueAtivos.isAtuadorLigado(6)) {                       // Se solenoide SV2 esteja ligada, indica que o processo está esvaziando o tanque de ativos para o tanque final
            if (tanqueFinal.isNivelAlto()) {                                  // Se tanque final esteja em nível alto
              mqtt.ativarAlerta(21);                                          // Envia alerta
            } else {                                                          // Caso as condições permitam
              tanqueAtivos.ligaAtuador(0);                                    // Liga bomba PM2
              processo.etapaAtual = Processo::Etapa::EsvaziandoTanqueAtivos;  // Atualiza o controle da etapa para que o processo retorne de onde parou caso o modo de operação volte a automático
            }
          }
        }
      } else if (strcmp(mensagem, "PM3") == 0) {        // Caso comando PM3
        if (tanqueEfluentes.isAtuadorLigado(0)) {       // Se a bomba PM3 estiver ligada, indica que o comando é para desligar a bomba
          tanqueEfluentes.desligaAtuador(0);            // Desliga a bomba
        } else {                                        // Caso PM3 desligada
          if (tanqueArmazenamento.isNivelAlto()) {      // Caso Tanque de armazenamento em nível alto
            mqtt.ativarAlerta(22);                      // Envia alerta
          } else if (tanqueEfluentes.isNivelBaixo()) {  // Caso tanque de efluentes em nível baixo
            mqtt.ativarAlerta(23);                      // Envia alerta
          } else {                                      // Caso condições permitam
            tanqueEfluentes.ligaAtuador(0);             // Liga PM3
          }
        }
      } else if (strcmp(mensagem, "SV1") == 0) {  // Caso comando SV1
        if (tanqueAtivos.isAtuadorLigado(5)) {    // Se a solenoide SV1 estiver ligada, indica que o comando é para desligar a mesma
          tanqueAtivos.desligaAtuador(5);         // Desliga a solenoide de controle de fluxo para o tanque de efluentes
        } else {                                  // Caso contrario
          if (tanqueAtivos.isAtuadorLigado(6)) {  // Se a solenoide SV2 estiver ligada
            mqtt.ativarAlerta(24);                // Envia alerta
          } else {                                // Caso nenhuma solenoide esteja acionada
            tanqueAtivos.ligaAtuador(5);          // Liga a solenoide de controle de fluxo para o tanque de efluentes
          }
        }
      } else if (strcmp(mensagem, "SV2") == 0) {  // Caso comando SV2
        if (tanqueAtivos.isAtuadorLigado(6)) {    // Se a solenoide SV2 estiver ligada, indica que o comando é para desligar a mesma
          tanqueAtivos.desligaAtuador(6);         // Desliga a solenoide de controle de fluxo para o tanque final
        } else {                                  // Caso contrario
          if (tanqueAtivos.isAtuadorLigado(5)) {  // Se a solenoide SV1 estiver ligada
            mqtt.ativarAlerta(25);                // Envia alerta
          } else {                                // Caso nenhuma solenoide esteja acionada
            tanqueAtivos.ligaAtuador(6);          // Liga a solenoide de controle de fluxo para o tanque final
          }
        }
      } else if (strcmp(mensagem, "RM1") == 0) {  // Caso comando RM1
        tanqueAtivos.comutaAtuador(1);            // Comuta o estado do misturador do tanque de ativos
      } else if (strcmp(mensagem, "RM2") == 0) {  // Caso comando RM2
        tanqueFinal.comutaAtuador(0);             // Comuta o estado do misturador do tanque de ativos
      } else if (strcmp(mensagem, "DM1") == 0) {  // Caso comando DM1
        tanqueAtivos.comutaAtuador(2);            // Comuta o estado do dosador de coagulante
        processo.etapaAtual = Processo::Etapa::VerificandoPh;
      } else if (strcmp(mensagem, "DM2") == 0) {  // Caso comando DM2
        tanqueAtivos.comutaAtuador(3);            // Comuta o estado do dosador de alcalinizante
        processo.etapaAtual = Processo::Etapa::VerificandoPh;
      } else if (strcmp(mensagem, "DM3") == 0) {  // Caso comando DM3
        tanqueAtivos.comutaAtuador(4);            // Comuta o estado do dosador de sanitizante
        processo.etapaAtual = Processo::Etapa::VerificandoPh;
      } else if (strcmp(mensagem, "H1") == 0) {  // Caso comando H1
        tanqueFinal.comutaAtuador(1);            // Comuta o estado da luz UV do tanque final
      }
    }
  }

  /******************************************/
  /***** Envio de mensagens *****************/
  /******************************************/

  // Envia as informações do processo para o broker MQTT
  if (((millis() - dtUpdate) >= tempoUpdate) && mqtt.isConnected()) {
    // TODO: Alterar lógica dos alertas para enviar apénas o código do alerta. Desconsiderar o envio caso o alerta esteja em aberto
    /******************************************/
    /***** Alertas ****************************/
    /******************************************/
    // Chaves de nível //
    if (tanqueArmazenamento.isFalhaSensores()) {
      mqtt.ativarAlerta(1);
    }
    if (tanqueAtivos.isFalhaSensores()) {
      mqtt.ativarAlerta(2);
    }
    if (tanqueFinal.isFalhaSensores()) {
      mqtt.ativarAlerta(3);
    }
    if (tanqueEfluentes.isFalhaSensores()) {
      mqtt.ativarAlerta(4);
    }

    // pH //
    float tempFloat = tanqueAtivos.getPH();
    if (tempFloat <= 4.0f) {
      mqtt.ativarAlerta(5);
    }
    if (tempFloat >= 8.0f) {
      mqtt.ativarAlerta(6);
    }

    tempFloat = tanqueFinal.getPH();
    if (tempFloat <= 4.0f) {
      mqtt.ativarAlerta(7);
    }
    if (tempFloat >= 8.0f) {
      mqtt.ativarAlerta(8);
    }

    // Turbidez //
    if (tanqueFinal.getTurbidez() >= 5.0f) {
      mqtt.ativarAlerta(9);
    }

    // Temperatura //
    tempFloat = tempArmaz.getTemperatura();
    if (tempArmaz.getTemperatura() <= 15.0f) {
      mqtt.ativarAlerta(10);
    }
    if (tempFloat >= 35.0f) {
      mqtt.ativarAlerta(11);
    }

    tempFloat = tanqueAtivos.getTemperatura();
    if (tempFloat <= 15.0f) {
      mqtt.ativarAlerta(12);
    }
    if (tempFloat >= 35.0f) {
      mqtt.ativarAlerta(13);
    }

    tempFloat = tanqueFinal.getTemperatura();
    if (tempFloat <= 15.0f) {
      mqtt.ativarAlerta(14);
    }
    if (tempFloat >= 35.0f) {
      mqtt.ativarAlerta(15);
    }

    atualizaTelemetria();                               // Atualiza os dados do processo no objeto JSON e no buffer para envio
    mqtt.publish(TOPICO_TELEMETRIA, bufferTelemetria);  // Envia a telemetria atualizada para o broker MQTT no tópico definido
    mqtt.publicarMensagensAlerta();                     // Publica mensagens de alerta acumuladas no buffer, caso existam
    dtUpdate = millis();                                // Atualiza o temporizador de atualização da telemetria para o próximo envio
  }

  /******************************************/
  /***** Modos de operação ******************/
  /******************************************/

  /**
   * Emergencia
   * O modo de Emergencia interrompe todo o processo.
   * Desligando todos os atuadores e garante que o processo reiniciará caso velte ao normal
   */
  if (processo.modoOperacao == Processo::ModoOperacao::Emergencia) {
    tanqueArmazenamento.desligaAtuador(0);

    tanqueAtivos.desligaAtuador(0);
    tanqueAtivos.desligaAtuador(1);
    tanqueAtivos.desligaAtuador(2);
    tanqueAtivos.desligaAtuador(3);
    tanqueAtivos.desligaAtuador(4);
    tanqueAtivos.desligaAtuador(5);
    tanqueAtivos.desligaAtuador(6);

    tanqueEfluentes.desligaAtuador(0);

    tanqueFinal.desligaAtuador(0);
    tanqueFinal.desligaAtuador(1);

    processo.etapaAtual = processo.controleEtapa = Processo::Etapa::Inicial;
    return;
  }

  /******************************************/
  /***** Condições sem etapa específica *****/
  /******************************************/

  // IMPORTANT: Essas condições vão desligar as bombas imediatamente quando dependendo do nível dos tanques
  // a etapa de verificação de nível fica obsoleta nesse caso
  // Tanque armazenamento
  if (tanqueArmazenamento.isNivelBaixo() || tanqueAtivos.isNivelAlto()) {  // (TAR.1) Se armazenamento nível baixo ou ativos nível alto
    tanqueArmazenamento.desligaAtuador(0);                                 // Desliga bomba PM1
  }

  // Tanque ativos
  if (tanqueAtivos.isNivelBaixo()) {
    tanqueAtivos.desligaAtuador(1);  // Desliga mexedor
    tanqueAtivos.desligaAtuador(0);  // Desliga bomba PM2
  }

  // Tanque Final
  if (tanqueFinal.isNivelBaixo()) {  // (TF.1) Se tanque final nível baixo
    tanqueFinal.desligaAtuador(0);   // Desliga mexedor
    tanqueFinal.desligaAtuador(1);   // Desliga lampada UV
  }

  // Tanque Efluentes
  if (tanqueEfluentes.isNivelAlto() && !tanqueArmazenamento.isNivelAlto()) {         // (TE.1) Se tanque de efluentes nível alto e tanque de armazenamento não nível alto
    tanqueEfluentes.ligaAtuador(0);                                                  // Aciona PM3
  } else if (tanqueEfluentes.isNivelBaixo() || tanqueArmazenamento.isNivelAlto()) {  // (TE.2) Se tanque de armazenamento nível alto || efluentes baixo
    tanqueEfluentes.desligaAtuador(0);                                               // Desaciona PM3
  }

  /**********************************/
  /***** Processo de tratamento *****/
  /**********************************/

  /* Tanque de Armazenamento */
  // Inicio -> Verificação de Nível -> Esvaziando Armazenamento //
  // 1.1 - Se nível alto -------------------------------------------------------------------------> VerificNivelAltoArmaz
  // 1.2 - Por 1s, liga bomba PM1 ----------------------------------------------------------------> EsvaziandoTqArmaz
  // 1.3 - Se nível baixo || Tanque ativos nível alto --------------------------------------------> VerificTransferenciaAtivos

  // Verificando Tranferencia //
  // 2.1 - Por 1s, Desliga PM1
  // 2.2 - Se tanque ativos !nível alto ----------------------------------------------------------> Inicial
  // 2.3 - Else, liga RM1
  // 2.4 - Atualiza temporizador de dosagem ------------------------------------------------------> DosandoCoagulante

  /* Tanque Ativos */
  // Dosagem Coagulante //
  // 3.1 - Liga DM1
  // 3.2 - Após Tempo definido de pausa na dosagem
  // 3.3 - Desliga DM1R
  // 3.4 - Atualiza controle processo para Dosagem de coagulante
  // 3.5 - Se dtDosagemCoagulante >= tempoDosagemCoagulante
  // 3.6 - Armazena controle de etapa como PreparaCoagulacao
  // 3.7 - Atualiza temporizador de Homogeneizacao -----------------------------------------------> VerificaPH

  // Verificação de pH //
  // 4.1 - Se tempo homogeneização concluido
  // 4.2 - Se pH < que (6 - histerese), atualiza temporizador de dosagem de alcalinizante
  // 4.3 - Muda controle de processo para DosandoAlcalinizante
  // 4.4 - Se pH >= (alvoPH - histerese) || pH <= (alvoPH + histerese)
  // 4.5 - Se controle de processo for PreparaCoagulação
  // 4.6 - Atualiza temporizador de coagulação
  // 4.7 - Se ph > phalvo + histerese, muda controle do processo para Dosando coagulante
  // 4.8 - Muda etapa atual para controle do processo --------------------------------------------> controleEtapa

  // Dosagem Alcalinizante //
  // 5.1 - Ligar Dosador DM2
  // 5.2 - Se passou tempo de dosagem
  // 5.3 - Desliga DM2
  // 5.4 - Atualiza controle de processo para Dosagmem de coagulante -----------------------------> VerificandoPh

  // Preparo para Coagulação //
  // 6.1 - Se tempo de coagulação >= tempo de homogeneização
  // 6.2 - Atualiza tempo de coagulação
  // 6.3 - Desliga RM1
  // 6.4 - Inicia a coagulação -------------------------------------------------------------------> Decantacao

  // Coagulação //
  // 7.1 - Se tempo de coagulação for atingido
  // 7.2 - Se ntu alvo atingido, Aciona SV1
  // 7.3 - Atualiza o temporizador de descarga de efluentes --------------------------------------> PreparandoLibercaoEfluentes

  // Descarga de Efluentes //
  // 7.5 - Se tempo de verificação, Acionando PM2, atualiza temporizador de descarga -------------> LiberandoEfluentes

  // Liberando Efluentes //
  // 7.6 - Manter PM2 até tempo de descarga
  // 7.7 - Desaciona SV1, PM2 e
  // 7.8 - Atualiza o temporizador de dosagem de sanitizante e Iniciar agitação (RM1) ------------> DosandoSanitizante

  // Dosagem de Sanitizante //
  // 8.1 - Se o tempo de dosagem atingir tempo de dosagem total
  // 8.2 - Desliga Dosador DM3
  // 8.3 - Atualiza temporizador de transferencia para tanque final
  // 8.4 - Aciona SV2 ---------------------------------------------------------------------------> PreparandoEsvaziamentoAtivos

  // Preparando Transferencia para Tanque Final //
  // 8.5 - Se tempo de verificação alcancado
  // 8.6 - Se tanque final nivel baixo, Aciona PM2 -----------------------------------------------> EsvaziandoTanqueAtivos

  // Transferindo para tanque final //
  // 9.1 - Se tanque final nível alto || tanque ativos nível baixo
  // 9.2 - Atualiza temporizador tanque final ----------------------------------------------------> VerificTransferenciaFinal
  // 9.3 - Se tanque final nível alto || tanque ativos nível baixo
  // 9.4 - Se o tempo de verificação alcançado, deliga PM2, deliga SV2
  // 9.5 - Liga RM2, liga H1 UV e atualiza temporizador de tratamento UV -------------------------> TratamentoUv

  /* Tanque Final */
  // 9.6 - Se o tempo mínimo de tratamento for alcançado
  // 9.7 - Finaliza o processo -------------------------------------------------------------------> TratamentoUv

  /* Condições sem etapa */
  // Tanque de Armazenamento
  // TAR.1 - Se armazenamento nível baixo || ativos nível alto, desliga PM1

  // Tanque final //
  // TF.1 - Se tanque final nível baixo desliga H1 e RM2

  // Efluentes //
  // TE.1 - Se tanque efluentes nível alto && armazenamento não nível alto, Aciona PM3
  // TE.2 - Se Armazenamento Nível alto || Efluentes Nível baixo, Desaciona PM3

  if (processo.modoOperacao == Processo::ModoOperacao::Automatico) {
    /* ----- Processo Tanque Armazenamento ----- */
    if (processo.etapaAtual == Processo::Etapa::Inicial) {             // Tanque de armazenamento Enchendo
      if (tanqueArmazenamento.isNivelAlto()) {                         // (1.1) Se tanque de armazenamento em nível alto
        dtArmazenamento = millis();                                    // Atualiza variável auxiliar com o tempo atual em milissegundos
        processo.etapaAtual = Processo::Etapa::VerificNivelAltoArmaz;  // (-> 1.1) Muda a etapa do processo para "VerificNivelAltoArmaz"
      }
    }

    if (processo.etapaAtual == Processo::Etapa::VerificNivelAltoArmaz) {  // Verificando nível do tanque de armazenamento
      if (tanqueArmazenamento.isNivelAlto()) {                            // (1.1) Se nível alto tanque de armazenamento
        if (millis() - dtArmazenamento >= tempoDeVerificacao) {           // (1.2) Verifica se o tempo de verificação foi atingido desde o último acionamento
          tanqueArmazenamento.ligaAtuador(0);                             // (1.2)Liga a bomba PM1 para transferir água para o tanque de ativos
          processo.etapaAtual = Processo::Etapa::EsvaziandoTqArmaz;       // (-> 1.2) Muda a etapa do processo para "EsvaziandoTqArmaz"
        }
      }
    }

    if (processo.etapaAtual == Processo::Etapa::EsvaziandoTqArmaz) {           // Esvaziando o tanque de armazenamento
      if (tanqueArmazenamento.isNivelBaixo() || tanqueAtivos.isNivelAlto()) {  // (1.3) Se nível baixo armazenamento ou nivel alto tanque de ativos
        dtArmazenamento = millis();                                            // Atualiza o tempo atual em milissegundos para o tanque de ativos
        processo.etapaAtual = Processo::Etapa::VerificTransferenciaAtivos;     // (-> 1.3)Muda a etapa do processo para "VerificTransferenciaAtivos"
      }
    }

    if (processo.etapaAtual == Processo::Etapa::VerificTransferenciaAtivos) {  // Verificando nível do tanque de ativos
      if (tanqueArmazenamento.isNivelBaixo() || tanqueAtivos.isNivelAlto()) {  // (1.3) Se nível baixo armazenamento ou nivel alto tanque de ativos
        if (millis() - dtArmazenamento >= tempoDeVerificacao) {                // (2.1) Verifica se o tempo de verificação foi atingido desde o último acionamento
          tanqueArmazenamento.desligaAtuador(0);                               // (2.1) Desliga a bomba PM1 para interromper a transferência de água para o tanque de ativos

          if (!tanqueAtivos.isNivelAlto()) {                           // (2.2) Se o tanque de ativos ainda não estiver cheio
            processo.etapaAtual = Processo::Etapa::Inicial;            // (-> 2.2)Muda a etapa do processo para "Inicial"
          } else {                                                     // (2.3) Caso esteja cheio
            tanqueAtivos.ligaAtuador(1);                               // (2.3) Liga o misturador RM1 para iniciar a agitação da água no tanque de ativos
            dtDosagemCoagulante = millis();                            // (2.4) Atualiza o temporizador de dozagem do coagulante
            dtPausaDosagem = millis();                                 // (2.4) Atualiza o temporizador de pausa da dosagem do coagulante
            processo.etapaAtual = Processo::Etapa::DosandoCoagulante;  // (-> 2.4) Muda a etapa do processo para "DosandoCoagulante"
          }
        }
      }
    }

    /* ----- Processo Tanque Ativos ----- */
    if (processo.etapaAtual == Processo::Etapa::DosandoCoagulante) {  // Dosando o coagulante e verificando parâmetros
      tanqueAtivos.ligaAtuador(2);                                    // (3.1) Liga o dosador do coagulante

      // Se o resto do (tempo atual menos a ultima atualização) dividido pelo tempo de pausa for == a 1, significa que se passou o tempo definido desde a ultima verificação
      if ((millis() - dtPausaDosagem) >= tempoPausaDosagem) {         // (3.2) Após dosar uma pequena quantidade de coagulante
        tanqueAtivos.desligaAtuador(2);                               // (3.3) Desliga o dosador de coagulante
        processo.controleEtapa = Processo::Etapa::DosandoCoagulante;  // (3.4) Atualiza o controle do processo como DosandoCoagulante garantindo o retorno para a dosagem de coagulante caso necessário

        if ((millis() - dtDosagemCoagulante) >= tempoDosagemCoagulante) {  // (3.5) Se o tempo total de dosagem do coagulante for alcançado
          processo.controleEtapa = Processo::Etapa::PreparaCoagulacao;     // (3.6) Atualiza controle de etapa do processo
        }

        dtHomogeneizacao = millis();                           // (3.7) Atualiza temporizador de Homogeneizacao
        dtPausaDosagem = millis();                             // (3.7) Atualiza temporizador de pausa da dosagem do coagulante
        processo.etapaAtual = Processo::Etapa::VerificandoPh;  // (-> 3.7) Muda a etapa do processo para "VerificaPH"
      }
    }

    if (processo.etapaAtual == Processo::Etapa::VerificandoPh) {  // Verificando o pH após a dosagem de quimicos
      if (millis() - dtHomogeneizacao >= tempoHomogeneizacao) {   // (4.1) Se o tempo de homogeneização concluido
        float ph = tanqueAtivos.getPH();

        if (ph < (6.0f + histerese)) {                                         // (4.2) Se o ph da agua estiver muito ácido
          dtDosagemAlcalinizante = millis();                                   // (4.2) Atualiza o temporizador de dosagem do alcalinizante
          processo.controleEtapa = Processo::Etapa::DosandoAlcalinizante;      // (4.3) Muda o controle de etapa do processo para "DosandoAlcalinizante"
        } else if (ph >= alvoPhCoagulante - histerese &&                       // (4.4) Se ph estiver acima do parâmetro de pH - histerese
                   ph <= alvoPhCoagulante + histerese) {                       // (4.4) Ou abaixo do parâmetro + histerese
          if (processo.controleEtapa == Processo::Etapa::PreparaCoagulacao) {  // (4.5) Se o controle do processo indicar que a proxima etapa é a coagulação
            dtCoagulacao = millis();                                           // (4.6) Atualiza temporizador de coagulação
          }
        } else if (ph > alvoPhCoagulante + histerese) {                 // (4.7) Se pH alcalino
          dtPausaDosagem = millis();                                    // (4.7) Atualiza temporizador de dosagem do coagulante
          processo.controleEtapa = Processo::Etapa::DosandoCoagulante;  // (4.7) Atualiza controle do processo indicando a proxima etapa como DosandoCoagulante
        }

        processo.etapaAtual = processo.controleEtapa;  // (-> 4.8) Muda a etapa do processo para "processo.controleEtapa"
      }
    }

    if (processo.etapaAtual == Processo::Etapa::DosandoAlcalinizante) {  // Dosando o alcalinizante e verificando parâmetros
      tanqueAtivos.ligaAtuador(3);                                       // (5.1) Liga o dosador do alcalinizante

      if (((millis() - dtDosagemAlcalinizante) >= tempoDosagemAlcalinizante)) {  // (5.2) Após dosar uma pequena quantidade de alcalinizante
        tanqueAtivos.desligaAtuador(3);                                          // (5.3) Desliga DM2
        processo.controleEtapa = Processo::Etapa::DosandoCoagulante;             // (5.4) Atualiza o controle do processo para voltar a etapa de dosagem de coagulante após verificar pH
        processo.etapaAtual = Processo::Etapa::VerificandoPh;                    // (-> 5.4) Muda a etapa do processo para VerificandoPh
      }
    }

    /* Preparo para Coagulação */
    if (processo.etapaAtual == Processo::Etapa::PreparaCoagulacao) {  // Inicio da coagulação
      if (millis() - dtCoagulacao >= tempoHomogeneizacao) {           // (6.1) Mantém o agitador ligado pelo tempo definido para homogeneização
        dtCoagulacao = millis();                                      // (6.2) Atualiza o tempo atual em milissegundos
        tanqueAtivos.desligaAtuador(1);                               // (6.3) Desliga o mexedor RM1 para aguardar a coagulação dos sólidos
        processo.etapaAtual = Processo::Etapa::Decantacao;            // (-> 6.4) Muda a etapa do processo para "Coagulação"
      }
    }

    /* Coagulação */
    if (processo.etapaAtual == Processo::Etapa::Decantacao) {                 // Aguardando coagulação dos sólidos
      if (millis() - dtCoagulacao >= tempoCoagulacao) {                       // (7.1) Aguarda o tempo determinado de coagulação antes de seguir com o processo
        tanqueAtivos.ligaAtuador(5);                                          // (7.2) Aciona SV1
        dtDescargaEfluentes = millis();                                       // (7.3) Atualiza o temporizador de descarga de efluentes
        processo.etapaAtual = Processo::Etapa::PreparandoLibercaoSedimentos;  // (-> 7.3) Muda a etapa do processo para "PreparandoLibercaoSedimentos
      }
    }

    /* Efluentes */
    if (processo.etapaAtual == Processo::Etapa::PreparandoLibercaoSedimentos) {
      if (millis() - dtDescargaEfluentes >= tempoDeVerificacao) {    // (7.6) Aguarda tempo de transferencia
        tanqueAtivos.ligaAtuador(0);                                 // (7.5) Liga bomba de transferencia
        dtDescargaEfluentes = millis();                              // (7.5) Atualiza o tempo atual em milissegundos
        processo.etapaAtual = Processo::Etapa::RemovendoSedimentos;  // (-> 7.5) Muda a etapa do processo para "RemovendoSedimentos
      }
    }

    if (processo.etapaAtual == Processo::Etapa::RemovendoSedimentos) {
      if (millis() - dtDescargaEfluentes >= tempoDescargaEfluentes) {  // (7.6) Aguarda tempo de transferencia
        tanqueAtivos.desligaAtuador(0);                                // (7.7) Desliga bomba
        tanqueAtivos.desligaAtuador(5);                                // (7.7) Desliga solenoide
        tanqueAtivos.ligaAtuador(1);                                   // (7.7) Liga Agitação
        tanqueAtivos.ligaAtuador(4);                                   // (7.7) Liga dosador de sanitizante
        dtDosagemSanitizante = millis();                               // (7.8) Atualiza temporizador de dosagem de sanitizante
        processo.etapaAtual = Processo::Etapa::DosandoSanitizante;     // (-> 7.8) Muda etapa do processo para "DosandoSanitizante"
      }
    }

    /* Sanitização */
    if (processo.etapaAtual == Processo::Etapa::DosandoSanitizante) {         // Dosando o sanitizante e verificando parâmetros
      if ((millis() - dtDosagemSanitizante) >= tempoDosagemSanitizante) {     // (8.1) Se o tempo total de dosagem do sanitizante for alcançado
        tanqueAtivos.desligaAtuador(4);                                       // (8.2) Desliga o dosador de sanitizante
        dtAtivos = millis();                                                  // (8.3) Atualiza temporizador
        tanqueAtivos.ligaAtuador(6);                                          // (8.4) Aciona Solenoide de transferencia para o tanque final
        processo.etapaAtual = Processo::Etapa::PreparandoEsvaziamentoAtivos;  // (-> 8.4) Atualiza etapa do processo
      }
    }

    /* Transferencia para tanque final */
    if (processo.etapaAtual == Processo::Etapa::PreparandoEsvaziamentoAtivos) {  // Preparando para transferir água tratada para o tanque final
      if (millis() - dtAtivos >= tempoDeVerificacao) {                           // (8.5) Se tempo de verificação atingido
        if (tanqueFinal.isNivelBaixo()) {                                        // (8.6) Se tanque final em nível baixo
          tanqueAtivos.ligaAtuador(0);                                           // (8.6) Liga bomba de transferencia
          processo.etapaAtual = Processo::Etapa::EsvaziandoTanqueAtivos;         // (-> 8.6) Muda etapa do processo para EsvaziandoTanqueAtivos
        }
      }
    }

    if (processo.etapaAtual == Processo::Etapa::EsvaziandoTanqueAtivos) {  // Transferindo água tratada
      if (tanqueFinal.isNivelAlto() || tanqueAtivos.isNivelBaixo()) {      // (9.1) Se tanque final em nível baixo
        dtFinal = millis();                                                // (9.2) Atualiza o temporizador
        processo.etapaAtual = Processo::Etapa::VerificTransferenciaFinal;  // (-> 9.2) Muda etapa do processo para VerificTransferenciaFinal
      }
    }

    /* ----- Processo Tanque Final ----- */
    if (processo.etapaAtual == Processo::Etapa::VerificTransferenciaFinal) {  // Preparando para transferir água tratada para o tanque final
      if (tanqueFinal.isNivelAlto() || tanqueAtivos.isNivelBaixo()) {         // (9.3) Se nível do tanque de ativos baixo ou final alto
        if (millis() - dtFinal >= tempoDeVerificacao) {                       // (9.4) Se tempo de verificação atingido
          tanqueAtivos.desligaAtuador(0);                                     // (9.4) Desliga bomba de transferencia PM2
          tanqueAtivos.desligaAtuador(6);                                     // (9.4) Desliga solenoide SV2
          tanqueFinal.ligaAtuador(0);                                         // (9.5) Liga mexedor RM2
          tanqueFinal.ligaAtuador(1);                                         // (9.5) Liga lampada UV
          dtFinal = millis();                                                 // (9.5) Atualiza o temporizador do tanque final
          processo.etapaAtual = Processo::Etapa::TratamentoUv;                // (-> 9.5) Muda etapa do processo para TratamentoUv
        }
      }
    }

    if (processo.etapaAtual == Processo::Etapa::TratamentoUv) {  // Tratamento UV
      if (millis() - dtFinal >= tempoTratamentoUv) {             // (9.6) Se tempo de tratamento mínimo atingido
        processo.etapaAtual = Processo::Etapa::Finalizado;       // (-> 9.7) Muda etapa do processo para Finalizado
      }
    }
  }

  // Serial.print("Tempo total dentro de loop(): ");
  // Serial.print(millis() - dt);
  // Serial.println("ms");
  //Serial.println("");
}
