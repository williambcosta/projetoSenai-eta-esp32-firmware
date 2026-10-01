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
#define PIN_SOL_EFLU_HVK1 "0.3"   // Pino da solenoide de controle de fluxo para o tanque de efluentes
#define PIN_SOL_FINAL_HVK2 "0.4"  // Pino da solenoide de controle de fluxo para o tanque final

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
  Inicial,                       // Etapa inicial do processo, indica que o tanque de armazenamento esta enchendo
  VerificNivelAltoArmaz,         // Verificação do nível alto do tanque de armazenamento de água bruta
  EsvaziandoTqArmaz,             // Esvaziamento do tanque de armazenamento
  VerificTransferenciaAtivos,    // Verificação do nível do tanque de ativos
  DosandoCoagulante,             // Dosagem do coagulante
  VerificandoPh,                 // Verificação de pH após dosagem de coagulante
  DosandoAlcalinizante,          // Dosagem de alcalinizante
  DosandoSanitizante,            // Dosagem de sanitizante
  PreparaCoagulacao,             // Continua a homogeinização antes da coagulação e decantação dos flocos
  Coagulacao,                    // Aguardando a coagulação e decantação dos flocos
  Homogeneizando,                // Homogeneizando mistura de químicos
  PreparandoLibercaoSedimentos,  // Preparando para liberar o decantado para o tanque de efluentes
  RemovendoSedimentos,           // Removendo sedimentos do tanque de ativos
  PreparandoEsvaziamentoAtivos,  // Preparando para liberar a agua tratada para o tanque final
  EsvaziandoTanqueAtivos,        // Esvaziamento do tanque de ativos
  TratamentoUv,                  // Tratamento com luz UV
  Finalizado                     // Processo finalizado
};

EtapaProcesso etapaAtual = EtapaProcesso::Inicial;     // Variável para armazenar a etapa atual do processo
EtapaProcesso controleEtapa = EtapaProcesso::Inicial;  // Variável para armazenar a etapa anterior do processo

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
    SaidaDigital(PIN_SOL_EFLU_HVK1),      // 5 - Solenoide de controle de fluxo para efluentes
    SaidaDigital(PIN_SOL_FINAL_HVK2)      // 6 - Solenoide de controle de fluxo para tanque final
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
int tempoDeVerificacao = 1000;         // Tempo para verificar o nível dos tanques
int tempoDosagemCoagulante = 5000;     // Tempo total de dosagem do coagulante
int tempoDosagemAlcalinizante = 5000;  // Tempo de dosagem do alcalinizante
int tempoDosagemSanitizante = 5000;    // Tempo de dosagem do sanitizante
int tempoPausaDosagem = 5000;          // Tempo que determina quando a dosagem será pausada para homogeneizar os quimicos com a agua
int tempoHomogeneizacao = 5000;        // Tempo após dosagem de quimica para homogeinização da agua antes de nova medição de parâmetros
int tempoDescargaEfluentes = 30000;    // Tempo que a bomba de transferencia do tanque de ativos fica ligada ao esvaziar o decantado para o tanque de efluentes
int tempoCoagulacao = 3000000;         // Tempo de espera para que a coagulação aconteça

