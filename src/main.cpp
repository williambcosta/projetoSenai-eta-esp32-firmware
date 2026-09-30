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
#define MQTT_PORTA 8883           // Porta padrão para conexões MQTT seguras (TLS/SSL)
#define MQTT_SENHA "esp12345"     // Senha do usuário MQTT
#define MQTT_USUARIO "espclient"  // Usuário MQTT
#define WIFI_SENHA "#Ws120912"    // Senha da rede Wi-Fi
#define WIFI_SSID "Willian"       // SSID da rede Wi-Fi

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
#define PIN_DOSADOR_FeCl3_DM1 "0.7"  // Pino do dosador de cloreto
#define PIN_DOSADOR_CaCO3_DM2 "1.0"  // Pino do dosador de carbonato
#define PIN_DOSADOR_NaClO_DM3 "1.1"  // Pino do dosador de hipoclorito

// Lâmpadas
#define PIN_LAMPADA_UV "1.2"  // Pino da lâmpada UV

// Enumeração para representar as etapas do processo de tratamento de água
enum class EtapaProcesso {
  Inicial,                         // Etapa inicial do processo, indica que o tanque de armazenamento esta enchendo
  VerificNivelAltoArmaz,           // Verificação do nível alto do tanque de armazenamento de água bruta
  EsvaziandoTqArmaz,               // Esvaziamento do tanque de armazenamento
  VerificTransferenciaAtivos,      // Verificação do nível do tanque de ativos
  DosandoCoagulante,               // Dosagem do coagulante
  VerificandoPhAposCoagulante,     // Verificação de pH após dosagem de coagulante
  DosandoAlcalinizante,            // Dosagem de alcalinizante
  VerificandoPhAposAlcalinizante,  // Verificação de pH após dosagem de alcalinizante
  DosandoSanitizante,              // Dosagem de sanitizante
  VerificandoPhAposSanitizante,    // Verificação de pH após dosagem de sanitizante
  Homogeneizando,                  // Homogeneizando mistura de químicos
  PreparaCoagulacao,               // Continua a homogeinização antes da coagulação e decantação dos flocos
  Coagulacao,                      // Aguardando a coagulação e decantação dos flocos
  PreparandoLibercaoEfluentes,     // Preparando para liberar o decantado para o tanque de efluentes
  LiberandoEfluentes,              // Liberando o decantado para o tanque de efluentes
  VerificNivelBaixoAtivos,         // Verificando nível do tanque de ativos
  RemovendoSedimentos,             // Removendo sedimentos do tanque de ativos
  EsvaziandoTanqueAtivos,          // Esvaziamento do tanque de ativos
  TratamentoUv,                    // Tratamento com luz UV
  Finalizado                       // Processo finalizado
};

EtapaProcesso etapaAtual = EtapaProcesso::Inicial;     // Variável para armazenar a etapa atual do processo
EtapaProcesso etapaAnterior = EtapaProcesso::Inicial;  // Variável para armazenar a etapa anterior do processo

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
std::vector<SaidaDigital> saidasAtivos = {
    // Indices
    SaidaDigital(PIN_BOMBA_PM2),          // 0 - Bomba de teansferencia do tanque de ativos para tanque final/efluentes
    SaidaDigital(PIN_RM1_ATIV),           // 1 - Mexedor
    SaidaDigital(PIN_DOSADOR_FeCl3_DM1),  // 2 - Coagulante
    SaidaDigital(PIN_DOSADOR_CaCO3_DM2),  // 3 - Alcalinizante
    SaidaDigital(PIN_DOSADOR_NaClO_DM3),  // 4 - Sanitizante
    SaidaDigital(PIN_SOL_EFLU),           // 5 - Solenoide de controle de fluxo para efluentes
    SaidaDigital(PIN_SOL_FINAL)           // 6 - Solenoide de controle de fluxo para tanque final
};

// Instancia
TanqueTratamento tanqueAtivos = TanqueTratamento(tempAtiv, phAtivos, ntuAtiv, saidasAtivos, PIN_SNA_ATIVOS, PIN_SNB_ATIVOS);

