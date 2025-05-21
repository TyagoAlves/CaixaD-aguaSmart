# Como Mudar Entre Versões do Projeto

Este projeto suporta três versões diferentes do firmware, cada uma com características específicas. Para alternar entre elas, siga as instruções abaixo:

## Versões Disponíveis

1. **Versão 1.0 - Básica**
   - Implementação padrão com funcionalidades essenciais
   - Arquivo base: `main.cpp`

2. **Versão 2.0 - Debug Avançado**
   - Adiciona página de depuração melhorada
   - Arquivo base: `pagina debug boa 15-54 20-05-2025.ino`

3. **Versão 3.0 - Configuração Completa**
   - Sistema completo com múltiplas páginas de configuração
   - Arquivo base: `vaidarcerto.ino`

## Como Alterar a Versão

1. Abra o arquivo `src/version_manager.h`
2. Você verá três definições de versão:
   ```cpp
   // Versão 1.0 - Básica (main.cpp padrão)
   #define VERSION_BASIC

   // Versão 2.0 - Debug Avançado (pagina debug boa 15-54 20-05-2025.ino)
   //#define VERSION_DEBUG

   // Versão 3.0 - Configuração Completa (vaidarcerto.ino)
   //#define VERSION_FULL
   ```
3. Descomente apenas a versão que deseja utilizar e comente as demais
4. Por exemplo, para usar a versão 2.0 (Debug Avançado):
   ```cpp
   // Versão 1.0 - Básica (main.cpp padrão)
   //#define VERSION_BASIC

   // Versão 2.0 - Debug Avançado (pagina debug boa 15-54 20-05-2025.ino)
   #define VERSION_DEBUG

   // Versão 3.0 - Configuração Completa (vaidarcerto.ino)
   //#define VERSION_FULL
   ```
5. Salve o arquivo e compile o projeto

## Importante

- Certifique-se de que apenas uma versão esteja descomentada por vez
- Após alterar a versão, recompile o projeto para que as mudanças tenham efeito
- Cada versão tem suas próprias características e funcionalidades específicas

## Detalhes das Versões

### Versão 1.0 - Básica
- Funcionalidades essenciais de conexão WiFi e MQTT
- Interface web simples para configuração
- Leitura básica do sensor ultrassônico

### Versão 2.0 - Debug Avançado
- Todas as funcionalidades da versão básica
- Página de debug aprimorada com mais informações
- Interface web com melhor design e usabilidade

### Versão 3.0 - Configuração Completa
- Sistema completo com servidor web em duas portas
- Página de monitoramento dedicada na porta 8080
- Suporte a armazenamento de logs no sistema de arquivos SPIFFS
- Tratamento avançado de interrupções para o botão FLASH