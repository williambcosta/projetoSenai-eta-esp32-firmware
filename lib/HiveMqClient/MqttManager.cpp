#include "MqttManager.h"

// Inicializa o ponteiro estático como nulo
MqttManager* MqttManager::_instance = nullptr;

MqttManager::MqttManager(const char* ssid, const char* wifiSenha,
                         const char* servidor, int porta,
                         const char* usuario, const char* usuarioSenha)
    : alertas{
          {0, 0, "Alertas", false, false},
          {1, 2, "Tanque de Armazenamento: Falha em chaves de nível", false, false},
          {2, 2, "Tanque de Ativos: Falha em chaves de nível", false, false},
          {3, 2, "Tanque Final: Falha em chaves de nível", false, false},
          {4, 2, "Tanque de Eluentes: Falha em chaves de nível", false, false},
          {5, 1, "Tanque de Ativos: pH abaixo de 4.0", false, false},
          {6, 1, "Tanque de Ativos: pH acima de 8.0", false, false},
          {7, 1, "Tanque Final: pH abaixo de 4.0", false, false},
          {8, 1, "Tanque Final: pH acima de 8.0", false, false},
          {9, 2, "Tanque Final: Turbidez acima de 5 NTU", false, false},
          {10, 0, "Tanque de Armazenamento: Temperatura abaixo dos 15 graus", false, false},
          {11, 0, "Tanque de Armazenamento: Temperatura acima dos 35 graus", false, false},
          {12, 0, "Tanque de Ativos: Temperatura abaixo dos 15 graus", false, false},
          {13, 0, "Tanque de Ativos: Temperatura acima dos 35 graus", false, false},
          {14, 0, "Tanque Final: Temperatura abaixo dos 15 graus", false, false},
          {15, 0, "Tanque Final: Temperatura acima dos 35 graus", false, false},
          {16, 1, "Impossível Ligar PM1: Tanque inicial em nível baixo", false, false},
          {17, 1, "Impossível Ligar PM1: Tanque de ativos em nível alto", false, false},
          {18, 1, "Impossível Ligar PM2: Nenhuma solenoide acionada", false, false},
          {19, 1, "Impossível Ligar PM2: Ambas as solenoides estão acionadas", false, false},
          {20, 1, "Impossível Ligar PM2: Tanque de efluentes em nível alto", false, false},
          {21, 1, "Impossível Ligar PM2: Tanque final em nível alto", false, false},
          {22, 1, "Impossível Ligar PM3: Tanque de armazenamento em nível alto", false, false},
          {23, 1, "Impossível Ligar PM3: Tanque de efluentes em nível baixo", false, false},
          {24, 1, "Impossível Ligar SV1: Solenoide SV2 ligada", false, false},
          {25, 1, "Impossível Ligar SV2: Solenoide SV1 ligada", false, false}} {
  wifi_ssid = ssid;
  wifi_senha = wifiSenha;
  mqtt_servidor = servidor;
  mqtt_porta = porta;
  mqtt_usuario = usuario;
  mqtt_senha = usuarioSenha;

  client.setClient(espClient);  // Configura o cliente MQTT para usar o cliente seguro
  client.setKeepAlive(30);      // Configura o tempo de conexão para 30 segundos. Caso nenhuma interação aconteça dentro desse tempo a conexão é automaticamente derrubada
  _instance = this;             // Salva esta instância para o callback estático usar
}

// Função para configurar e conectar ao Wi-Fi
void MqttManager::setupWifi() {
  if (WiFi.status() == WL_CONNECTED) return;

  // Dispara a tentativa de conexão sem aguardar em loop bloqueante
  Serial.print("\nIniciando conexão Wi-Fi em ");
  Serial.println(wifi_ssid);
  Serial.println("");
  WiFi.begin(wifi_ssid, wifi_senha);
}

// Função para reconectar ao servidor MQTT caso a conexão seja perdida. Tenta apenas uma vez para evitar travar o loop principal caso não consiga se conectar
void MqttManager::reconnect() {
  // Verifica Wi-Fi de forma assíncrona
  if (WiFi.status() != WL_CONNECTED) {
    setupWifi();
    return;  // Retorna imediatamente para não bloquear a malha principal de segurança
  }

  // Se Wi-Fi está OK, tenta conectar ao MQTT Broker
  if (!client.connected()) {                                                    // verifica se o cliente MQTT está conectado
    String clientId = "ESP32-ETA-" + String((uint32_t)ESP.getEfuseMac(), HEX);  // Cria um id único para o cliente MQTT baseado no MAC do ESP32
    Serial.print("Tentando conexão MQTT com HiveMQ...");
    Serial.println("");

    if (client.connect(clientId.c_str(), mqtt_usuario, mqtt_senha)) {  // Tenta conectar ao broker MQTT com o id único e as credenciais fornecidas
      Serial.println("conectado!");
      Serial.println("");
      if (mqtt_topico_comandos != nullptr) {     // Se o tópico de comandos foi definido
        client.subscribe(mqtt_topico_comandos);  // Assina o tópico de comandos para receber mensagens
      }
    } else {
      Serial.print("Falha: ");
      Serial.println(client.state());
      Serial.println("");
    }
  }
}

