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

// Função para configurar a conexão Wi-Fi
void MqttManager::setupWifi() {
  if (WiFi.status() == WL_CONNECTED) return;  // Caso a placa já esteja conectada retorna sem fazer nada

  Serial.print("\nConectando em ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_senha);  // Tenta se conectar

  while (WiFi.status() != WL_CONNECTED) {  // Verifica o status da conexão enquanto ela não for estabelecida
    delay(500);                            // aguarda meio segundo para verificar novament o status da conexão
    Serial.print(".");
  }

  Serial.println("\nWi-Fi conectado!");
}

// Função para reconectar ao servidor MQTT caso a conexão seja perdida
void MqttManager::reconnect() {
  while (!client.connected()) {
    setupWifi();  // Garante que o Wi-Fi está ligado antes de tentar o MQTT

    Serial.print("Tentando conexão MQTT com HiveMQ...");
    String clientId = "espclient";

    // Tenta se conectar ao servidor
    if (client.connect(clientId.c_str(), mqtt_usuario, mqtt_senha)) {
      Serial.println("conectado!");

      // Se houver um tópico para assinar, assina automaticamente
      if (mqtt_topico_comandos != nullptr) {
        client.subscribe(mqtt_topico_comandos);
      }

    } else {  // Caso não consiga se conectar espera 5 segundos para tentar novamente
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" Reconectando em 5 segundos...");

      delay(5000);
    }
  }
}

// Função para iniciar a conexão Wi-Fi e configurar o cliente MQTT
void MqttManager::begin(const char* topicoComandos, const char* topicoDados, const char* topicoAlertas) {
  mqtt_topico_comandos = topicoComandos;
  mqtt_topico_dados = topicoDados;
  mqtt_topico_alertas = topicoAlertas;
  espClient.setInsecure();                        // Não valida a conexão, apenas aceita o que recebe. Para o nosso fim é o suficiente
  client.setServer(mqtt_servidor, mqtt_porta);    // Configura o servidor
  client.setCallback(MqttManager::mqttCallback);  // Configura o callback responsável pelo recebimento das mensagens
}

// Callback estático que será chamado pelo PubSubClient quando uma mensagem for recebida
void MqttManager::mqttCallback(char* topico, byte* mensagem, unsigned int tamanho) {
  if (_instance != nullptr) {                         // Verifica se a instancia existe
    _instance->handleMsg(topico, mensagem, tamanho);  // Redireciona os dados para a função real da classe que lida com a mensagem
  }
}

// Função que processa a mensagem recebida
void MqttManager::handleMsg(char* topico, byte* mensagem, unsigned int tamanho) {
  String menssagem = "";
  for (int i = 0; i < tamanho; i++) {
    menssagem += (char)mensagem[i];
  }

  Serial.printf("[Classe MqttManager] Mensagem recebida no [%s]: %s\n", topico, menssagem.c_str());

  ultimaMsg = menssagem;
}

// Função que deve ser chamada no loop principal para manter a conexão
void MqttManager::handle() {
  if (!client.connected()) {  // Caso o cliente não esteja conectado
    reconnect();              // Reconecta
  }

  client.loop();  // Itera a conexão MQTT, verifica novas mensagens, envia as pendentes e mantém a conexão ativa
}

// Função para publicar mensagens em um tópico específico
bool MqttManager::publish(uint8_t topico, const char* mensagem) {
  if (!client.connected()) return false;  // Caso não esteja conectado não faz nada

  // Seleciona o tópico correto baseado no parâmetro recebido
  const char* topicoSelecionado = (topico == TOPICO_DADOS) ? mqtt_topico_dados : ((topico == TOPICO_COMANDOS) ? mqtt_topico_comandos : mqtt_topico_alertas);

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