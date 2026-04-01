# AI Agent Context - PhotonRPC

This document provides architectural guidance and development standards for AI agents working on the PhotonRPC project. Adherence to these principles is mandatory for all interactions.

## 1. Project Overview
**PhotonRPC** is a high-performance RPC runtime implemented in **C++20**, designed as a communication substrate for **distributed control planes** (e.g., metadata services, coordinators, and schedulers). It features a custom application-layer protocol, a non-blocking network library based on the `Epoll` + `Reactor` pattern, and deep integration with Google Protobuf for serialization.

## 2. Tech Stack
* **Language**: C++20 (strictly enforced)
* **Build System**: CMake (>= 3.15)
* **Networking**: Epoll-based single-threaded Reactor (Roadmap: Multi-Reactor and Coroutines)
* **Serialization**: Google Protobuf (v3)
* **Logging**: Asynchronous logging via `spdlog` (MIT License)
* **Configuration**: XML parsing via `tinyxml2` (zlib License)
* **Testing**: Google Test (GTest) for units, Python scripts for integration tests
* **Formatting**: Clang-Format (refer to `.clang-format` in root)

## 3. Module Distribution
* **`include/photonrpc/` & `src/`**: Core logic and public headers.
    * **`common/`**: Foundation components (XML `config`, `logger` wrapper).
    * **`net/`**: High-performance network library (Reactor, Poller, Channel, Acceptor, TcpConnection, TcpServer, Buffer).
    * **`rpc/`**: RPC framework (RpcServer, RpcClient, RpcChannel). Uses Protobuf reflection for service routing.
* **`protobuf/`**: Protocol definitions (`rpc_message.proto`).
* **`conf/`**: Configuration files (e.g., `photonrpc.xml`).
* **`test/`**: Unit tests (`test_buffer.cc`, `test_codec.cc`), performance benchmarks (`benchmark.cc`), and integration tests (`integration_test/`).
* **`third_party/`**: Source code for external dependencies (`googletest`, `spdlog`, `tinyxml2`).

## 4. Development Conventions

### 4.1 Coding Style
* **Naming**: Generally follows Google C++ Style Guide with specific project overrides:
    * **Classes**: `CamelCase` (e.g., `RpcServer`).
    * **Methods**: `CamelCase` starting with uppercase (e.g., `StartServer`, `HandleRequest`).
    * **Member Variables**: `snake_case` with a trailing underscore (e.g., `tcp_server_`, `service_map_`).
* **Memory Management**: Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) to manage resources and avoid memory leaks.
* **Modern C++**: Leverage C++20 features to ensure idiomatic and efficient code.

### 4.2 Architecture & Logic
* **Control Plane Focus**: Prioritize low latency, predictability, and explainability over complex or "black-box" abstractions.
* **Decoupling**: The `net/` layer must remain protocol-agnostic (focusing on byte-streams and event driving). The `rpc/` layer handles protocol framing, serialization, and service dispatch.
* **Error Handling**: Utilize the logging system (`spdlog`) for feedback across `DEBUG`, `INFO`, and `ERROR` levels.

### 4.3 Build & Test Requirements
* **Output Paths**: Executables must reside in `bin/`, static libraries in `lib/`.
* **Verification**: Every code change **must** be accompanied by a new or updated test case.
* **Build Script**: Always run `auto_build.bash` in the project root to verify compilation and basic integration before finalizing changes.

---
> **AI Instruction**: Before reading or modifying project files, verify the structure and standards defined above. A task is considered complete only when `auto_build.bash` passes and all relevant tests (GTest & Python) are successful.
