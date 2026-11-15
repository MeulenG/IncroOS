using System.Runtime.InteropServices;
using Incro.Runtime;

namespace IncroOS.X86
{
    // kernel/cpu/idt.c: 256 interrupt gates, every one to the unhandled-interrupt handler except the
    // vectors the kernel handles. IncroCompiler gives each [Interrupt] method an entry stub (save the
    // registers, call, iretq); &Handler is that stub's address.
    internal static unsafe class Idt
    {
        [StructLayout(LayoutKind.Sequential)]
        private struct InterruptDescriptor
        {
            public ushort Offset1;          // offset bits 0..15
            public ushort Selector;         // a code segment selector in the GDT
            public byte Ist;                // bits 0..2: Interrupt Stack Table offset
            public byte TypeAttributes;     // gate type, dpl and p
            public ushort Offset2;          // offset bits 16..31
            public uint Offset3;            // offset bits 32..63
            public uint Zero;
        }

        private struct Table
        {
            public fixed ulong Raw[256 * 2];   // 256 descriptors of 16 bytes
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct Idtr
        {
            public ushort Limit;
            public ulong Base;
        }

        private static Table table;

        public static int SetGate(int slot, void* handler)
        {
            ulong address = (ulong)handler;
            fixed (ulong* raw = table.Raw)
            {
                var gate = (InterruptDescriptor*)raw + slot;
                gate->Offset1 = (ushort)(address & 0xFFFF);
                gate->Offset2 = (ushort)((address >> 16) & 0xFFFF);
                gate->Offset3 = (uint)((address >> 32) & 0xFFFFFFFF);
                gate->Selector = 0x0008;
                gate->TypeAttributes = 0x8E;
            }
            return 0;
        }

        public static int FillSlots()
        {
            for (int i = 0; i < 256; i++)
            {
                SetGate(i, (delegate*<InterruptFrame*, void>)&InterruptHandlers.Unhandled);
            }
            SetGate(0, (delegate*<InterruptFrame*, void>)&InterruptHandlers.DivideByZero);
            SetGate(14, (delegate*<InterruptFrame*, ulong, void>)&InterruptHandlers.PageFault);
            SetGate(32, (delegate*<InterruptFrame*, void>)&InterruptHandlers.Irq0);
            SetGate(33, (delegate*<InterruptFrame*, void>)&InterruptHandlers.Irq1);
            SetGate(39, (delegate*<InterruptFrame*, void>)&InterruptHandlers.Irq7);
            SetGate(46, (delegate*<InterruptFrame*, void>)&InterruptHandlers.Irq14);
            SetGate(47, (delegate*<InterruptFrame*, void>)&InterruptHandlers.Irq15);

            Idtr idtr;
            fixed (ulong* raw = table.Raw)
            {
                idtr.Base = (ulong)raw;
            }
            idtr.Limit = 256 * 16 - 1;
            Cpu.LoadIdt(&idtr);
            Cpu.Sti();
            return 0;
        }
    }
}
