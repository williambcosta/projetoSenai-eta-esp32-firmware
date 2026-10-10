#include "MqttManager.h"

// Inicializa o ponteiro estático como nulo
MqttManager* MqttManager::_instance = nullptr;

MqttManager::MqttManager(const char* ssid, const char* wifiSenha, const char* servidor, int porta, const char* usuario, const char* usuarioSenha) {
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

    if (client.connect(clientId.c_str(), mqtt_usuario, mqtt_senha)) {  // Tenta conectar ao broker MQTT com o id único e as credenciais fornecidas
      Serial.println("conectado!");
      if (mqtt_topico_comandos != nullptr) {     // Se o tópico de comandos foi definido
        client.subscribe(mqtt_topico_comandos);  // Assina o tópico de comandos para receber mensagens
      }
    } else {
      Serial.print("Falha: ");
      Serial.println(client.state());
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

  msgAtual.reserve(51);
  ultimaMsg.reserve(51);
}

// Callback estático que será chamado pelo PubSubClient quando uma mensagem for recebida
void MqttManager::mqttCallback(char* topico, byte* mensagem, unsigned int tamanho) {
  if (_instance != nullptr) {                         // Verifica se a instancia existe
    _instance->handleMsg(topico, mensagem, tamanho);  // Redireciona os dados para a função real da classe que lida com a mensagem
  }
}

// Função que processa a mensagem recebida
void MqttManager::handleMsg(char* topico, byte* mensagem, unsigned int tamanho) {
  static String msg = "";
  msg.reserve(1024);  // Reserva espaço para a mensagem recebida
  msg = "";           // Limpa a mensagem para receber a nova

  // Transforma o array de bytes recebido em uma string para facilitar o processamento
  for (int i = 0; i < tamanho; i++) {
    msg += (char)mensagem[i];
  }

  Serial.printf("[Classe MqttManager] Mensagem recebida no [%s]: %s\n", topico, msg.c_str());  // Loga a mensagem recebida no console
  msgAtual = msg;                                                                              // Salva a última mensagem recebida para que possa ser acessada posteriormente
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

  if (msgAtual != ultimaMsg) {
    ultimaMsg = msgAtual;
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
    return true;
  } else {  // Caso contrario sinaliza a falha retornando false
    return false;
  }
}

// Função para verificar se o cliente MQTT está conectado ao servidor
bool MqttManager::isConnected() {
  return client.connected();  // Retorna o estado da conexão com o servidor MQTT
}

// Função para adicionar uma mensagem ao buffer de mensagens a serem publicadas
bool MqttManager::addMensagemAlerta(const char* mensagem) {
  if (totalMsgs >= 10) {
    return false;  // Fila cheia
  }

  snprintf(poolMsg[inicio], 256, "%s", mensagem);  // Copia a mensagem para o buffer
  inicio = (inicio + 1) % 10;                      // Avança o índice e volta ao 0 quando chega em 10
  totalMsgs++;                                     // Incrementa o total de mensagens acumuladas

  return true;
}

// Consome e envia a mensagem mais antiga
bool MqttManager::publicarProximaMensagemAlerta() {
  if (totalMsgs == 0) {
    return false;  // Fila vazia
  }

  if (!publish(TOPICO_ALERTAS, poolMsg[fim])) {
    return false;  // Caso o envio falhe retorna false
  }

  fim = (fim + 1) % 10;  // Avança a leitura
  totalMsgs--;

  return true;
}

// Função para publicar todas as mensagens curtas armazenadas no buffer
bool MqttManager::publicarMensagensAlerta() {
  while (totalMsgs > 0) {
    if (!publicarProximaMensagemAlerta()) {  // Caso o envio de alguma mensagem falhar retorna false
      return false;
    }
  }
  return true;
}