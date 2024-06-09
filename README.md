<p align="center">
    <img width="400" src="./docs/img/logo-light.png#gh-light-mode-only" alt="mintDB Logo">
    <img width="400" src="./docs/img/logo.png#gh-dark-mode-only" alt="mintDB Logo">
</p>
<h2 align="center">A Load Balancer for mintdb stack</h2>
<p align="center">
    <img src="https://img.shields.io/badge/version-0.1.0-000000">
    <img src="https://img.shields.io/docker/pulls/eddique/mintdb-stack?color=1DA1F2">
    <img src="https://img.shields.io/badge/built%20with-C-00427A.svg">
    <img src="https://img.shields.io/badge/license-MIT-750014">
</p>

## Quick Start

```sh
make
./bin/mintdb-pool -p 4321 -r 3 -n mintdb
```

## Usage

```sh
mint -p PORT_NUMBER -r MINTDB_REPLICA_NUMBER -n MINTDB_SERVICE_NAME_PREFIX
```

## Environment
```sh
TARGET_HOST_PREFIX=TARGET_HOST_PREFIX
TARGET_PORT=TARGET_SERVER_PORT
REPLICAS=NUM_REPLICAS
PORT=SERVER_PORT
```