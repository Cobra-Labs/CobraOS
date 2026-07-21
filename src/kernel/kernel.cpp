#include <stdint.h>
#include <stddef.h>
#include "../paging/pmm.h"
#include "../paging/vmm.h"
#include "../arch/gdt.h"
#include "../arch/idt.h"
#include "../arch/apic.h"
#include "../arch/tss.h"
#include "../arch/syscall.h"
#include "process.h"
#include "tar.h"
#include "elf.h"
#include "../drivers/virtio_blk.h"
#include "../fs/ext2.h"

// --- Limine Setup ---
#define LIMINE_REQ __attribute__((used, section(".limine_requests")))
#define LIMINE_COMMON_MAGIC 0xc7b1dd30df4c8b88, 0x0a82e883a194f07b

struct limine_file {
    uint64_t revision;
    void* address;
    uint64_t size;
    char* path;
    char* cmdline;
    uint32_t media_type;
    uint32_t unused;
    uint32_t tftp_ip;
    uint32_t tftp_port;
    uint32_t partition_index;
    uint32_t mbr_disk_id;
    struct { uint64_t a, b; } gpt_disk_uuid;
    struct { uint64_t a, b; } gpt_part_uuid;
    struct { uint64_t a, b; } part_uuid;
};

struct limine_framebuffer {
    void* address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
    uint16_t bpp;
    uint8_t memory_model;
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;
    uint8_t unused[7];
    uint64_t edid_size;
    void* edid;
};

struct limine_framebuffer_response {
    uint64_t revision;
    uint64_t framebuffer_count;
    struct limine_framebuffer** framebuffers;
};

struct limine_framebuffer_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_framebuffer_response* response;
};

static volatile struct limine_framebuffer_request framebuffer_request LIMINE_REQ = {
    .id = { LIMINE_COMMON_MAGIC, 0x9d5827dcd881dd75, 0xa3148604f6fab11b },
    .revision = 0, .response = nullptr
};

struct limine_module_response {
    uint64_t revision;
    uint64_t module_count;
    struct limine_file** modules;
};

struct limine_module_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_module_response* response;
};

static volatile struct limine_module_request module_request LIMINE_REQ = {
    .id = { LIMINE_COMMON_MAGIC, 0x3e7e279702be32af, 0xca1c4f3bd1280cee },
    .revision = 0, .response = nullptr
};

