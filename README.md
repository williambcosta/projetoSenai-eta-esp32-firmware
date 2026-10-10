# Firmware ESP32 — Controle & Telemetria para ETA

> **Projeto de Conclusão de Curso (TCC)**  
> Curso Técnico em Eletroeletrônica — SENAI  
> **Grupo 2**

---

## Sobre o Repositório

Este projeto foi desenvolvido como **Trabalho de Conclusão de Curso (TCC)** do grupo 2 do curso Técnico em Eletroeletrônica do **SENAI** e tem como objetivo demonstrar a capacidade dos integrantes de projetar, implementar e validar uma solução completa de **Internet das Coisas (IoT)** para monitoramento telemétrico em tempo real, utilizando as competencias adquiridas durante o curso.

A proposta do projeto é desenvolver um sistema de monitoramento e controle remoto para uma maquete física de uma **Estação de Tratamento de Água (ETA)**, permitindo a coleta de dados de sensores, o acionamento de atuadores e a visualização das informações em tempo real através de um dashboard web.

A solução utiliza o microcontrolador **ESP32** para a aquisição de dados através de sensores digitais e analógicos, realizando a estruturação, sincronização temporal e envio seguro das medições para a nuvem via protocolo **MQTT**.

Este repositório contém o código-fonte C++ do firmware gravado no microcontrolador, que será responsável por automatizar a maquete física da Estação de Tratamento de Água (ETA), realizando a leitura periódica de sensores e gerenciando o acionamento de bombas e dosadores em tempo real.

### Objetivos do Sistema

* **Coleta de Dados:** Leitura precisa de temperatura utilizando sensores digitais via barramento *1-Wire*.
* **Sincronização Temporal:** Registro exato da data e hora de cada medição utilizando servidores NTP.
* **Padronização de Dados:** Serialização das leituras no formato estruturado **[JSON](https://www.json.org/json-en.html)** para facilitada integração com dashboards e bancos de dados.
* **Comunicação na Nuvem:** Transmissão eficiente e leve via **MQTT** integrada ao broker em nuvem **[HiveMQ Cloud](https://www.hivemq.com/)**.
* **Arquitetura Escalável e Não Bloqueante:** Utilização de conexões Wi-Fi em modo *Station* e bibliotecas assíncronas para garantir estabilidade e resposta rápida do sistema.

---

## Configurações do Projeto

As definições de configuração do projeto estão localizadas no arquivo `configuracoes.h`. Antes de compilar o firmware, é necessário criar o arquivo `configuracoes.h` na raiz do projeto e preencher as informações de conexão com a rede Wi-Fi e o broker MQTT.

Abaixo está um exemplo de como o arquivo `configuracoes.h` deve ser estruturado:

```cpp
#define MQTT_BROKER "url.do.broker"  // URL do broker MQTT (HiveMQ Cloud)
#define MQTT_PORTA 8883              // Porta padrão para conexões MQTT seguras (TLS/SSL)
#define MQTT_SENHA "espclient12345"  // Senha do usuário MQTT
#define MQTT_USUARIO "espclient"     // Usuário MQTT
#define WIFI_SENHA "WIFI_SENHA"      // Senha da rede Wi-Fi
#define WIFI_SSID "WIFI_SSID"        // SSID da rede Wi-Fi
```

---

## Arquitetura de Hardware & Bibliotecas

### **Periféricos da Maquete**

* **Sensores:** pH, Turbidez, Nível dos Tanques e Temperatura.
* **Atuadores:** Motores / Reles e Válvula Solenoide.

### **Bibliotecas Principais**

* **[WiFi.h](https://www.arduino.cc/en/Reference/WiFi)** — Conexão do ESP32 à rede Wi-Fi local no modo *Station*.
* **[PubSubClient](https://github.com/knolleary/pubsubclient)** — Cliente MQTT leve para comunicação com o broker na nuvem (**[HiveMQ Cloud](https://www.hivemq.com/)**).
* **[AsyncMqttClient](https://github.com/marvinroger/async-mqtt-client)** — Cliente MQTT assíncrono para gerenciamento de mensagens e eventos de rede de forma não bloqueante.
* **[ArduinoJson](https://arduinojson.org/)** — Serialização das leituras dos sensores em formato **[JSON](https://www.json.org/json-en.html)** e parsing das mensagens de comando.
* **[DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library)** — Leitura e controle dos sensores de temperatura digitais DS18B20.
* **[OneWire](https://github.com/PaulStoffregen/OneWire)** — Comunicação no barramento de fio único (*1-Wire*) necessário para a interface com os sensores de temperatura.
* **[NTPClient](https://github.com/arduino-libraries/NTPClient)** — Sincronização do relógio do ESP32 via servidores NTP para registro exato de horário (*timestamp*) nas medições.

---

## Fluxo de Comunicação (MQTT)

```text
[ Sensores/Atuadores ] ──(Pinos GPIO)──> [ ESP32 ] ──(Wi-Fi)──> [ HiveMQ Cloud ] ──> [ Dashboard Web Vue.js ]
```

## Próximos Passos

* [X] Configuração inicial do projeto
* [X] Criação de Classes para os sensores
  * [X] SensorTurbidez
    * [X] Calibração
  * [X] PH
    * [X] Calibração
  * [X] Temperatura
* [X] Criação de Classes para atuadores
* [X] Desenvolvimento inicial do fluxo do processo
  * [X] Configuração inicial das classes, sensores e atuadore
  * [X] Processo tanque inicial
  * [X] Processo tanque de ativos
    * [X] Dosagem de Coagulante
    * [X] Dosagem de alcalinizante
    * [X] Dosagem de sanitizante
  * [X] Processo tanque de efluentes
  * [X] Processo tanque de água tratada
    * [X] Tratamento UV
* [X] Comunicação com HiveMQ Cloud
  * [X] Conexão com o broker MQTT
  * [X] Publicação das leituras dos sensores
  * [X] Recebimento de comandos para acionamento dos atuadores
  * [X] Criação de alertas
* [ ] Teste final do processo
