<p align="center">
    <img width="400" src="./docs/img/logo-light.png#gh-light-mode-only" alt="mintDB Logo">
    <img width="400" src="./docs/img/logo.png#gh-dark-mode-only" alt="mintDB Logo">
</p>
<h2 align="center">An Open Source Vector Database</h2>
<p align="center">
    <img src="https://img.shields.io/badge/version-0.1.0-10d99d">
    <img src="https://img.shields.io/docker/pulls/eddique/mintdb-stack">
    <img src="https://img.shields.io/badge/built_with-Rust-dca282.svg">
    <img src="https://img.shields.io/badge/license-MIT-critical">
    <a href="https://www.linkedin.com/in/eric-rodriguez-3a402811b/"><img src="https://img.shields.io/badge/linkedIn-connect-4777AF"></a>
</p>

# Quick Start

```sh
make
./bin/mint -p 8080 -r 3 -n mintdb
```

# Usage

```sh
mint -p PORT_NUMBER -r MINTDB_REPLICA_NUMBER -n MINTDB_SERVICE_NAME_PREFIX
```