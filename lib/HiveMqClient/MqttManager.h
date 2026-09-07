/**
 * Classe para gerenciar a comunicação MQTT com o HiveMq Cloud, incluindo conexão Wi-Fi, publicação e assinatura de tópicos.
 *
 * Deve ser configurada informando rede wifi a ser conectado, senha da rede, servidor mqtt, porta, usuário mqtt, senha e o método hundle deve ser
 * chamado na função loop pois a mesma é responsável por verificar a chegada de novas mensagens vindas do servidor.
 */

#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

class MqttManager {
   private:
    const char* wifi_ssid;             // SSID da rede Wi-Fi
    const char* wifi_senha;            // Senha da rede Wi-Fi
    const char* mqtt_servidor;         // Endereço do servidor MQTT
    int mqtt_porta;                    // Porta do servidor MQTT
    const char* mqtt_usuario;          // Nome de usuário para autenticação MQTT
    const char* mqtt_senha;            // Senha para autenticação MQTT
    const char* mqtt_topico_dados;     // Tópico para publicar dados. É como um filtro de mensagems
    const char* mqtt_topico_comandos;  // Tópico para receber dados.

    WiFiClientSecure espClient;  // Cliente seguro para comunicação MQTT
    PubSubClient client;         // Cliente MQTT

    String ultimaMsg = "";  // Armazena a última mensagem recebida. Usar com cautela, pois só é atualizado com uma nova mensagem.

    static MqttManager* _instance;  // Ponteiro estático para armazenar a instância atual da classe

    void setupWifi();  // Função para configurar a conexão Wi-Fi
    void reconnect();  // Função para reconectar ao servidor MQTT caso a conexão seja perdida

    // Função que processa a mensagem recebida
    void handleMsg(char* topico, byte* payload, unsigned int tamanho);

    // Callback estático que será usado pela PubSubClient
    static void mqttCallback(char* topico, byte* payload, unsigned int tamanho);

   public:
    MqttManager(const char* ssid, const char* wifiSenha, const char* servidor, int porta, const char* usuario, const char* usuarioSenha);

    void begin(const char* topicoComandos = nullptr, const char* topicoDados = nullptr);   // Inicializa a conexão Wi-Fi e configura o cliente MQTT, opcionalmente assinando um tópico de comandos
    void handle();                                                                         // Função responsável por chamar o loop do cliente MQTT e manter a conexão ativa
    bool publish(const char* payload);                                                     // Função para publicar mensagens em um tópico específico

    String getUltimaMsg() const { return ultimaMsg; }  // Retorna a última mensagem recebida

    void setTopicoDados(const char* topico) { mqtt_topico_dados = topico; }        // Define o tópico para publicar os dados
    void setTopicoComandos(const char* topico) { mqtt_topico_comandos = topico; }  // Define o tópico para receber dados
};

#endif