using System.Runtime.InteropServices;
using Incro.Runtime;

namespace IncroOS.Kernel
{
    // IRQ0, the PIT, handled in C#. With -DINCRO_CSHARP=ON, kernel/cpu/idt.c points vector 32 at this
    // handler's entry stub, which IncroCompiler generates: it saves the registers, calls Tick and
    // returns with iretq.
    public static unsafe class Timer
    {
        private const ushort PicCommand = 0x20;
        private const byte EndOfInterrupt = 0x20;
        private const ulong TicksPerSecond = 18;   // the PIT's default rate is about 18.2 Hz

        private static ulong ticks;

        [Interrupt, UnmanagedCallersOnly(EntryPoint = "csharp_timer_isr")]
        private static void Tick(InterruptFrame* frame)
        {
            ticks++;
            if (ticks == TicksPerSecond)
            {
                Serial.Write("C# timer interrupt: one second of ticks\n");
            }
            Cpu.Out8(PicCommand, EndOfInterrupt);
        }
    }
}
