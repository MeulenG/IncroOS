using System.Runtime.InteropServices;
using Incro.Runtime;
using IncroOS.Lib;

namespace IncroOS.X86
{
    // kernel/cpu/gdt.c: a flat GDT with kernel and user code and data segments, loaded with lgdt, then
    // CS reloaded with a far return and the data segment registers with the kernel data selector
    internal static unsafe class Gdt
    {
        // SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_LONG / SEG_SIZE | SEG_GRAN(1) | SEG_PRIV(n) | type, see gdt.h
        public const ushort CodePl0 = 0xA09A;
        public const ushort DataPl0 = 0xC092;
        public const ushort CodePl3 = 0xA0FA;
        public const ushort DataPl3 = 0xC0F2;

        private const ushort KernelCode = 0x08;
        private const ushort KernelData = 0x10;

        private struct Table
        {
            public fixed ulong Entries[5];
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct Gdtr
        {
            public ushort Limit;   // size of the GDT in bytes minus 1
            public ulong Base;
        }

        private static Table table;

        public static int CreateDescriptor(int slot, uint baseAddress, uint limit, ushort flag)
        {
            // the high 32 bits: limit 19:16, type, p, dpl, s, g, d/b, l, avl, base 23:16 and 31:24
            ulong descriptor = limit & 0x000F0000;
            descriptor |= (uint)((flag << 8) & 0x00F0FF00);
            descriptor |= (baseAddress >> 16) & 0x000000FF;
            descriptor |= baseAddress & 0xFF000000;
            descriptor <<= 32;

            // the low 32 bits: base 15:0 and limit 15:0
            descriptor |= baseAddress << 16;
            descriptor |= limit & 0x0000FFFF;

            fixed (ulong* gdt = table.Entries)
            {
                gdt[slot] = descriptor;
            }
            return 0;
        }

        public static int Initialize()
        {
            CreateDescriptor(0, 0, 0x00000000, 0);
            CreateDescriptor(1, 0, 0xFFFFF, CodePl0);
            CreateDescriptor(2, 0, 0xFFFFF, DataPl0);
            // user data before user code, as sysret expects
            CreateDescriptor(3, 0, 0xFFFFF, DataPl3);
            CreateDescriptor(4, 0, 0xFFFFF, CodePl3);

            Gdtr gdtr;
            fixed (ulong* gdt = table.Entries)
            {
                gdtr.Base = (ulong)gdt;
            }
            gdtr.Limit = 5 * 8 - 1;
            Cpu.LoadGdt(&gdtr);
            Cpu.ReloadSegments(KernelCode, KernelData);
            return 0;
        }

        // The flags live in the high 32 bits; the accessed bit (8) is masked out, the CPU sets it
        private static bool FlagsMatch(ulong descriptor, ushort flag) =>
            ((uint)(descriptor >> 32) & 0x00F0FE00) == (((uint)flag << 8) & 0x00F0FE00);

        public static int Validate()
        {
            fixed (ulong* gdt = table.Entries)
            {
                if (gdt[0] != 0)
                {
                    Print.Format("GDT validation failed: Null descriptor is not zero\n");
                    return -1;
                }
                if (!FlagsMatch(gdt[1], CodePl0))
                {
                    Print.Format("GDT validation failed: Kernel code segment descriptor is incorrect\n");
                    return -1;
                }
                if (!FlagsMatch(gdt[2], DataPl0))
                {
                    Print.Format("GDT validation failed: Kernel data segment descriptor is incorrect\n");
                    return -1;
                }
                if (!FlagsMatch(gdt[3], DataPl3))
                {
                    Print.Format("GDT validation failed: User data segment descriptor is incorrect\n");
                    return -1;
                }
                if (!FlagsMatch(gdt[4], CodePl3))
                {
                    Print.Format("GDT validation failed: User code segment descriptor is incorrect\n");
                    return -1;
                }
            }
            Print.Format("GDT validation passed!\n");
            return 0;
        }
    }
}
