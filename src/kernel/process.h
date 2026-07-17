#pragma once
#include <stdint.h> 
#include <stddef.h>
#include "../paging/vmm.h"
#include "../paging/pmm.h"

struct Process {
	uint64_t pid;
	uint64_t entry_point;
	uint64_t stack_pointer;
	PageTable* page_table;
};

class ProcessManager {
	private:
		PMM* pmm;
		VMM* vmm;

	public:
		void init(PMM* pmm, VMM* vmm);
		void start(PageTable* address_space, uint64_t entry_point, uint64_t stack_pointer);
        void kill(uint64_t pid);
};
