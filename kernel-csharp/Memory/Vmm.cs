using Incro.Runtime;
using IncroOS.Drivers;

namespace IncroOS.Memory
{
    // kernel/memory/vmm.c: 4-level paging on top of the bootloader's identity map. New page tables come
    // from the PMM, which works because the first 4 GiB are identity mapped.
    internal static unsafe class Vmm
    {
        public const ulong Present = 1 << 0;
        public const ulong Writable = 1 << 1;
        public const ulong User = 1 << 2;
        public const ulong WriteThrough = 1 << 3;
        public const ulong CacheDisable = 1 << 4;

        private const ulong AddressMask = 0x000FFFFFFFFFF000UL;

        private static ulong* pml4;

        private static ulong Pml4Index(ulong address) => (address >> 39) & 0x1FF;
        private static ulong PdptIndex(ulong address) => (address >> 30) & 0x1FF;
        private static ulong PdIndex(ulong address) => (address >> 21) & 0x1FF;
        private static ulong PtIndex(ulong address) => (address >> 12) & 0x1FF;

        private static ulong* AllocatePageTable()
        {
            ulong physical = Pmm.AllocatePage();
            if (physical == 0)
            {
                return null;
            }
            var table = (ulong*)physical;
            for (int i = 0; i < 512; i++)
            {
                table[i] = 0;
            }
            return table;
        }

        private static ulong* GetOrCreateTable(ulong* parent, ulong index, ulong flags)
        {
            if ((parent[index] & Present) != 0)
            {
                return (ulong*)(parent[index] & AddressMask);
            }
            ulong* table = AllocatePageTable();
            if (table == null)
            {
                return null;
            }
            parent[index] = (ulong)table | flags | Present;
            return table;
        }

        public static int Initialize()
        {
            pml4 = (ulong*)Cpu.ReadCr3();
            Serial.WriteString("[VMM] Virtual Memory Manager initialized\n");
            return 0;
        }

        public static bool MapPage(ulong virtualAddress, ulong physicalAddress, ulong flags)
        {
            ulong* pdpt = GetOrCreateTable(pml4, Pml4Index(virtualAddress), Writable);
            if (pdpt == null)
            {
                return false;
            }
            ulong* pd = GetOrCreateTable(pdpt, PdptIndex(virtualAddress), Writable);
            if (pd == null)
            {
                return false;
            }
            ulong* pt = GetOrCreateTable(pd, PdIndex(virtualAddress), Writable);
            if (pt == null)
            {
                return false;
            }
            pt[PtIndex(virtualAddress)] = (physicalAddress & AddressMask) | flags | Present;
            Cpu.InvalidatePage((void*)virtualAddress);
            return true;
        }

        // The page table entry of a mapped address, or null when a level is missing
        private static ulong* Walk(ulong virtualAddress)
        {
            ulong pml4Entry = pml4[Pml4Index(virtualAddress)];
            if ((pml4Entry & Present) == 0)
            {
                return null;
            }
            var pdpt = (ulong*)(pml4Entry & AddressMask);
            ulong pdptEntry = pdpt[PdptIndex(virtualAddress)];
            if ((pdptEntry & Present) == 0)
            {
                return null;
            }
            var pd = (ulong*)(pdptEntry & AddressMask);
            ulong pdEntry = pd[PdIndex(virtualAddress)];
            if ((pdEntry & Present) == 0)
            {
                return null;
            }
            var pt = (ulong*)(pdEntry & AddressMask);
            return pt + PtIndex(virtualAddress);
        }

        public static int UnmapPage(ulong virtualAddress)
        {
            ulong* entry = Walk(virtualAddress);
            if (entry == null)
            {
                return -1;
            }
            *entry = 0;
            Cpu.InvalidatePage((void*)virtualAddress);
            return 0;
        }

        // The physical address, or 0 when it is not mapped
        public static ulong GetPhysical(ulong virtualAddress)
        {
            ulong* entry = Walk(virtualAddress);
            if (entry == null || (*entry & Present) == 0)
            {
                return 0;
            }
            return (*entry & AddressMask) + (virtualAddress & 0xFFF);
        }

        public static bool IsMapped(ulong virtualAddress) => GetPhysical(virtualAddress) != 0;
    }
}