// Função para iniciar a conexão Wi-Fi e configurar o cliente MQTT
void MqttManager::begin(const char* topicoComandos, const char* topicoTelemetria, const char* topicoAlertas) {
  mqtt_topico_comandos = topicoComandos;
  mqtt_topico_telemetria = topicoTelemetria;
  mqtt_topico_alertas = topicoAlertas;

  setupWifi();  // Configura a conexão Wi-Fi

  espClient.setInsecure();                        // Não valida a conexão, apenas aceita o que recebe. Para o nosso fim é o suficiente
  client.setServer(mqtt_servidor, mqtt_porta);    // Configura o servidor
  client.setCallback(MqttManager::mqttCallback);  // Configura o callback responsável pelo recebimento das mensagens
  client.setBufferSize(1024);                     // Configura o tamanho do buffer para receber mensagens maiores
}

// Callback estático que será chamado pelo PubSubClient quando uma mensagem for recebida
void MqttManager::mqttCallback(char* topico, byte* mensagem, unsigned int tamanho) {
  if (_instance != nullptr) {                         // Verifica se a instancia existe
    _instance->handleMsg(topico, mensagem, tamanho);  // Redireciona os dados para a função real da classe que lida com a mensagem
  }
}

// Função que processa a mensagem recebida
void MqttManager::handleMsg(char* topico, byte* mensagem, unsigned int tamanho) {
  snprintf(msgAtual, tamanho + 1, "%s", mensagem);  // Copia a mensagem recebida para o buffer de mensagem atual

  Serial.printf("[Classe MqttManager] Mensagem recebida no [%s]: %s\n", topico, msgAtual);  // Loga a mensagem recebida no console
  Serial.println("");
}

// Função que deve ser chamada no loop principal para manter a conexão
bool MqttManager::handle() {
  if (client.connected()) {
    client.loop();
  } else {
    // Tenta reconectar apenas quando transcorrer o intervalo especificado
    unsigned long agora = millis();
    if (agora - ultimoIntervaloReconexao >= intervaloReconexao) {
      ultimoIntervaloReconexao = agora;
      reconnect();
    }
  }

  if (strcmp(msgAtual, ultimaMsg) != 0) {
    strcpy(ultimaMsg, msgAtual);  // Atualiza a última mensagem recebida
    return true;
  } else {
    return false;
  }
}

// Função para publicar mensagens em um tópico específico
bool MqttManager::publish(uint8_t topico, const char* mensagem) {
  if (!client.connected()) return false;  // Caso não esteja conectado não faz nada

  // Seleciona o tópico correto baseado no parâmetro recebido
  const char* topicoSelecionado = (topico == TOPICO_TELEMETRIA) ? mqtt_topico_telemetria : ((topico == TOPICO_COMANDOS) ? mqtt_topico_comandos : mqtt_topico_alertas);

  // Proteção contra ponteiro nulo caso o tópico não tenha sido definido
  if (topicoSelecionado == nullptr) return false;

  if (client.publish(topicoSelecionado, mensagem)) {  // Tenta publicar a mensagem
    // Caso bem sucedida loga a saida e retorna true sinalizando o sucesso
    Serial.println("Mensagem publicada");
    Serial.print("Topico: ");
    Serial.println(topicoSelecionado);
    Serial.print("Mensagem:");
    Serial.println(mensagem);
    Serial.println("");
    return true;
  } else {  // Caso contrario sinaliza a falha retornando false
    return false;
  }
}

// Função para verificar se o cliente MQTT está conectado ao servidor
bool MqttManager::isConnected() {
  return client.connected();  // Retorna o estado da conexão com o servidor MQTT
}

// Função para ativar um alerta específico
void MqttManager::ativarAlerta(int codigoAlerta) {
  if (!alertas[codigoAlerta].ativo) {
    alertas[codigoAlerta].ativo = true;  // Ativa o alerta correspondente ao código fornecido
  }
}

// Função para desativar um alerta específico
void MqttManager::desativarAlerta(int codigoAlerta) {
  Serial.println("mensagem\n");
  alertas[codigoAlerta].ativo = false;    // Desativa o alerta correspondente ao código fornecido
  alertas[codigoAlerta].enviado = false;  // Marca o alerta como não enviado
}

// Função para publicar todas as mensagens curtas armazenadas no buffer
bool MqttManager::publicarMensagensAlerta() {
  for (int i = 0; i < 26; i++) {
    if (alertas[i].ativo && !alertas[i].enviado) {
      // Formata a mensagem de alerta no buffer antes de enviá-la
      snprintf(bufferMsgAlerta,
               sizeof(bufferMsgAlerta),
               "%d;%d;%s",
               alertas[i].codigo,
               alertas[i].severidade,
               alertas[i].mensagem);
      if (!publish(TOPICO_ALERTAS, bufferMsgAlerta)) {  // Tenta publicar a mensagem de alerta
        return false;                                   // Caso o envio falhe retorna false
      }
    }
    alertas[i].enviado = true;  // Marca o alerta como enviado
  }
  return true;
}