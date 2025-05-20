# Como rodar este projeto no Docker

Este projeto já vem pronto para ser usado em um container Docker com PlatformIO e todas as dependências para ESP8266/Arduino.

## Pré-requisitos
- Ter o [Docker](https://www.docker.com/products/docker-desktop/) instalado no seu computador.

## Passos para rodar

1. **Abra o terminal na pasta do projeto**

2. **Construa a imagem Docker:**
```sh
docker build -t esp8266-pio .
```

3. **Rode o container com acesso à porta USB (para upload no ESP):**
   - No Windows, compartilhe a porta COM (exemplo: COM5) usando o Docker Desktop (veja docs Docker para serial/USB no Windows).
   - No Linux, rode:
```sh
docker run -it --rm --device=/dev/ttyUSB0 -v %cd%:/workspace esp8266-pio
```
   - No Windows (PowerShell):
```powershell
docker run -it --rm -v ${PWD}:/workspace esp8266-pio
```
   - (No Windows, o upload direto para o ESP pode não funcionar via Docker, mas você pode compilar normalmente e copiar o firmware para gravar fora do container.)

4. **Dentro do container, compile e faça upload normalmente:**
```sh
platformio run           # Compila
platformio run -t upload # Faz upload (se USB disponível)
```

5. **Para abrir um shell no container:**
```sh
docker run -it --rm -v ${PWD}:/workspace esp8266-pio
```

## Observações
- O código-fonte do projeto fica sincronizado entre o host e o container.
- Se precisar instalar mais bibliotecas, use `platformio lib install <nome>` dentro do container.
- Para rodar scripts PowerShell de automação (gitpush.ps1), rode-os no Windows, não dentro do container.

---
Dúvidas? Só perguntar!
