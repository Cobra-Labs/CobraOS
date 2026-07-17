# CobraOS

CobraOS is a modern, high-performance x86_64 microkernel operating system written from scratch in C++. It is designed with a strict focus on system stability, security, and a unified ecosystem to provide a clean alternative to monolithic kernel architectures.

## Architecture Vision

Unlike monolithic kernels (such as Linux or Windows NT) where drivers and file systems run with full privileges in Kernel space, CobraOS isolates system services into independent Userspace (Ring 3) processes. This design philosophy drastically reduces the attack surface, improves system resilience, and makes the kernel auditable and maintainable.

* **Microkernel Architecture:** The core kernel (Ring 0) is strictly under 10,000 lines of code, handling only essential primitives: Thread scheduling, Virtual Memory Management (Paging), and Inter-Process Communication (IPC).
* **POSIX Compliance:** CobraOS uses standard Unix paths (`/home/`, `/bin/`, `/dev/`) and natively executes **ELF64** binaries linked against a custom-ported standard C library.
* **Isolating Legacy (Win32 Layer):** To maintain a clean ecosystem while ensuring compatibility, Windows PE support is treated as an optional Userspace translation layer (similar to Wine). Win32 applications run isolated in their own address spaces without compromising OS stability.

## Current Status

### ✅ Implemented Features

* **Bootloader:** Seamless integration with the modern **Limine Bootloader** (x86_64 BIOS/UEFI).
* **Memory Management:** Advanced Paging infrastructure implementing strict **W^X (Write XOR Execute)** permissions and **NX (No-Execute)** bits on page tables to prevent buffer-overflow exploits.
* **CPU Initialization:** Custom Global Descriptor Table (GDT), Task State Segment (TSS) setup, and Interrupt handling.
* **System Calls:** High-efficiency syscall dispatching adhering closely to the **System V AMD64 ABI** (Linux calling convention) for optimal compiler compatibility.
* **Process Management:** Basic process creation, address space isolation, and ELF binary loading.
* **Framebuffer Support:** Direct framebuffer access via Limine bootloader with syscall interface for userspace display servers.
* **Minimal libc:** Custom C library with syscall wrappers, CRT0 initialization, and basic I/O (write syscall).
* **Init Process (PID 1):** Userspace init process framework ready for service spawning and orchestration.

### 🔄 In Progress

* **Virtual File System (VFS) / EXT2 Support (Read-Only):** Near completion. Filesystem server will run as a userspace service, handling file I/O for other processes.

### 📋 Roadmap to v1.0

- [ ] **Finish VFS/EXT2 Server** – Complete read-only filesystem support and prepare for read-write capabilities
- [x] **Core IPC Performance Optimization** – Syscall infrastructure is in place
- [x] **Porting a minimal `libc`** – Basic libc is implemented
- [ ] **Native GUI Compositor (Userspace Display Server)** – Window manager and GUI framework needed (see [Issue #2](https://github.com/Cobra-Labs/CobraOS/issues/2))
- [ ] **The Doom Milestone:** Running an ELF64 port of Doom natively
- [ ] **PE Loader & Win32 API Subsystem Prototype** – Win32 compatibility layer as userspace service

## Building & Running

### Prerequisites
- Meson build system
- x86_64 C++ compiler (with bare-metal support, e.g., `x86_64-elf-g++`)
- `xorriso` (ISO creation)
- `limine` (bootloader tools)

### Build
```bash
meson setup build
meson compile -C build
```

### Run in QEMU
```bash
qemu-system-x86_64 -cdrom build/cobraos.iso
```

## Repository Structure

```
src/
  kernel/          Microkernel core (pmm, vmm, process mgmt)
  arch/            x86_64 architecture (GDT, IDT, APIC, TSS, syscalls)
  paging/          Physical Memory Manager (PMM) and Virtual Memory Manager (VMM)
  include/         Kernel headers and interfaces

libc/              Minimal C library for userspace
  include/         syscall.h, unistd.h
  src/             crt0.S, syscall.S, unistd.cpp

userspace/         Userspace services and applications
  init.cpp         Init process (PID 1)

esp/               UEFI/BIOS boot configuration
initramfs/         Initial ramdisk containing init binary and services

meson.build        Build configuration
```

## Contributing

We're actively seeking contributors, especially:
- **GUI / Window System Developers** – Help us build the GUI compositor and window manager (see [Issue #2](https://github.com/Cobra-Labs/CobraOS/issues/2))
- **Filesystem Service Developers** – Complete VFS/EXT2 server implementation
- **System Service Authors** – Network stack, device drivers, and other userspace services
- **Documentation & Testing** – Help document APIs and improve test coverage

## License

This project is licensed under the **GNU General Public License v3.0** (GPL-3.0) - see the [LICENSE](LICENSE) file for details. Developed by Cobra-Labs.
