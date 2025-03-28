FROM ubuntu:22.04

# Evita prompts interactivos
ENV DEBIAN_FRONTEND=noninteractive

# Instala herramientas necesarias
RUN apt update && apt install -y \
    build-essential \
    g++ \
    gcc \
    make \
    vim \
    git \
    manpages-dev \
    && rm -rf /var/lib/apt/lists/*

# Crea un directorio de trabajo
WORKDIR /app

# Copia el contenido del proyecto al contenedor
COPY . .

# Comando por defecto (puedes cambiarlo si no usas Makefile)
CMD ["make"]

