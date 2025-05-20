# Como rodar este projeto no Docker

## 1. Baixe o projeto do GitHub
Se ainda não baixou/clonou o projeto, rode no terminal:
```powershell
git clone https://github.com/TyagoAlves/CaixaD-aguaSmart.git
cd CaixaD-aguaSmart
```

---

## Importante: Docker Desktop precisa estar aberto no Windows!
Se você está no Windows, é obrigatório abrir o aplicativo "Docker Desktop" e aguardar até aparecer "Docker Desktop is running" antes de rodar qualquer comando Docker. Caso contrário, o Docker não será reconhecido pelo terminal.

Se aparecer erro como:
```
ERROR: error during connect: Head "http://%2F%2F.%2Fpipe%2FdockerDesktopLinuxEngine/_ping": open //./pipe/dockerDesktopLinuxEngine: O sistema não pode encontrar o arquivo especificado.
```
Isso significa que o Docker Desktop não está aberto ou inicializado. Abra o Docker Desktop e tente novamente.

---

Este projeto já vem pronto para ser usado em um container Docker com PlatformIO e todas as dependências para ESP8266/Arduino.

## Pré-requisitos
- Ter o [Docker](https://www.docker.com/products/docker-desktop/) instalado no seu computador.

## Passo a passo para upload do firmware via Docker no Windows

### 1. Instale o winget (se ainda não tiver)

O `winget` é o gerenciador de pacotes oficial do Windows. Ele facilita a instalação de programas via linha de comando.

- No Windows 10/11 atualizado, o `winget` já deve estar disponível. Para testar, abra o PowerShell e digite:
  ```powershell
  winget --version
  ```
- Se aparecer a versão, prossiga. Se der erro, atualize o Windows ou instale o [App Installer](https://apps.microsoft.com/store/detail/app-installer/9NBLGGH4NNS1) pela Microsoft Store.

### 2. Instale o WinSocat usando o winget

Tente primeiro o comando abaixo (recomendado):

```powershell
winget install -e --id Firejox.WinSocat
```

Se não funcionar, tente esta alternativa:

```powershell
winget install GnuWin32.WinSocat
```

Aguarde a instalação ser concluída.

### 3. Exponha a porta serial do Windows para o Docker usando o WinSocat

- Identifique o nome da porta COM do seu ESP8266 no Gerenciador de Dispositivos (exemplo: COM5).
- Execute o comando abaixo, substituindo `COM5` pelo nome correto da sua porta:

```powershell
winsocat.exe -d -d pty,link=\\.\COM5,raw,echo=0 tcp-listen:12345,bind=127.0.0.1
```

Deixe essa janela aberta enquanto usar o Docker.

### 4. Execute o container Docker mapeando a porta virtual

No PowerShell, rode o comando abaixo (ajuste o caminho do projeto se necessário):

```powershell
docker run --rm -it -v "${PWD}:/workspace" -w /workspace platformio/platformio-core pio run -t upload --upload-port=socket://host.docker.internal:12345
```

- Certifique-se de que o Docker Desktop está em execução.
- Se o caminho do seu projeto tiver espaços, mantenha as aspas.

### 5. Se não funcionar: faça o upload fora do Docker

Se o upload via Docker/socat não funcionar (por exemplo, por limitações do Windows ou do seu hardware), faça o upload do firmware usando o PlatformIO instalado diretamente no Windows ou outra ferramenta de sua preferência.

- Você pode instalar o PlatformIO como extensão do VS Code ou via pip:
  ```powershell
  pip install platformio
  ```
- Depois, rode:
  ```powershell
  pio run -t upload
  ```

---

Dessa forma, siga a ordem: instale o que precisa, tente o método automatizado, e, se não der certo, use a alternativa fora do Docker. Para identificar a porta COM correta, veja a imagem `exemplo-com5.png` incluída neste repositório.

## Observações
- O código-fonte do projeto fica sincronizado entre o host e o container.
- Se precisar instalar mais bibliotecas, use `platformio lib install <nome>` dentro do container.
- Para rodar scripts PowerShell de automação (gitpush.ps1), rode-os no Windows, não dentro do container.

---
Dúvidas? Só perguntar!
