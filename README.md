# mqtt_dsr_agent

[![License](https://img.shields.io/badge/License-Apache%202.0-green.svg)](https://opensource.org/licenses/Apache-2.0)
[![Build](https://github.com/grupo-avispa/mqtt_dsr_agent/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/grupo-avispa/mqtt_dsr_agent/actions/workflows/build.yml)

## Overview

This is a simple MQTT agent that listens to a topic and sends the received messages to the DSR graph.

## Installation

### Building from Source

#### Building

To build from source, clone the latest version from this repository and compile the package using the following command:
```bash
cd $HOME
git clone https://github.com/grupo-avispa/mqtt_dsr_agent.git
cd mqtt_dsr_agent
mkdir -p build && cd build
cmake .. && make -j4 && sudo make install