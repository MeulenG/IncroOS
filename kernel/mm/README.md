# IncroOS Memory Manager

This directory contains the memory management subsystem for IncroOS, consisting of three main components:

## Components

### 1. Physical Memory Manager (PMM)
**Files:** `pmm.h`, `pmm.c`

The PMM manages physical page frames (4KB pages) using a bitmap allocator.

**Features:**
- Bitmap-based tracking of free/allocated pages
- Manages 32MB of physical memory starting at 0x200000 (2MB)
- Initial allocations from identity-mapped region (0x10000-0x100000) to bootstrap page table allocation
- Simple and efficient O(n) allocation

**API:**
```c
void pmm_init(void);                    // Initialize the PMM
uint64_t pmm_alloc_page(void);          // Allocate a 4KB page frame
void pmm_free_page(uint64_t addr);      // Free a page frame
size_t pmm_get_free_pages(void);        // Get number of free pages
size_t pmm_get_total_pages(void);       // Get total number of pages
```

### 2. Virtual Memory Manager (VMM)
**Files:** `vmm.h`, `vmm.c`

The VMM manages the 4-level page table structure for x86-64 long mode.

**Features:**
- Dynamic page table allocation using PMM
- Support for 4KB pages
- TLB invalidation on mapping changes
- Page table walking for virtual-to-physical translation

**API:**
```c
void vmm_init(void);                                        // Initialize the VMM
bool vmm_map_page(uint64_t virt, uint64_t phys, uint64_t flags);  // Map virtual to physical
void vmm_unmap_page(uint64_t virt);                        // Unmap a virtual address
uint64_t vmm_get_physical(uint64_t virt);                  // Get physical address
uint64_t vmm_get_cr3(void);                                 // Get current page directory base
```

**Page Table Entry Flags:**
- `PTE_PRESENT` - Page is present in memory
- `PTE_WRITABLE` - Page is writable
- `PTE_USER` - Page is accessible from user mode
- `PTE_WRITETHROUGH` - Write-through caching
- `PTE_NOCACHE` - Disable caching
- `PTE_ACCESSED` - Page has been accessed
- `PTE_DIRTY` - Page has been written to
- `PTE_HUGEPAGE` - Use huge pages (2MB/1GB)
- `PTE_GLOBAL` - Global page (not flushed on CR3 reload)

### 3. Heap Allocator
**Files:** `heap.h`, `heap.c`

The heap allocator provides `kmalloc`/`kfree` functionality for dynamic kernel memory allocation.

**Features:**
- Linked list of free blocks (first-fit algorithm)
- Block splitting and coalescing
- Dynamic heap expansion using VMM and PMM
- Starts with 1MB heap, can grow up to 16MB
- Heap located at virtual address 0x10000000 (256MB)

**API:**
```c
void heap_init(void);                   // Initialize the heap
void* kmalloc(size_t size);             // Allocate memory
void kfree(void* ptr);                  // Free memory
size_t heap_get_used(void);             // Get used heap space
size_t heap_get_free(void);             // Get free heap space
```

## Initialization Order

The memory manager components must be initialized in this specific order:

1. **PMM** - Must be initialized first to provide physical pages
2. **VMM** - Needs PMM to allocate page tables
3. **Heap** - Needs both PMM and VMM to allocate and map pages

Example from `kernel.c`:
```c
pmm_init();
vmm_init();
heap_init();
```

## Memory Layout

```
0x00000000 - 0x0000FFFF : Reserved (BIOS, etc.)
0x00010000 - 0x000FFFFF : Identity-mapped region (initial allocations)
  0x00001000 - 0x00004FFF : Initial page tables (PML4, PDPT, PD, PT)
  0x00010000 - 0x000FFFFF : Initial PMM allocation pool
0x00100000 - 0x001FFFFF : Kernel code and data
0x00200000 - 0x01FFFFFF : PMM-managed physical memory (32MB)
0x10000000 - 0x10FFFFFF : Kernel heap (up to 16MB)
```

## Limitations and Future Improvements

Current limitations:
- Only 32MB of physical memory managed
- Simple first-fit heap allocator (could be optimized)
- No memory defragmentation
- No NUMA awareness
- Page tables are not freed when no longer needed

Future improvements:
- Parse BIOS/UEFI memory map for actual available memory
- Implement buddy allocator for better physical memory management
- Add slab allocator for kernel objects
- Support for huge pages (2MB/1GB)
- Better heap allocator (e.g., Doug Lea's allocator)
- Memory statistics and debugging tools
