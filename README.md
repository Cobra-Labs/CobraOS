# CobraOS

CobraOS is a modern, high-performance x86_64 microkernel operating system written from scratch in C++. It is designed with a strict focus on system stability, security, and a unified ecosystem to prevent OS fragmentation.

## Architecture Vision

Unlike monolithic kernels (such as Linux or Windows NT) where drivers and file systems run with full privileges in Kernel space, CobraOS isolates system services into independent Userspace (Ring 3) processes. 

* **Microkernel Architecture:** The core kernel (Ring 0) is strictly under 10,000 lines of code, handling only essential primitives: Thread scheduling, Virtual Memory Management (Paging), and Inter-Process Communication (IPC).
* **POSIX Compliance:** CobraOS uses standard Unix paths (`/home/`, `/bin/`, `/dev/`) and natively executes **ELF64** binaries linked against a custom-ported standard C library.
* **Isolating Legacy (Win32 Layer):** To maintain a clean ecosystem while ensuring compatibility, Windows PE support is treated as an optional Userspace translation layer (similar to Wine). Win32 API calls are translated into native POSIX system calls before reaching the kernel. If the compatibility layer crashes, the core system remains completely unaffected.

## Implemented Features

* **Bootloader:** Seamless integration with the modern **Limine Bootloader** (x86_64 response).
* **Memory Management:** Advanced Paging infrastructure implementing strict **W^X (Write XOR Execute)** permissions and **NX (No-Execute)** bits on page tables to prevent buffer-overflow exploits.
* **CPU Initialization:** Custom Global Descriptor Table (GDT), Task State Segment (TSS) setup, and Interrupt handling.
* **System Calls:** High-efficiency syscall dispatching adhering closely to the **System V AMD64 ABI** (Linux calling convention) for optimal compiler compatibility.

## Roadmap to v1.0

- [ ] Core IPC Performance Optimization
- [ ] Native Virtual File System (VFS) Server
- [ ] Porting a minimal `libc` (musl/mlibc)
- [ ] Native GUI Compositor (Userspace Display Server)
- [ ] **The Doom Milestone:** Running an ELF64 port of Doom natively
- [ ] PE Loader & Win32 API Subsystem Prototype

## License

This project is licensed under the **GNU General Public License v3.0** (GPL-3.0) - see the [LICENSE](LICENSE) file for details. Developed by Cobra-Labs.