/***** TANQUE FINAL *****/
// Sensores
SensorTemperatura tempFinal = SensorTemperatura(PIN_TEMP_FINAL);                   // Temperatura
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
// Parâmetros de tempo (em milisegundos)
int tempoDeVerificacao = 1000;            // Intervalo de tempo para verificar o nível dos tanques
int tempoMistura = 5000;                  // Intervalo após dosagem de quimica para homogeinização da agua antes de nova medição de parâmetros
int tempoDosagemCoagulante = 5000;        // Tempo de dosagem do coagulante
int tempoDosagemAlcalinizante = 5000;     // Tempo de dosagem do alcalinizante
int tempoDosagemSanitizante = 5000;       // Tempo de dosagem do sanitizante
int tempoCoagulacao = 3000000;            // Tempo de espera para que a coagulação aconteça
int tempoTransferenciaEfluentes = 30000;  // Tempo que a bomba de transferencia do tanque de ativos fica ligada ao esvaziar o decantado para o tanque de efluentes

// Temporizadores
unsigned long dtDosagemCoagulante = 0;     // Tempo percorrido da contagem da dosagem de coagulante
unsigned long dtDosagemAlcalinizante = 0;  // Tempo percorrido da contagem da dosagem de alcalinizante
unsigned long dtArmazenamento = 0;         // Tempo percorrido da contagem do tanque de armazenamento
unsigned long tempoAtivos = 0;             // Tempo percorrido da contagem do tanque de ativos
unsigned long tempoFinal = 0;              // Tempo percorrido da contagem do tanque final
unsigned long dtCoagulacao = 0;            // Tempo percorrido da coagulação dos residuos do tanque de ativos

