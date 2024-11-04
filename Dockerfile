FROM grupoavispa/cortex:development-latest

# Install dependencies
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install --no-install-recommends -y \
        cmake \
        g++ \
        make \
        git \
        openssl \
        libssl-dev \
        nlohmann-json3-dev && \
    rm -rf /var/lib/apt/lists/*

# Build
COPY . ./mqtt_dsr_agent
WORKDIR /mqtt_dsr_agent
RUN mkdir build && \
    cd build && \
    cmake .. && \
    make install
RUN rm -rf /mqtt_dsr_agent

CMD ["/bin/bash"]