using IncroOS.Drivers;

namespace IncroOS.Memory
{
    // kernel/memory/kmalloc.c: a first-fit list of blocks, each [header][data], tiling the heap. The heap
    // starts just above the bootloader's 4 GiB identity map and grows on demand with frames from the PMM.
    // It also backs incro_alloc, so the C# heap (Incro.CoreLib's collector) gets its chunks from here.
    internal static unsafe class KMalloc
    {
        private struct BlockHeader
        {
            public ulong Size;
            public bool IsFree;
            public BlockHeader* Next;
        }

        private const ulong HeapVirtStart = 0x100000000UL;
        private const ulong HeapVirtMax = HeapVirtStart + 0x10000000UL;   // 256 MiB
        private const ulong HeapInitialSize = 0x10000;

        private static ulong HeaderSize => (ulong)sizeof(BlockHeader);

        private static BlockHeader* heapStart;
        private static ulong heapEnd = HeapVirtStart;   // the first unmapped address
        private static ulong totalAllocated;

        private static ulong AlignSize(ulong size) => (size + 15) & ~15UL;

        // Maps at least size more bytes at the end of the heap; the bytes mapped, or 0 on failure
        private static ulong Grow(ulong size)
        {
            ulong grow = (size + Pmm.PageSize - 1) & ~(Pmm.PageSize - 1);
            if (grow > HeapVirtMax - heapEnd)
            {
                return 0;
            }
            for (ulong offset = 0; offset < grow; offset += Pmm.PageSize)
            {
                ulong physical = Pmm.AllocatePage();
                if (physical == 0 || !Vmm.MapPage(heapEnd + offset, physical, Vmm.Writable))
                {
                    if (physical != 0)
                    {
                        Pmm.FreePage(physical);
                    }
                    // undo the pages mapped so far
                    for (ulong undo = 0; undo < offset; undo += Pmm.PageSize)
                    {
                        Pmm.FreePage(Vmm.GetPhysical(heapEnd + undo));
                        Vmm.UnmapPage(heapEnd + undo);
                    }
                    return 0;
                }
            }
            heapEnd += grow;
            return grow;
        }

        public static int Initialize()
        {
            if (Grow(HeapInitialSize) == 0)
            {
                Serial.WriteString("[KMALLOC] Failed to map the initial heap\n");
                return -1;
            }
            heapStart = (BlockHeader*)HeapVirtStart;
            heapStart->Size = HeapInitialSize - HeaderSize;
            heapStart->IsFree = true;
            heapStart->Next = null;

            Serial.WriteString("[KMALLOC] Kernel heap allocator initialized\n");
            return 0;
        }

        public static void* Allocate(ulong size)
        {
            if (size == 0)
            {
                return null;
            }
            size = AlignSize(size);

            for (BlockHeader* current = heapStart; current != null; current = current->Next)
            {
                if (current->IsFree && current->Size >= size)
                {
                    if (current->Size >= size + HeaderSize + 16)
                    {
                        var split = (BlockHeader*)((byte*)current + HeaderSize + size);
                        split->Size = current->Size - size - HeaderSize;
                        split->IsFree = true;
                        split->Next = current->Next;

                        current->Size = size;
                        current->Next = split;
                    }
                    current->IsFree = false;
                    totalAllocated += current->Size;
                    return (byte*)current + HeaderSize;
                }
            }

            // no free block fits, so extend the heap and retry
            BlockHeader* last = heapStart;
            while (last->Next != null)
            {
                last = last->Next;
            }
            ulong grown = Grow(last->IsFree ? size - last->Size : size + HeaderSize);
            if (grown == 0)
            {
                return null;
            }
            // blocks tile the heap, so the last block always ends where the new pages begin
            if (last->IsFree)
            {
                last->Size += grown;
            }
            else
            {
                var block = (BlockHeader*)((byte*)last + HeaderSize + last->Size);
                block->Size = grown - HeaderSize;
                block->IsFree = true;
                block->Next = null;
                last->Next = block;
            }
            return Allocate(size);
        }

        public static int Free(void* pointer)
        {
            if (pointer == null)
            {
                return 0;
            }
            var block = (BlockHeader*)((byte*)pointer - HeaderSize);
            if (block->IsFree)
            {
                return 0;
            }
            block->IsFree = true;
            totalAllocated -= block->Size;

            if (block->Next != null && block->Next->IsFree)
            {
                block->Size += HeaderSize + block->Next->Size;
                block->Next = block->Next->Next;
            }

            BlockHeader* current = heapStart;
            while (current != null && current->Next != block)
            {
                current = current->Next;
            }
            if (current != null && current->IsFree)
            {
                current->Size += HeaderSize + block->Size;
                current->Next = block->Next;
            }
            return 0;
        }

        public static ulong Used => totalAllocated;

        public static ulong FreeBytes
        {
            get
            {
                ulong free = 0;
                for (BlockHeader* current = heapStart; current != null; current = current->Next)
                {
                    if (current->IsFree)
                    {
                        free += current->Size;
                    }
                }
                return free;
            }
        }

        public static int Validate()
        {
            ulong usedBefore = Used;

            // larger than the initial heap, so this has to grow it
            ulong size = 2 * 1024 * 1024;
            var buffer = (byte*)Allocate(size);
            if (buffer == null)
            {
                Serial.WriteString("KMALLOC validation failed: could not grow the heap\n");
                return -1;
            }

            // a distinct value per page catches unmapped pages and pages mapped to the same frame
            for (ulong offset = 0; offset < size; offset += Pmm.PageSize)
            {
                buffer[offset] = (byte)(offset / Pmm.PageSize);
            }
            for (ulong offset = 0; offset < size; offset += Pmm.PageSize)
            {
                if (buffer[offset] != (byte)(offset / Pmm.PageSize))
                {
                    Serial.WriteString("KMALLOC validation failed: heap pages alias each other\n");
                    return -1;
                }
            }

            void* small = Allocate(32);
            if (small == null)
            {
                Serial.WriteString("KMALLOC validation failed: small allocation after growth\n");
                return -1;
            }
            Free(small);
            Free(buffer);

            if (Used != usedBefore)
            {
                Serial.WriteString("KMALLOC validation failed: used bytes did not return to baseline\n");
                return -1;
            }
            Serial.WriteString("KMALLOC validation passed!\n");
            return 0;
        }
    }
}
