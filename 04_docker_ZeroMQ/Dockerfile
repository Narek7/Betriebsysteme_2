# Verwende ein leichtes Linux-Image
FROM debian:bullseye

# Installiere benötigte Tools und ZeroMQ
RUN apt-get update && apt-get install -y \
    build-essential \
    libzmq3-dev \
    gcc \
    && rm -rf /var/lib/apt/lists/*

# Kopiere den Server- und Client-Code ins Image
COPY server.c /server.c
COPY client.c /client.c

# Kompiliere den Server und den Client
RUN gcc -o server server.c -lzmq
RUN gcc -o client client.c -lzmq

# Standard-Kommando für den Container
CMD ["/bin/bash"]