// Parâmetros das caracteristicas da água
float histerese = 0.5f;            // Histerese para verificação de ph
float alvoPhCoagulante = 7.0f;     // Alvo de pH após dosagem do Coagulante
float alvoPhAlcalinizante = 7.0f;  // Alvo de pH após dosagem do Alcalinizante
float alvoPhSanitizante = 7.0f;    // Alvo de pH após dosagem do Sanitizante
float alvoNtu = 20.0f;             // NTU alvo após coagulação

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

  /** Processo de tratamento **/
  /* Tanque de Armazenamento */
  //  1.1 .......................... Inicial - Se nível alto --------------------------------> VerificNivelAltoArmaz
  //  1.2 ............ VerificNivelAltoArmaz - Por 1s, liga bomba PM1 -----------------------> EsvaziandoTqArmaz
  //  1.3 ................ EsvaziandoTqArmaz - Se nível baixo || Tanque ativos nível alto ---> VerificTransferenciaAtivos
  //  1.4 ....... VerificTransferenciaAtivos - Por 1s, Desliga PM1
  //  1.5 ....... VerificTransferenciaAtivos - Se tanque ativos !nível alto -----------------> Inicial
  //  1.6 ....... VerificTransferenciaAtivos - Else, liga RM1
  //  1.7 ....... VerificTransferenciaAtivos - Atualiza temporizador de dosagem -------------> DosandoCoagulante

  /* Tanque Ativos */
  // Dosagem Coagulante e Alcalinizante //
  //  2.1 ................ DosandoCoagulante - Liga DM1
  //  2.2 ................ DosandoCoagulante - Após 1s
  //  2.3 ................ DosandoCoagulante - Desliga DM1R, armazena ultima etapa como DosandoCoagulante ----> VerificaPH
  //  2.4 ................ DosandoCoagulante - Se dtDosagemCoagulante >= tempoDosagemCoagulante
  //  2.5 ................ DosandoCoagulante - Se pH < que (6 - histerese)
  //  2.6 ................ DosandoCoagulante - Atualiza temporizador de dosagem de alcalinizante -------------> DosandoAlcalinizante
  //  2.7 ................ DosandoCoagulante - Se pH >= (alvoPH - histerese) || pH <= (alvoPH + histerese)
  //  2.8 ................ DosandoCoagulante - Atualiza temporizador de coagulação ---------------------------> PreparaCoagulacao
  //  2.9 ............. DosandoAlcalinizante - Ligar Dosador DM2
  // 2.10 ............. DosandoAlcalinizante - Após 1s, desliga DM2 --------------------------------> VerificandoPhAposAlcalinizante
  // 2.11 ... VerificandoPhAposAlcalinizante - Se pH >= (alvoPH - histerese) || pH <= (alvoPH + histerese) ---> DosandoCoagulante

  // Coagulação //
  // PreparaCoagulacao -

  //- Coagulação -//
  // 2.10 - Prepara a coagulação
  // 2.11 - Mantém a agitação por mais 5s
  // 2.12 - Inicia a coagulação
  // 2.14 - Pausa a agitação por 5m

  // 2.15 - Aciona HVK1, Descarrega efluentes (acionando PM2) após verificação de NTU
  // 2.16 - Manter PM2 por +/- 30s
  // 2.17 - Desaciona HVK1 e PM2
  // 2.18 - Iniciar agitação (RM1)

  // 2.19 - Calcula o tempo de dosagem do sanitizante (formula ? vai considerar o volume)
  // 2.20 - Dosar um pouco por vez e medir o ph após pausa na dosagem de ?s
  // 2.21 - Ph em nível 7 interromper dosagem de sanitizante
  // 2.22 - Mantém a agitação até PH interrupção de dosagem sanitizante
  // 2.23 - Aciona HVK2
  // 2.24 - Aciona PM2 se nível baixo
  // 2.25 - PM2 desaciona quando Armazenamento Nivel alto || Ativos Nível Baixo

  /* Tanque Final */
  // 3.1 - Se !Nível alto && nível baixo Aciona H1 e RM2
  // 3.2 - Se Nível alto Desaciona PM2
  // 3.3 - Se !Nível baixo && !Nível alto para RM2 e H1

  /* Efluentes */
  // 4.1 - Quando nível alto && (!Nível alto  && Nível baixo Armazenamento)
  // 4.2 - Aciona PM3
  // 4.3 - Se Armazenamento Nível alto || Efluentes Nível baixo
  // 4.4 - Para PM3

  /* ----- Processo Tanque Armazenamento ----- */
  if (etapaAtual == EtapaProcesso::Inicial) {             // Tanque de armazenamento Enchendo
    if (tanqueArmazenamento.isNivelAlto()) {              // (1.1) Se tanque de armazenamento em nível alto
      dtArmazenamento = millis();                         // Atualiza variável auxiliar com o tempo atual em milissegundos
      etapaAnterior = etapaAtual;                         // Atualiza a etapa anterior
      etapaAtual = EtapaProcesso::VerificNivelAltoArmaz;  // (-> 1.1) Muda a etapa do processo para "VerificNivelAltoArmaz"
    }
  }

  if (etapaAtual == EtapaProcesso::VerificNivelAltoArmaz) {    // Verificando nível do tanque de armazenamento
    if (tanqueArmazenamento.isNivelAlto()) {                   // (1.1) Se nível alto tanque de armazenamento
      if (millis() - dtArmazenamento >= tempoDeVerificacao) {  // (1.2) Verifica se se passou 1 segundo desde o último acionamento
        tanqueArmazenamento.ligaAtuador(0);                    // (1.2)Liga a bomba PM1 para transferir água para o tanque de ativos
        etapaAnterior = etapaAtual;                            // Atualiza a etapa anterior
        etapaAtual = EtapaProcesso::EsvaziandoTqArmaz;         // (-> 1.2) Muda a etapa do processo para "EsvaziandoTqArmaz"
      }
    }
  }

  if (etapaAtual == EtapaProcesso::EsvaziandoTqArmaz) {                      // Esvaziando o tanque de armazenamento
    if (tanqueArmazenamento.isNivelBaixo() || tanqueAtivos.isNivelAlto()) {  // (1.3) Se nível baixo armazenamento ou nivel alto tanque de ativos
      dtArmazenamento = millis();                                            // Atualiza o tempo atual em milissegundos para o tanque de ativos
      etapaAnterior = etapaAtual;                                            // Atualiza a etapa anterior
      etapaAtual = EtapaProcesso::VerificTransferenciaAtivos;                // (-> 1.3)Muda a etapa do processo para "VerificTransferenciaAtivos"
    }
  }

  if (etapaAtual == EtapaProcesso::VerificTransferenciaAtivos) {             // Verificando nível do tanque de ativos
    if (tanqueArmazenamento.isNivelBaixo() || tanqueAtivos.isNivelAlto()) {  // (1.3) Se nível baixo armazenamento ou nivel alto tanque de ativos
      if (millis() - dtArmazenamento >= tempoDeVerificacao) {                // (1.4) Verifica se se passou 1 segundo desde o último acionamento
        tanqueArmazenamento.desligaAtuador(0);                               // (1.4) Desliga a bomba PM1 para interromper a transferência de água para o tanque de ativos

        if (!tanqueAtivos.isNivelAlto()) {                // (1.5) Se o tanque de ativos ainda não estiver cheio
          etapaAnterior = etapaAtual;                     // Atualiza a etapa anterior
          etapaAtual = EtapaProcesso::Inicial;            // (-> 1.5)Muda a etapa do processo para "Inicial"
        } else {                                          // (1.6) Caso esteja cheio
          tanqueAtivos.ligaAtuador(1);                    // (1.6) Liga o misturador RM1 para iniciar a agitação da água no tanque de ativos
          dtDosagemCoagulante = millis();                 // (1.7) Atualiza o temporizador de dozagem do coagulante
          etapaAnterior = etapaAtual;                     // Atualiza a etapa anterior
          etapaAtual = EtapaProcesso::DosandoCoagulante;  // (-> 1.6) Muda a etapa do processo para "DosandoCoagulante"
        }
      }
    }
  }

  /* ----- Processo Tanque Ativos ----- */
  if (etapaAtual == EtapaProcesso::DosandoCoagulante) {  // Dosando o coagulante e verificando parâmetros
    tanqueAtivos.ligaAtuador(2);                         // (2.1) Liga o dosador do coagulante

    // Se o resto do (tempo atual menos a ultima atualização) dividido pelo tempo de verificação for == a 1, significa que se passou 1 s desde a ultima verificação
    if (((millis() - dtDosagemCoagulante) % tempoDeVerificacao) == 1) {     // (2.2) Após dosar uma pequena quantidade de coagulante
      tanqueAtivos.desligaAtuador(2);                                       // (2.3) Desliga o dosador de coagulante
      if ((millis() - dtDosagemCoagulante) >= tempoDosagemCoagulante) {     // (2.4) Se o tempo total de dosagem do coagulante for alcançado
        if (tanqueAtivos.getPH() < (6.0f - histerese)) {                    // (2.5) Se o ph da agua estiver muito ácido
          dtDosagemAlcalinizante = millis();                                // (2.6) Atualiza o temporizador de dosagem do alcalinizante
          etapaAnterior = etapaAtual;                                       // Atualiza a etapa anterior
          etapaAtual = EtapaProcesso::DosandoAlcalinizante;                 // (-> 2.6) Muda a etapa do processo para "DosandoAlcalinizante"
        } else if (tanqueAtivos.getPH() >= alvoPhCoagulante - histerese ||  // (2.7) Se ph estiver dentro do parâmetro definido
                   tanqueAtivos.getPH() <= alvoPhCoagulante + histerese) {  //
          dtCoagulacao = millis();                                          // (2.8) Atualiza temporizador de coagulação
          etapaAnterior = etapaAtual;                                       // Atualiza a etapa anterior
          etapaAtual = EtapaProcesso::PreparaCoagulacao;                    // (-> 2.8) Muda a etapa do processo para "PreparaCoagulacao"
        }
      }
    }

    if (etapaAtual == EtapaProcesso::DosandoAlcalinizante) {  // Dosando o alcalinizante e verificando parâmetros
      tanqueAtivos.ligaAtuador(3);                            // (2.9) Liga o dosador do alcalinizante

      if (((millis() - dtDosagemAlcalinizante) == tempoDeVerificacao)) {  // (2.10) Após dosar uma pequena quantidade de alcalinizante
        tanqueAtivos.desligaAtuador(3);                                   // (2.10) Desliga DM2
        etapaAnterior = EtapaProcesso::VerificandoPhAposAlcalinizante;    // Atualiza a etapa anterior
        etapaAtual = EtapaProcesso::Homogeneizando;                       // (-> 2.10) Muda a etapa do processo indicando a mistura do alcalinizante
        tempoDosagem = millis();                                          // Atualiza o tempo atual em milissegundos
      } else if (millis() - tempoDosagem >= tempoDosagemAlcalinizante) {  // ou caso se passe o tempo máximo de dosagem
        etapaAnterior = etapaAtual;                                       // Atualiza a etapa anterior
        etapaAtual = EtapaProcesso::VerificandoPhAposAlcalinizante;       // Muda a etapa do processo para verificar o ph
      }
    }
  }

  // Verificando o pH após a dosagem do coagulante
  if (etapaAtual == EtapaProcesso::VerificandoPhAposCoagulante) {
    // (* 2.5) Se o pH alvo foi atingido
    if (tanqueAtivos.getPH() >= alvoPhCoagulante - histerese ||
        tanqueAtivos.getPH() <= alvoPhCoagulante + histerese) {
      tanqueAtivos.desligaAtuador(2);                    // (* 2.5) Pausa a dosagem para homogeneizar a quimica com a agua
      tempoDosagem = millis();                           // Atualiza o tempo atual em milissegundos
      tempoDosagemAlcalinizante = 5000;                  // (* 2.7) Calcula o tempo de dosagem do alcalinizante com base no pH da água do tanque de ativos
      etapaAnterior = etapaAtual;                        // Atualiza a etapa anterior
      etapaAtual = EtapaProcesso::DosandoAlcalinizante;  // (* 2.6) Muda a etapa do processo para "DosandoAlcalinizante"
    } else {
      if (tempoDosagemCoagulante < 500.0f) {  // Caso o tempo calculado para a dosagem não tenha sido suficiente para acertar o ph,
        tempoDosagemCoagulante += 500.0f;     // adiciona mais 0.5s para
      }
      etapaAnterior = etapaAtual;                     // Atualiza a etapa anterior
      etapaAtual = EtapaProcesso::DosandoCoagulante;  // (* 2.4) Muda a etapa do processo para "DosandoAlcalinizante"
    }
  }

  /* Alcalinização */
  // TODO: Implementar Alcalinização (* 2.6) - Vai sair
  // Verificando o pH após a dosagem do alcalinizante
  if (etapaAtual == EtapaProcesso::VerificandoPhAposAlcalinizante) {
    // (* 2.9) Se o pH alvo foi atingido
    if (tanqueAtivos.getPH() >= alvoPhAlcalinizante - histerese ||
        tanqueAtivos.getPH() <= alvoPhAlcalinizante + histerese) {
      etapaAnterior = etapaAtual;                     // Atualiza a etapa anterior
      etapaAtual = EtapaProcesso::PreparaCoagulacao;  // (* 2.10) Muda a etapa do processo para "Coagulacao"
      tempoCoagulacao = millis();                     // Atualiza o tempo atual em milissegundos
      tanqueAtivos.desligaAtuador(3);                 // (* 2.9) Pausa a dosagem para homogeneizar a quimica com a agua
    } else {
      if (tempoDosagemAlcalinizante < 500.0f) {  // Caso o tempo calculado para a dosagem não tenha sido suficiente para acertar o ph,
        tempoDosagemAlcalinizante += 500.0f;     // adiciona mais 0.5s para
      }
      etapaAnterior = etapaAtual;                        // Atualiza a etapa anterior
      etapaAtual = EtapaProcesso::DosandoAlcalinizante;  // (* 2.8) Muda a etapa do processo para "DosandoAlcalinizante"
    }
  }

  /* Coagulação */
  if (etapaAtual == EtapaProcesso::PreparaCoagulacao) {          // (* 2.10) Inicio da coagulação
    if (millis() - tempoCoagulacao >= tempoDeVerificacao * 5) {  // (* 2.11) Mantém o agitador ligado por mais 5s
      tanqueAtivos.desligaAtuador(1);                            // (* 2.14) pausa a agitação por 5m
      tempoCoagulacao = millis();                                // Atualiza o tempo atual em milissegundos
      etapaAnterior = etapaAtual;                                // Atualiza a etapa anterior
      etapaAtual = EtapaProcesso::Coagulacao;                    // (* 2.14) Muda a etapa do processo para "Coagulação"
    }
  }

  if (etapaAtual == EtapaProcesso::Coagulacao) {                // (* 2.12) Inicio da coagulação
    if (millis() - tempoCoagulacao >= tempoEsperaCoagulacao) {  // (* 2.14) pausa a agitação por 5m
      tempoAtivos = millis();                                   // Atualiza o tempo atual em milissegundos
      etapaAnterior = etapaAtual;                               // Atualiza a etapa anterior
      etapaAtual = EtapaProcesso::PreparandoLibercaoEfluentes;  // (* 2.15) Muda a etapa do processo para "PreparandoLibercaoEfluentes
    }
  }

  /* Efluentes */
  if (etapaAtual == EtapaProcesso::PreparandoLibercaoEfluentes) {
    if (tanqueAtivos.getTurbidez() <= alvoNtu) {  // Alvo de ntu atingido
      tanqueAtivos.ligaAtuador(5);                // (* 2.15) Aciona HVK1

      if (millis() - tempoAtivos >= tempoDeVerificacao * 2) {
        tanqueAtivos.ligaAtuador(0);                     // (* 2.15) Liga bomba de transferencia
        tempoAtivos = millis();                          // Atualiza o tempo atual em milissegundos
        etapaAnterior = etapaAtual;                      // Atualiza a etapa anterior
        etapaAtual = EtapaProcesso::LiberandoEfluentes;  // (* 2.16) Muda a etapa do processo para "LiberandoEfluentes
      }
    }
  }

  if (etapaAtual == EtapaProcesso::LiberandoEfluentes) {
    if (millis() - tempoAtivos >= tempoTransferenciaEfluentes) {  //(* 2.16) Aguarda tempo de transferencia
      tanqueAtivos.desligaAtuador(0);                             // (* 2.17) Desliga HVK1
      tanqueAtivos.desligaAtuador(5);                             // (* 2.17) Desliga PM2
      tanqueAtivos.ligaAtuador(1);                                // (* 2.18) Liga Agitação
    }
  }

  /* Sanitização */
  // TODO: Implementar Sanitização 2.19

  // Mistura dos quimicos
  if (etapaAtual == EtapaProcesso::Homogeneizando) {  // (* 2.4, 2.8 e 2.20) Homogeneizar por um tempo
    tanqueAtivos.desligaAtuador(2);                   // Pausa a dosagem para homogeneizar a quimica com a agua
    tanqueAtivos.desligaAtuador(3);                   // Pausa a dosagem para homogeneizar a quimica com a agua
    tanqueAtivos.desligaAtuador(4);                   // Pausa a dosagem para homogeneizar a quimica com a agua

    if (millis() - tempoDosagem >= tempoMistura) {
      tempoDosagem = millis();     // Atualiza o tempo atual em milissegundos
      etapaAtual = etapaAnterior;  // Se o tempo de mistura foi atingido, retorna para a última etapa
    }
  }

  /* TODO: Não haverá mais o calculo de coagulante, será feito um teste para determinar a quantidade mais adequada
   *  portanto tempoDosagemCoagulante será um valor constante determinado como variável global
   *  item 2.3 não sera mais utilizado (linha 297)
   */

  /* TODO: Dosagem de alcalinizante não dependerá de tempo (item 2.7 linha 315) e sim da medição de pH.
   *  Dosar alcalinizante até ph 7
   */

  /* TODO: Dosagem de coagulante e alcalinizante andarão juntas. Caso o ph durante a dosagem de coagulante ficar menor que 6
   *  pausa dosagem de coagulante e dosa alcalinizante mantendo ele próximo de 7
   */
}