__attribute__((used)) static volatile uint64_t limine_base_revision[3] = {
    0xfcf5f7c9d1158b2f, 0x16b628fc559ec7c5, 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker = 0;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker = 0;

static volatile struct limine_memmap_request memmap_request LIMINE_REQ = {
    .id = { LIMINE_COMMON_MAGIC, 0x67cf3d9d378a806f, 0xe304acdfc50c3c62 },
    .revision = 0, .response = nullptr
};

static volatile struct limine_hhdm_request hhdm_request LIMINE_REQ = {
    .id = { LIMINE_COMMON_MAGIC, 0x48dcf1cb8ad2b852, 0x63984e959a98244b },
    .revision = 0, .response = nullptr
};

struct limine_kernel_address_response {
    uint64_t revision;
    uint64_t physical_base;
    uint64_t virtual_base;
};

struct limine_kernel_address_request {
    uint64_t id[4];
    uint64_t revision;
    limine_kernel_address_response* response;
};

static volatile struct limine_kernel_address_request kaddr_request LIMINE_REQ = {
    .id = { LIMINE_COMMON_MAGIC, 0x71ba76863cc55f63, 0xb2644a48c516a487 },
    .revision = 0, .response = nullptr
};

void userspace_test() {
    while (true) asm volatile("nop");
}

static uint8_t pmm_storage[sizeof(PMM)];
PMM* volatile g_pmm = nullptr;

static uint8_t vmm_storage[sizeof(VMM)];
VMM* volatile g_vmm = nullptr;

extern "C" [[noreturn]] void kmain(void) {

    if (memmap_request.response == nullptr || hhdm_request.response == nullptr) {
        while (true) asm volatile("hlt");
    }

    for (size_t i = 0; i < sizeof(PMM); i++) pmm_storage[i] = 0;
    g_pmm = (PMM*)pmm_storage;

    g_pmm->init(
        (limine_memmap_response*)memmap_request.response,
                ((limine_hhdm_response*)hhdm_request.response)->offset
    );

    for (size_t i = 0; i < sizeof(VMM); i++) vmm_storage[i] = 0;
    g_vmm = (VMM*)vmm_storage;

    g_vmm->init(g_pmm);

    static uint8_t gdt_storage[sizeof(GDTManager)];
    GDTManager* volatile g_gdt = nullptr;
    for (size_t i = 0; i < sizeof(GDTManager); i++) gdt_storage[i] = 0;
    g_gdt = (GDTManager*)gdt_storage;

    g_gdt->init();

    static uint8_t idt_storage[sizeof(IDTManager)];
    IDTManager* volatile g_idt = nullptr;
    for (size_t i = 0; i < sizeof(IDTManager); i++) idt_storage[i] = 0;
    g_idt = (IDTManager*)idt_storage;

    g_idt->init();

    // Kernel mappen
    for (size_t i = 0; i < memmap_request.response->entry_count; i++) {
        limine_memmap_entry* entry = (limine_memmap_entry*)memmap_request.response->entries[i];
        if (entry->type == 6) {
            uint64_t phys = entry->base;
            uint64_t virt = phys + kaddr_request.response->virtual_base
            - kaddr_request.response->physical_base;
            for (uint64_t off = 0; off < entry->length; off += 4096) {
                g_vmm->map_page(virt + off, phys + off, PAGE_PRESENT | PAGE_WRITE);
            }
        }
    }

    // HHDM mappen
    uint64_t hhdm = hhdm_request.response->offset;
    for (size_t i = 0; i < memmap_request.response->entry_count; i++) {
        limine_memmap_entry* entry = (limine_memmap_entry*)memmap_request.response->entries[i];
        if (entry->type == 0 || entry->type == 5 || entry->type == 6) {
            uint64_t phys = entry->base;
            uint64_t virt = phys + hhdm;
            for (uint64_t off = 0; off < entry->length; off += 4096) {
                g_vmm->map_page(virt + off, phys + off, PAGE_PRESENT | PAGE_WRITE | PAGE_NX);
            }
        }
    }

    // Stack mappen
    uint64_t rsp;
    asm volatile("mov %%rsp, %0" : "=r"(rsp));
    uint64_t stack_phys = rsp - hhdm;
    // 64 KiB Stack mappen (nach oben und unten)
    for (uint64_t off = 0; off < 0x10000; off += 4096) {
        g_vmm->map_page(rsp - off, stack_phys - off, PAGE_PRESENT | PAGE_WRITE | PAGE_NX);
    }

    // LAPIC mappen
    g_vmm->map_page(0xFEE00000 + hhdm, 0xFEE00000, PAGE_PRESENT | PAGE_WRITE | PAGE_NX);

    g_vmm->activate();

    static uint8_t apic_storage[sizeof(APICManager)];
    APICManager* volatile g_apic = nullptr;
    for (size_t i = 0; i < sizeof(APICManager); i++) apic_storage[i] = 0;
    g_apic = (APICManager*)apic_storage;

    g_apic->init(hhdm);

    g_hhdm_offset = hhdm;

    static uint8_t kernel_stack[16384];

    static uint8_t tss_storage[sizeof(TSSManager)];
    TSSManager* volatile g_tss = nullptr;
    for (size_t i = 0; i < sizeof(TSSManager); i++) tss_storage[i] = 0;
    g_tss = (TSSManager*)tss_storage;

    g_tss->init((uint64_t)kernel_stack + sizeof(kernel_stack), g_gdt);

    g_cpu_local.kernel_stack = (uint64_t)kernel_stack + sizeof(kernel_stack);

    static uint8_t syscall_storage[sizeof(SyscallManager)];
    SyscallManager* volatile g_syscall = nullptr;
    for (size_t i = 0; i < sizeof(SyscallManager); i++) syscall_storage[i] = 0;
    g_syscall = (SyscallManager*)syscall_storage;

    g_syscall->init();

    if (module_request.response != nullptr && module_request.response->module_count > 0) {
        qemu_print("Initramfs geladen!\n");
        qemu_print_hex(module_request.response->module_count);
        qemu_print("\n");
    } else {
        qemu_print("Kein Modul gefunden!\n");
    }

    struct limine_file* mod = (struct limine_file*)module_request.response->modules[0];
    const uint8_t* tar = (const uint8_t*)mod->address;

    uint64_t file_size = 0;
    const uint8_t* file = tar_find(tar, mod->size, "./init", &file_size);

    if (file != nullptr) {
        qemu_print("init gefunden!\n");
    } else {
        qemu_print("init nicht gefunden!\n");
    }

    if (framebuffer_request.response != nullptr && framebuffer_request.response->framebuffer_count > 0) {
        limine_framebuffer* fb = framebuffer_request.response->framebuffers[0];
        qemu_print("Framebuffer gefunden: ");
        qemu_print_hex(fb->width);
        qemu_print(" x ");
        qemu_print_hex(fb->height);
        qemu_print(" @ ");
        qemu_print_hex(fb->bpp);
        qemu_print(" bpp\n");
    } else {
        qemu_print("Kein Framebuffer!\n");
    }

    // --- virtio-blk Disk initialisieren ---
    static uint8_t blk_storage[sizeof(VirtioBlkDevice)];
    VirtioBlkDevice* g_blk = (VirtioBlkDevice*)blk_storage;

    if (virtio_blk_init(g_blk, g_pmm)) {
        qemu_print("virtio-blk initialisiert!\n");

        static uint8_t sector_buf[512];
        if (virtio_blk_read(g_blk, g_pmm, 0, sector_buf)) {
            qemu_print("Sektor 0 gelesen, erste 16 Bytes: ");
            for (int i = 0; i < 16; i++) {
                qemu_print_hex(sector_buf[i]);
                qemu_print(" ");
            }
            qemu_print("\n");
        } else {
            qemu_print("virtio-blk read fehlgeschlagen!\n");
        }
    } else {
        qemu_print("virtio-blk init fehlgeschlagen!\n");
    }

    static uint8_t ext2_storage[sizeof(Ext2FS)];
    Ext2FS* g_ext2 = (Ext2FS*)ext2_storage;

    if (ext2_mount(g_ext2, g_blk, g_pmm)) {
        qemu_print("ext2 gemountet!\n");
    } else {
        qemu_print("ext2 nicht gemountet, keine weitere fehlermeldung\n");
    }
    
    uint32_t inode_num = ext2_find(g_ext2, "/hello.txt");
    qemu_print("ext2_find(/hello.txt): ");
    qemu_print_hex(inode_num);
    qemu_print("\n");

    if (inode_num != 0) {
        static uint8_t file_buf[4096];
        uint32_t len = ext2_read_file(g_ext2, inode_num, file_buf, sizeof(file_buf));
        qemu_print("gelesen: ");
        qemu_print_hex(len);
        qemu_print(" Bytes: ");
        for (uint32_t i = 0; i < len; i++) {
            char c = file_buf[i];
            char b[2] = { c, 0 };
            qemu_print(b);
        }
        qemu_print("\n");
    }
    
    static uint8_t pm_storage[sizeof(ProcessManager)];
    ProcessManager* volatile g_pm = nullptr;
    for (size_t i = 0; i < sizeof(ProcessManager); i++) pm_storage[i] = 0;
    g_pm = (ProcessManager*)pm_storage;
    g_pm->init(g_pmm, g_vmm);
    
    // --- init von ext2 laden, mit Fallback auf Tar-Initramfs ---
    uint32_t init_inode = ext2_find(g_ext2, "/init");
    qemu_print("ext2_find(/init): ");
    qemu_print_hex(init_inode);
    qemu_print("\n");

    static uint8_t init_elf_buf[65536];
    uint64_t init_file_size = 0;
    const uint8_t* init_file = nullptr;

    if (init_inode != 0) {
        uint32_t len = ext2_read_file(g_ext2, init_inode, init_elf_buf, sizeof(init_elf_buf));
        if (len > 0) {
            init_file = init_elf_buf;
            init_file_size = len;
            qemu_print("init von ext2 geladen, Groesse: ");
            qemu_print_hex(len);
            qemu_print("\n");
        }
    }

    if (init_file == nullptr) {
        qemu_print("init nicht in ext2 gefunden, Fallback auf Tar-Initramfs!\n");
        init_file = file;
        init_file_size = file_size;
    }

    if (init_file != nullptr) {
        PageTable* proc_as = g_vmm->create_address_space();
        uint64_t entry = elf_load(g_vmm, g_pmm, proc_as, init_file, init_file_size);
        if (entry == 0) {
            qemu_print("ELF ungueltig!\n");
        } else {
            uint64_t user_stack_top = 0x7FFFFFFFF000; // page-aligned, kanonisch lower half
            g_pm->start(proc_as, entry, user_stack_top);
        }
    } else {
        qemu_print("init nicht gefunden!\n");
    }

    qemu_print("CobraOS booted!\n");
    while (true) asm volatile("hlt");
}
