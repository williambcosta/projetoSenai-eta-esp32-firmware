# Firmware ESP32 — Controle & Telemetria para ETA

> **Projeto de Conclusão de Curso (TCC)**  
> Curso Técnico em Eletroeletrônica — SENAI  
> **Grupo 2**

---

## Sobre o Repositório

Este repositório contém o código-fonte C++ gravado no microcontrolador **ESP32**. 

O firmware é responsável por automatizar a maquete física da Estação de Tratamento de Água (ETA), realizando a leitura periódica de sensores e gerenciando o acionamento de bombas e dosadores em tempo real.

---

## Arquitetura de Hardware & Bibliotecas

### **Periféricos da Maquete**
* **Sensores:** pH, Turbidez, Nível dos Tanques e Temperatura.
* **Atuadores:** Motores / Reles e Válvula Solenoide.

### **Bibliotecas Principais**
* **[WiFi.h](https://www.arduino.cc/en/Reference/WiFi)** — Conexão do ESP32 à rede Wi-Fi local no modo *Station*.
* **[PubSubClient](https://github.com/knolleary/pubsubclient)** — Cliente MQTT leve para comunicação com o broker na nuvem (**HiveMQ Cloud**).
* **[ArduinoJson](https://arduinojson.org/)** — Serialização das leituras dos sensores em formato JSON e parsing das mensagens de comando.

---

## Fluxo de Comunicação (MQTT)

```text
[ Sensores/Atuadores ] ──(Pinos GPIO)──> [ ESP32 ] ──(Wi-Fi)──> [ HiveMQ Cloud ] ──> [ Dashboard Web Vue.js ]
```