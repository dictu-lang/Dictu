FROM ubuntu:latest

RUN adduser vscode && \
    mkdir -p /etc/sudoers.d && \
    echo vscode ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/vscode && \
    chmod 0440 /etc/sudoers.d/vscode

WORKDIR Dictu
ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt upgrade -y && \
    apt install -y --no-install-recommends \
        build-essential \
        git cmake \
        libcurl4-gnutls-dev \
        uuid-dev \
        ca-certificates && \
	rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/dictu-lang/Dictu.git && \
    cd Dictu && \
	cmake -DCMAKE_BUILD_TYPE=Release -B build && \
	cmake --build ./build && \
	./dictu tests/runTests.du && \
	cp dictu /usr/bin/ && \
	&& rm -rf * 

USER root
