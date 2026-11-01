# bars-ipc Specification

## Purpose

A comunicação do daemon das barras de reserva (`kwinish-ui bars`) com o compositor, sem travar nem aceitar respostas parciais ou grandes demais.

## Requirements

### Requirement: Leitura limitada do socket das barras
O daemon SHALL limitar o tempo e o tamanho da resposta do socket e descartar uma resposta incompleta ou excessiva.

#### Scenario: Resposta parada
- **WHEN** o compositor não termina de enviar a resposta no prazo
- **THEN** a leitura termina e a resposta é descartada

#### Scenario: Resposta excessiva
- **WHEN** a resposta excede o limite de tamanho
- **THEN** a leitura termina e a resposta é descartada
