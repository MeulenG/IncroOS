using Incro.Runtime;

namespace IncroOS.X86
{
    // kernel/cpu/pic.c: the two 8259 PICs, remapped to vectors 0x20 and 0x28
    internal static class Pic
    {
        private const ushort Pic1Command = 0x20;
        private const ushort Pic1Data = 0x21;
        private const ushort Pic2Command = 0xA0;
        private const ushort Pic2Data = 0xA1;
        private const byte EndOfInterrupt = 0x20;

        private const byte Icw1Icw4 = 0x01;     // ICW4 will be present
        private const byte Icw1Init = 0x10;     // initialization, required
        private const byte Icw48086 = 0x01;     // 8086/88 mode
        private const int CascadeIrq = 2;

        public static int SendEoi(byte irq)
        {
            // an IRQ from the slave PIC needs an EOI to both
            if (irq >= 8)
            {
                Cpu.Out8(Pic2Command, EndOfInterrupt);
            }
            Cpu.Out8(Pic1Command, EndOfInterrupt);
            return 0;
        }

        // Some older machines need time between PIC commands; a write to port 0x80 takes about that long
        private static void IoWait() => Cpu.Out8(0x80, 0);

        public static int Remap(int offset1, int offset2)
        {
            Cpu.Out8(Pic1Command, Icw1Init | Icw1Icw4);   // start the initialization sequence (cascade mode)
            IoWait();
            Cpu.Out8(Pic2Command, Icw1Init | Icw1Icw4);
            IoWait();
            Cpu.Out8(Pic1Data, (byte)offset1);            // ICW2: master vector offset
            IoWait();
            Cpu.Out8(Pic2Data, (byte)offset2);            // ICW2: slave vector offset
            IoWait();
            Cpu.Out8(Pic1Data, 1 << CascadeIrq);          // ICW3: the slave is on IRQ2
            IoWait();
            Cpu.Out8(Pic2Data, 2);                        // ICW3: the slave's cascade identity
            IoWait();

            Cpu.Out8(Pic1Data, Icw48086);                 // ICW4: 8086 mode
            IoWait();
            Cpu.Out8(Pic2Data, Icw48086);
            IoWait();

            // unmask both
            Cpu.Out8(Pic1Data, 0);
            Cpu.Out8(Pic2Data, 0);
            return 0;
        }

        public static int Initialize()
        {
            Remap(0x20, 0x28);
            return 0;
        }
    }
}
