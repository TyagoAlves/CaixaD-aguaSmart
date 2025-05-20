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

## Passos para rodar

1. **Abra o terminal na pasta do projeto**

2. **Construa a imagem Docker:**
```sh
docker build -t esp8266-pio .
```

3. **Rode o container para compilar o projeto:**
   - No Windows (PowerShell):
> O Docker Desktop para Windows não permite mapear a porta COM diretamente para upload no ESP. Você pode compilar normalmente dentro do container, mas o upload deve ser feito fora do Docker (usando o PlatformIO instalado no Windows ou outra ferramenta).
```powershell
docker run -it --rm -v "${PWD}:/workspace" esp8266-pio
```
   - No Linux, rode (com suporte a upload):
```sh
docker run -it --rm --device=/dev/ttyUSB0 -v $(pwd):/workspace esp8266-pio
```

4. **Dentro do container, compile normalmente:**
```sh
platformio run           # Compila
```

5. **Para fazer upload para o ESP no Windows:**
- Saia do container e use o PlatformIO instalado no Windows, ou outro gravador, para enviar o firmware gerado em `.pio/build/nome-da-plataforma/firmware.bin` para a porta COM do seu ESP.

## Observações
- O código-fonte do projeto fica sincronizado entre o host e o container.
- Se precisar instalar mais bibliotecas, use `platformio lib install <nome>` dentro do container.
- Para rodar scripts PowerShell de automação (gitpush.ps1), rode-os no Windows, não dentro do container.

---
Dúvidas? Só perguntar!
