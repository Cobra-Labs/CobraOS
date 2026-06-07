#include "tss.h"
#include "gdt.h"

void TSSManager::init(uint64_t kernel_stack, GDTManager* gdt) {
	tss = {};
    tss.rsp0 = kernel_stack;
	gdt->set_tss(&tss);
}
