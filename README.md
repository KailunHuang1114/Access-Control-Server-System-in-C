# Access Control Server System in C

## Overview
This project implements a basic **Access Control System** using the **Client-Server architecture**. Written in C, the system demonstrates network programming concepts using **TCP Sockets**. It consists of a server that handles connection requests and authentication logic, and a client that interacts with the user to send credentials.

This repository serves as a practical implementation of socket programming, process/thread handling, and standard C system calls.

## Features
* **Client-Server Architecture:** Separation of duties between the verification backend and the user frontend.
* **Socket Programming:** Uses standard C libraries (`<sys/socket.h>`, `<netinet/in.h>`) for network communication.
* **Automated Build:** Includes a `makefile` for easy compilation.
* **Concurrency (Optional):** Capable of handling connection requests (depending on implementation: `fork()` or `pthread`).

## Project Structure
* **`server.c`**: The server-side program. It binds to a port, listens for incoming connections, and performs access control logic (authentication).
* **`client.c`**: The client-side program. It connects to the server and allows the user to input data (e.g., ID/Password or Access Codes).
* **`makefile`**: Automates the compilation process for both the client and server executables.

## Prerequisites
To run this project, you need a Linux/Unix environment with the following installed:
* **GCC** (GNU Compiler Collection)
* **Make** tool

## Getting Started

### 1. Clone the Repository
```bash
git clone [https://github.com/KailunHuang1114/Access-Control-Server-System-in-C.git](https://github.com/KailunHuang1114/Access-Control-Server-System-in-C.git)
cd Access-Control-Server-System-in-C
