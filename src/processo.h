// Estrutura para armazenar informações sobre o processo de tratamento de água
struct Processo {
  enum class ModoOperacao {
    Automatico,  // Modo automático
    Manual,      // Modo manual de operação, Bombas continuam a ser desacionadas dependendo do nível dos tanques para evitar transbordo
    Parada,      // Finaliza a etapa atual e para o processo
    Emergencia   // Para todos os atuadores e reínicia todos os parâmetros
  };

  // Converte o modo de operação para uma string legível
  const char* modoOperacaoToString(ModoOperacao modo) {
    switch (modo) {
      case ModoOperacao::Manual:
        return "Manual";
      case ModoOperacao::Parada:
        return "Parada";
      case ModoOperacao::Emergencia:
        return "Emergencia";
      default:
        return "Automatico";
    }
  }

  ModoOperacao modoOperacao = ModoOperacao::Automatico;  // Armazena o modo de operação atual do processo

  // Enumeração para representar as etapas do processo de tratamento de água
  enum class Etapa {
    Inicial,                       // Etapa inicial do processo, indica que o tanque de armazenamento esta enchendo
    VerificNivelAltoArmaz,         // Verificação do nível alto do tanque de armazenamento de água bruta
    EsvaziandoTqArmaz,             // Esvaziamento do tanque de armazenamento
    VerificTransferenciaAtivos,    // Verificação do nível do tanque de ativos
    DosandoCoagulante,             // Dosagem do coagulante
    VerificandoPh,                 // Verificação de pH após dosagem de químicos
    DosandoAlcalinizante,          // Dosagem de alcalinizante
    DosandoSanitizante,            // Dosagem de sanitizante
    PreparaCoagulacao,             // Homogeneização antes da coagulação e decantação dos flocos
    Decantacao,                    // Aguardando a decantação dos flocos
    PreparandoLibercaoSedimentos,  // Preparando para liberar os sedimentos para o tanque de efluentes
    RemovendoSedimentos,           // Removendo sedimentos do tanque de ativos
    PreparandoEsvaziamentoAtivos,  // Preparando para liberar a agua tratada para o tanque final
    EsvaziandoTanqueAtivos,        // Esvaziamento do tanque de ativos
    VerificTransferenciaFinal,     // Verificação do nível do tanque final
    TratamentoUv,                  // Tratamento com luz UV
    Finalizado                     // Processo finalizado
  };

  // Converte a etapa do processo para uma string legível
  const char* etapaProcessoToString(Etapa etapa) {
    switch (etapa) {
      case Etapa::VerificNivelAltoArmaz:
        return "VerificNivelAltoArmaz";
      case Etapa::EsvaziandoTqArmaz:
        return "EsvaziandoTqArmaz";
      case Etapa::VerificTransferenciaAtivos:
        return "VerificTransferenciaAtivos";
      case Etapa::DosandoCoagulante:
        return "DosandoCoagulante";
      case Etapa::VerificandoPh:
        return "VerificandoPh";
      case Etapa::DosandoAlcalinizante:
        return "DosandoAlcalinizante";
      case Etapa::PreparaCoagulacao:
        return "PreparaCoagulacao";
      case Etapa::Decantacao:
        return "Decantacao";
      case Etapa::PreparandoLibercaoSedimentos:
        return "PreparandoLibercaoSedimentos";
      case Etapa::RemovendoSedimentos:
        return "RemovendoSedimentos";
      case Etapa::DosandoSanitizante:
        return "DosandoSanitizante";
      case Etapa::PreparandoEsvaziamentoAtivos:
        return "PreparandoEsvaziamentoAtivos";
      case Etapa::EsvaziandoTanqueAtivos:
        return "EsvaziandoTanqueAtivos";
      case Etapa::VerificTransferenciaFinal:
        return "VerificTransferenciaFinal";
      case Etapa::TratamentoUv:
        return "TratamentoUv";
      case Etapa::Finalizado:
        return "Finalizado";
      default:
        return "Inicial";
    }
  }

  Etapa etapaAtual = Etapa::Inicial;     // Variável para armazenar a etapa atual do processo
  Etapa controleEtapa = etapaAtual;      // Variável para armazenar a etapa anterior do processo
  Etapa controleParada = controleEtapa;  // Variável utilizada para garantir que a etapa atual do processo finalize caso uma parada seja acionada
};