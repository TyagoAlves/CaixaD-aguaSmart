# Dockerfile para ambiente PlatformIO/Arduino
FROM python:3.11-slim

# Instala dependências básicas
RUN apt-get update && \
    apt-get install -y git curl && \
    pip install --upgrade pip

# Instala PlatformIO
RUN pip install platformio

# Cria diretório de trabalho
WORKDIR /workspace

# Copia todos os arquivos do projeto para dentro do container
COPY . /workspace

# Instala dependências do projeto (caso existam)
RUN platformio run --target clean || true

# Comando padrão: abre um shell interativo
CMD ["bash"]
