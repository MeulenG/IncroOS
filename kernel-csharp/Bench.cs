#if INCRO_BENCH
using Incro.Runtime;
using IncroOS.Drivers;
using IncroOS.Lib;
using IncroOS.Memory;

namespace IncroOS
{
    // kernel/bench/bench.c in C#: the same workloads in the same order, built with -DINCRO_BENCH=ON. Each
    // prints "[BENCH] <name>: <cycles>". The last one has no C counterpart: the same objects through the
    // C# heap's garbage collector instead of kmalloc.
    internal static unsafe class Bench
    {
        private sealed class Block
        {
            public long A;
            public long B;
        }

        private static ulong* pages;

        private static void Report(string name, ulong cycles) => Print.Format("[BENCH] %s: %lu\n", name, cycles);

        // Decimal digits into output, NUL-terminated; the length
        private static int FormatU64(ulong value, byte* output)
        {
            byte* temp = stackalloc byte[20];
            int n = 0;
            do
            {
                temp[n++] = (byte)('0' + value % 10);
                value /= 10;
            }
            while (value > 0);
            int i = 0;
            while (n > 0)
            {
                output[i++] = temp[--n];
            }
            output[i] = 0;
            return i;
        }

        public static void Run(ulong bootCycles)
        {
            ulong checksum = 0;
            Report("boot services", bootCycles);

            // PMM: 512 pages allocated, then freed (first fit from page 0)
            pages = (ulong*)KMalloc.Allocate(512 * 8);
            ulong start = Cpu.ReadTimestamp();
            for (int i = 0; i < 512; i++)
            {
                pages[i] = Pmm.AllocatePage();
            }
            for (int i = 0; i < 512; i++)
            {
                Pmm.FreePage(pages[i]);
            }
            Report("pmm 512 alloc+free", Cpu.ReadTimestamp() - start);

            // VMM: 256 pages at 8 GiB mapped, looked up, unmapped
            const ulong baseAddress = 0x200000000UL;
            start = Cpu.ReadTimestamp();
            for (ulong i = 0; i < 256; i++)
            {
                Vmm.MapPage(baseAddress + i * Pmm.PageSize, pages[i], Vmm.Writable);
            }
            for (ulong i = 0; i < 256; i++)
            {
                checksum += Vmm.GetPhysical(baseAddress + i * Pmm.PageSize);
            }
            for (ulong i = 0; i < 256; i++)
            {
                Vmm.UnmapPage(baseAddress + i * Pmm.PageSize);
            }
            Report("vmm 256 map+lookup+unmap", Cpu.ReadTimestamp() - start);

            // kmalloc: 1000 blocks of 64 bytes, freed in reverse
            var blocks = (void**)KMalloc.Allocate(1000 * 8);
            start = Cpu.ReadTimestamp();
            for (int i = 0; i < 1000; i++)
            {
                blocks[i] = KMalloc.Allocate(64);
            }
            for (int i = 999; i >= 0; i--)
            {
                KMalloc.Free(blocks[i]);
            }
            Report("kmalloc 1000x64 alloc+free", Cpu.ReadTimestamp() - start);

            // kmalloc: 2000 blocks of 24 bytes, kept
            start = Cpu.ReadTimestamp();
            for (int i = 0; i < 2000; i++)
            {
                checksum += KMalloc.Allocate(24) != null ? 1UL : 0UL;
            }
            Report("kmalloc 2000x24 kept", Cpu.ReadTimestamp() - start);

            // Formatting: 20000 64-bit numbers to decimal
            byte* text = stackalloc byte[24];
            start = Cpu.ReadTimestamp();
            for (ulong v = 0; v < 20000; v++)
            {
                checksum += (ulong)FormatU64(v * 2654435761UL, text) + text[0];
            }
            Report("format 20000 u64", Cpu.ReadTimestamp() - start);

            // Memory: 1 MiB transformed byte by byte
            var a = (byte*)KMalloc.Allocate(1 << 20);
            var b = (byte*)KMalloc.Allocate(1 << 20);
            start = Cpu.ReadTimestamp();
            for (ulong i = 0; i < (1 << 20); i++)
            {
                b[i] = (byte)(a[i] ^ (byte)i);
            }
            Report("copy 1 MiB", Cpu.ReadTimestamp() - start);
            checksum += b[12345];
            KMalloc.Free(b);
            KMalloc.Free(a);

            // Framebuffer: 4000 glyphs through the 8x8 font
            start = Cpu.ReadTimestamp();
            for (int i = 0; i < 4000; i++)
            {
                Vesa.DrawChar(i % 100 * 8, 600, (byte)('A' + i % 26), 0x00FFFFFF);
            }
            Report("draw 4000 glyphs", Cpu.ReadTimestamp() - start);

            // C# only: 2000 objects of 24 bytes through the garbage-collected heap
            start = Cpu.ReadTimestamp();
            Block last = null;
            for (int i = 0; i < 2000; i++)
            {
                last = new Block { A = i, B = i };
            }
            Report("gc new 2000x24", Cpu.ReadTimestamp() - start);
            checksum += (ulong)last.A;

            Print.Format("[BENCH] checksum: %lu\n", checksum);
        }
    }
}
#endif