// Temporizadores
unsigned long dtDosagemCoagulante = 0;     // Tempo percorrido de dosagem de coagulante
unsigned long dtDosagemAlcalinizante = 0;  // Tempo percorrido de dosagem de alcalinizante
unsigned long dtDosagemSanitizante = 0;    // Tempo percorrido de dosagem de sanitizante
unsigned long dtDescargaEfluentes = 0;     // Tempo percorrido de descarga dos efluentes
unsigned long dtArmazenamento = 0;         // Tempo percorrido de verificação do tanque de armazenamento
unsigned long dtAtivos = 0;                // Tempo percorrido de verificação do tanque de ativos
unsigned long dtHomogeneizacao = 0;        // Tempo percorrido de homogeneização de quimicos
unsigned long dtCoagulacao = 0;            // Tempo percorrido de coagulação dos residuos do tanque de ativos

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

  /** Processo de tratamento **/
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
  // 6.4 - Inicia a coagulação -------------------------------------------------------------------> Coagulacao

  // Coagulação //
  // 7.1 - Se tempo de coagulação for atingido
  // 7.2 - Se ntu alvo atingido, Aciona HVK1
  // 7.3 - Atualiza o temporizador de descarga de efluentes --------------------------------------> PreparandoLibercaoEfluentes
  // 7.4 - Caso ntu não atingido talvez nova dosagem de alcalinizante // TODO: talvez recomeçãr dosagem de coagulante

  // Descarga de Efluentes //
  // 7.5 - Se tempo de verificação, Acionando PM2, atualiza temporizador de descarga -------------> LiberandoEfluentes

  // Liberando Efluentes //
  // 7.6 - Manter PM2 até tempo de descarga
  // 7.7 - Desaciona HVK1, PM2 e
  // 7.8 - Atualiza o temporizador de dosagem de sanitizante e Iniciar agitação (RM1) ------------> DosandoSanitizante

  // Dosagem de Sanitizante //
  // 8.1 - Se o tempo de dosagem atingir tempo de dosagem total
  // 8.2 - Desliga Dosador DM3
  // 8.3 - Atualiza temporizador de transferencia para tanque final
  // 8.4 - Aciona HVK2 ---------------------------------------------------------------------------> PreparandoEsvaziamentoAtivos

  // Preparando Transferencia para Tanque Final //
  // 8.5 - Se tempo de verificação alcancado
  // 8.6 - Se tanque final nivel baixo, Aciona PM2 ------------------------------------------------> EsvaziandoTanqueAtivos

  /* Tanque Final */
  //  - Se Tanque ativos nivel baixo || Tanque final nivel alto
  //  - Desaciona RM1, Desaciona PM2 e Desaciona HVK2
  
  /* Condições sem etapa */
  // Tanque final //
  //  - Se Tanque final nivel alto, Aciona H1 e RM2
  //  - Caso contrario desliga H1 e RM2

  // Efluentes // 
  //  - Quando nível alto && (!Nível alto armazenamento)
  //  - Aciona PM3
  //  - Se Armazenamento Nível alto || Efluentes Nível baixo
  //  - Para PM3

  /* ----- Processo Tanque Armazenamento ----- */
  if (etapaAtual == EtapaProcesso::Inicial) {             // Tanque de armazenamento Enchendo
    if (tanqueArmazenamento.isNivelAlto()) {              // (1.1) Se tanque de armazenamento em nível alto
      dtArmazenamento = millis();                         // Atualiza variável auxiliar com o tempo atual em milissegundos
      etapaAtual = EtapaProcesso::VerificNivelAltoArmaz;  // (-> 1.1) Muda a etapa do processo para "VerificNivelAltoArmaz"
    }
  }

  if (etapaAtual == EtapaProcesso::VerificNivelAltoArmaz) {    // Verificando nível do tanque de armazenamento
    if (tanqueArmazenamento.isNivelAlto()) {                   // (1.1) Se nível alto tanque de armazenamento
      if (millis() - dtArmazenamento >= tempoDeVerificacao) {  // (1.2) Verifica se o tempo de verificação foi atingido desde o último acionamento
        tanqueArmazenamento.ligaAtuador(0);                    // (1.2)Liga a bomba PM1 para transferir água para o tanque de ativos
        etapaAtual = EtapaProcesso::EsvaziandoTqArmaz;         // (-> 1.2) Muda a etapa do processo para "EsvaziandoTqArmaz"
      }
    }
  }

  if (etapaAtual == EtapaProcesso::EsvaziandoTqArmaz) {                      // Esvaziando o tanque de armazenamento
    if (tanqueArmazenamento.isNivelBaixo() || tanqueAtivos.isNivelAlto()) {  // (1.3) Se nível baixo armazenamento ou nivel alto tanque de ativos
      dtArmazenamento = millis();                                            // Atualiza o tempo atual em milissegundos para o tanque de ativos
      etapaAtual = EtapaProcesso::VerificTransferenciaAtivos;                // (-> 1.3)Muda a etapa do processo para "VerificTransferenciaAtivos"
    }
  }

  if (etapaAtual == EtapaProcesso::VerificTransferenciaAtivos) {             // Verificando nível do tanque de ativos
    if (tanqueArmazenamento.isNivelBaixo() || tanqueAtivos.isNivelAlto()) {  // (1.3) Se nível baixo armazenamento ou nivel alto tanque de ativos
      if (millis() - dtArmazenamento >= tempoDeVerificacao) {                // (2.1) Verifica se o tempo de verificação foi atingido desde o último acionamento
        tanqueArmazenamento.desligaAtuador(0);                               // (2.1) Desliga a bomba PM1 para interromper a transferência de água para o tanque de ativos

        if (!tanqueAtivos.isNivelAlto()) {                // (2.2) Se o tanque de ativos ainda não estiver cheio
          etapaAtual = EtapaProcesso::Inicial;            // (-> 2.2)Muda a etapa do processo para "Inicial"
        } else {                                          // (2.3) Caso esteja cheio
          tanqueAtivos.ligaAtuador(1);                    // (2.3) Liga o misturador RM1 para iniciar a agitação da água no tanque de ativos
          dtDosagemCoagulante = millis();                 // (2.4) Atualiza o temporizador de dozagem do coagulante
          etapaAtual = EtapaProcesso::DosandoCoagulante;  // (-> 2.4) Muda a etapa do processo para "DosandoCoagulante"
        }
      }
    }
  }

  /* ----- Processo Tanque Ativos ----- */
  if (etapaAtual == EtapaProcesso::DosandoCoagulante) {  // Dosando o coagulante e verificando parâmetros
    tanqueAtivos.ligaAtuador(2);                         // (3.1) Liga o dosador do coagulante

    // Se o resto do (tempo atual menos a ultima atualização) dividido pelo tempo de pausa for == a 1, significa que se passou o tempo definido desde a ultima verificação
    if (((millis() - dtDosagemCoagulante) % tempoPausaDosagem) == 1) {  // (3.2) Após dosar uma pequena quantidade de coagulante
      tanqueAtivos.desligaAtuador(2);                                   // (3.3) Desliga o dosador de coagulante
      controleEtapa = EtapaProcesso::DosandoCoagulante;                 // (3.4) Atualiza o controle do processo como DosandoCoagulante garantindo o retorno para a dosagem de coagulante caso necessário

      if ((millis() - dtDosagemCoagulante) >= tempoDosagemCoagulante) {  // (3.5) Se o tempo total de dosagem do coagulante for alcançado
        controleEtapa = EtapaProcesso::PreparaCoagulacao;                // (3.6) Atualiza controle de etapa do processo
      }

      dtHomogeneizacao = millis();                // (3.7) Atualiza temporizador de Homogeneizacao
      etapaAtual = EtapaProcesso::VerificandoPh;  // (-> 3.7) Muda a etapa do processo para "VerificaPH"
    }
  }

  if (etapaAtual == EtapaProcesso::VerificandoPh) {            // Verificando o pH após a dosagem de quimicos
    if (millis() - dtHomogeneizacao >= tempoHomogeneizacao) {  // (4.1) Se o tempo de homogeneização concluido
      float ph = tanqueAtivos.getPH();

      if (ph < (6.0f - histerese)) {                              // (4.2) Se o ph da agua estiver muito ácido
        dtDosagemAlcalinizante = millis();                        // (4.2) Atualiza o temporizador de dosagem do alcalinizante
        controleEtapa = EtapaProcesso::DosandoAlcalinizante;      // (4.3) Muda o controle de etapa do processo para "DosandoAlcalinizante"
      } else if (ph >= alvoPhCoagulante - histerese &&            // (4.4) Se ph estiver acima do parâmetro de pH - histerese
                 ph <= alvoPhCoagulante + histerese) {            // (4.4) Ou abaixo do parâmetro + histerese
        if (controleEtapa == EtapaProcesso::PreparaCoagulacao) {  // (4.5) Se o controle do processo indicar que a proxima etapa é a coagulação
          dtCoagulacao = millis();                                // (4.6) Atualiza temporizador de coagulação
        }
      } else if (ph > alvoPhCoagulante + histerese) {      // (4.7) Se pH alcalino
        controleEtapa = EtapaProcesso::DosandoCoagulante;  // (4.7) Atualiza controle do processo indicando a proxima etapa como DosandoCoagulante
      }

      etapaAtual = controleEtapa;  // (-> 4.8) Muda a etapa do processo para "ControleEtapa"
    }
  }

  if (etapaAtual == EtapaProcesso::DosandoAlcalinizante) {  // Dosando o alcalinizante e verificando parâmetros
    tanqueAtivos.ligaAtuador(3);                            // (5.1) Liga o dosador do alcalinizante

    if (((millis() - dtDosagemAlcalinizante) == tempoDosagemAlcalinizante)) {  // (5.2) Após dosar uma pequena quantidade de alcalinizante
      tanqueAtivos.desligaAtuador(3);                                          // (5.3) Desliga DM2
      controleEtapa = EtapaProcesso::DosandoCoagulante;                        // (5.4) Atualiza o controle do processo para voltar a etapa de dosagem de coagulante após verificar pH
      etapaAtual = EtapaProcesso::VerificandoPh;                               // (-> 5.4) Muda a etapa do processo para VerificandoPh
    }
  }

  /* Preparo para Coagulação */
  if (etapaAtual == EtapaProcesso::PreparaCoagulacao) {    // Inicio da coagulação
    if (millis() - dtCoagulacao >= tempoHomogeneizacao) {  // (6.1) Mantém o agitador ligado pelo tempo definido para homogeneização
      dtCoagulacao = millis();                             // (6.2) Atualiza o tempo atual em milissegundos
      tanqueAtivos.desligaAtuador(1);                      // (6.3) Desliga o mexedor RM1 para aguardar a coagulação dos sólidos
      etapaAtual = EtapaProcesso::Coagulacao;              // (-> 6.4) Muda a etapa do processo para "Coagulação"
    }
  }

  /* Coagulação */
  if (etapaAtual == EtapaProcesso::Coagulacao) {                   // Aguardando coagulação dos sólidos
    if (millis() - dtCoagulacao >= tempoCoagulacao) {              // (7.1) Aguarda o tempo determinado de coagulação antes de seguir com o processo
      if (tanqueAtivos.getTurbidez() <= alvoNtu) {                 // (7.2) Alvo de ntu atingido
        tanqueAtivos.ligaAtuador(5);                               // (7.2) Aciona HVK1
        dtDescargaEfluentes = millis();                            // (7.3) Atualiza o temporizador de descarga de efluentes
        etapaAtual = EtapaProcesso::PreparandoLibercaoSedimentos;  // (-> 7.3) Muda a etapa do processo para "PreparandoLibercaoSedimentos
      }

      // TODO: Else{ talvez nova dosagem de coagulante}
    }
  }

  /* Efluentes */
  if (etapaAtual == EtapaProcesso::PreparandoLibercaoSedimentos) {
    if (millis() - dtDescargaEfluentes >= tempoDescargaEfluentes) {  // (7.6) Aguarda tempo de transferencia
      tanqueAtivos.ligaAtuador(0);                                   // (7.5) Liga bomba de transferencia
      dtDescargaEfluentes = millis();                                // (7.5) Atualiza o tempo atual em milissegundos
      etapaAtual = EtapaProcesso::RemovendoSedimentos;               // (-> 7.5) Muda a etapa do processo para "RemovendoSedimentos
    }
  }

  if (etapaAtual == EtapaProcesso::RemovendoSedimentos) {
    if (millis() - dtDescargaEfluentes >= tempoDescargaEfluentes) {  // (7.6) Aguarda tempo de transferencia
      tanqueAtivos.desligaAtuador(0);                                // (7.7) Desliga bomba
      tanqueAtivos.desligaAtuador(5);                                // (7.7) Desliga solenoide
      tanqueAtivos.ligaAtuador(1);                                   // (7.7) Liga Agitação
      dtDosagemSanitizante = millis();                               // (7.8) Atualiza temporizador de dosagem de sanitizante
      etapaAtual = EtapaProcesso::DosandoSanitizante;                // (-> 7.8) Muda etapa do processo para "DosandoSanitizante"
    }
  }

  /* Sanitização */
  if (etapaAtual == EtapaProcesso::DosandoSanitizante) {                 // Dosando o sanitizante e verificando parâmetros
    if ((millis() - dtDosagemSanitizante) >= tempoDosagemSanitizante) {  // (8.1) Se o tempo total de dosagem do sanitizante for alcançado
      tanqueAtivos.desligaAtuador(4);                                    // (8.2) Desliga o dosador de sanitizante
      dtAtivos = millis();                                               // (8.3) Atualiza temporizador
      tanqueAtivos.ligaAtuador(6);                                       // (8.4) Aciona Solenoide de transferencia para o tanque final
      controleEtapa = EtapaProcesso::PreparandoEsvaziamentoAtivos;       // (-> 8.4) Atualiza controle de etapa do processo
    }
  }

  /* Transferencia para tanque final */
  if (etapaAtual == EtapaProcesso::PreparandoEsvaziamentoAtivos) {  // Preparando para transferir
    if (millis() - dtAtivos >= tempoDeVerificacao) {                // (8.5) Se tempo de verificação atingido
      if (tanqueFinal.isNivelBaixo()) {                             // (8.6) Se tanque final em nível baixo
        tanqueAtivos.ligaAtuador(0);                                // (8.6) Liga bomba de transferencia
        etapaAtual = EtapaProcesso::EsvaziandoTanqueAtivos;         // (-> 8.6) Muda etapa do processo para EsvaziandoTanqueAtivos
      }
    }
  }


  /* Condições sem etapa específica */
  if (etapaAtual == EtapaProcesso::EsvaziandoTanqueAtivos) {
    if (tanqueAtivos.isNivelBaixo() || tanqueFinal.isNivelAlto()) {

    }
  }

  // TODO: Implementar Descarga para o tanque final
  // TODO: Implementar Tanque de efluentes
  // TODO: Ação de interromper as bombas deve acontecer fora de uma etapa específica. Verificar encessidade de usar controle de 
  // TODO: processo ou somente seguir para a próxima etapa resolve
}
