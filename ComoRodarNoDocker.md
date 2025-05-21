# Guia de Execução do Projeto CaixaD'aguaSmart

Este guia explica como configurar e executar o projeto usando Docker, facilitando o desenvolvimento e upload de firmware para ESP8266.

## Pré-requisitos

- [Docker Desktop](https://www.docker.com/products/docker-desktop/) instalado e em execução
- Conexão USB com seu dispositivo ESP8266

## Métodos de Upload do Firmware (Recomendados em ordem)

### Método 1: PuTTY (Recomendado)

PuTTY é uma solução simples e confiável para redirecionar portas seriais:

1. **Instale o PuTTY**:
   ```powershell
   winget install --id PuTTY.PuTTY
   ```

2. **Configure o redirecionamento de porta**:
   - Abra PuTTY
   - Em "Connection > Serial", configure:
     - Serial line: COM5 (substitua pela sua porta)
     - Speed: 115200
     - Data bits: 8
     - Stop bits: 1
     - Parity: None
     - Flow control: None
   - No painel esquerdo, vá para "Connection > SSH > Tunnels"
   - Em "Source port", digite 12345
   - Em "Destination", digite localhost:COM5 ou apenas COM5
   - Selecione "Local" e "Auto"
   - Clique em "Add"
   - Volte para "Session" e salve a configuração (opcional)
   - Clique em "Open" para iniciar a conexão

3. **Execute o Docker com a porta redirecionada**:
   ```powershell
   docker run --rm -it -v "${PWD}:/workspace" -w /workspace platformio/platformio-core pio run -t upload --upload-port=socket://localhost:12345
   ```

   Se ocorrer erro de conexão recusada, tente:
   ```powershell
   # Descubra seu IP
   ipconfig
   
   # Use o IP no comando (substitua 192.168.1.x pelo seu IP real)
   docker run --rm -it -v "${PWD}:/workspace" -w /workspace platformio/platformio-core pio run -t upload --upload-port=socket://192.168.1.x:12345
   ```

### Método 2: Com2TCP (Alternativa ao PuTTY)

Se o PuTTY não funcionar, tente o Com2TCP:

1. **Instale Com2TCP**:
   - Baixe em: https://sourceforge.net/projects/com0com/files/com2tcp/
   - Instale seguindo as instruções

2. **Execute o Com2TCP**:
   ```powershell
   com2tcp --baud 115200 \\.\COM5 12345
   ```
   (Substitua COM5 pela sua porta)

3. **Execute o Docker** (mesmo comando do Método 1)

### Método 3: WinSocat

Se as opções anteriores não funcionarem:

1. **Instale o WinSocat** (escolha uma opção):
   ```powershell
   dotnet tool install -g winsocat
   ```
   ou
   ```powershell
   winget install -e --id Firejox.WinSocat
   ```

2. **Exponha a porta serial**:
   ```powershell
   winsocat "COM5" "tcp-l:12345,bind=127.0.0.1"
   ```
   (Substitua COM5 pela sua porta)

3. **Execute o Docker** (mesmo comando do Método 1)

### Método 4: Upload direto (sem Docker)

Se os métodos anteriores falharem:

1. **Instale o PlatformIO**:
   ```powershell
   pip install platformio
   ```
   ou instale a extensão PlatformIO no VS Code

2. **Execute o upload diretamente**:
   ```powershell
   pio run -t upload
   ```

## Guia Detalhado

### Começando com o projeto

1. **Clone o repositório**:
   ```powershell
   git clone https://github.com/TyagoAlves/CaixaD-aguaSmart.git
   cd CaixaD-aguaSmart
   ```

2. **Verifique o Docker Desktop**:
   - No Windows, o Docker Desktop **deve estar aberto e inicializado** antes de executar comandos
   - Aguarde até ver a mensagem "Docker Desktop is running"
   - Se aparecer erro como `ERROR: error during connect: Head "http://%2F%2F.%2Fpipe%2FdockerDesktopLinuxEngine/_ping"`, abra o Docker Desktop e tente novamente

### Solução de Problemas com PuTTY

Se você encontrar erros como:
```
ConnectionRefusedError: [Errno 111] Connection refused
serial.serialutil.SerialException: Could not open port socket://localhost:12345: [Errno 111] Connection refused
```

Verifique:
1. Se o PuTTY está configurado corretamente e a conexão está ativa
2. Se você está usando o endereço correto (tente `localhost`, `127.0.0.1` ou seu IP real)
3. Se a porta 12345 não está sendo bloqueada pelo firewall
4. Se o redirecionamento está configurado corretamente em "Connection > SSH > Tunnels"

### Detalhes sobre WinSocat

Se optar pelo WinSocat, observe:

- **Versão dotnet/Firejox**: Sintaxe mais simples, mas recursos limitados
  ```powershell
  winsocat "COM5" "tcp-l:12345,bind=127.0.0.1"
  ```

- **Versão GnuWin32**: Mais recursos, mas sintaxe mais complexa
  ```powershell
  winsocat.exe -d -d pty,link=\\.\COM5,raw,echo=0 tcp-listen:12345,bind=127.0.0.1
  ```

- **Requisito .NET 6.0**: Se necessário, instale:
  ```powershell
  winget install --id Microsoft.DotNet.DesktopRuntime.6 --source winget
  ```
  ou baixe em: https://aka.ms/dotnet-core-applaunch?framework=Microsoft.NETCore.App&framework_version=6.0.0&arch=x64&rid=win-x64&os=win10

### Identificando a porta COM

Para identificar a porta COM correta:
1. Abra o Gerenciador de Dispositivos do Windows
2. Expanda "Portas (COM e LPT)"
3. Localize seu dispositivo ESP8266 (geralmente aparece como "USB-SERIAL CH340")
4. Veja a imagem `exemplo-com5.png` incluída no repositório para referência

## Observações Adicionais

- O código-fonte fica sincronizado entre o host e o container
- Para instalar bibliotecas adicionais: `platformio lib install <nome>` dentro do container
- Scripts PowerShell de automação devem ser executados no Windows, não no container

---

Dúvidas? Abra uma issue no GitHub ou entre em contato!