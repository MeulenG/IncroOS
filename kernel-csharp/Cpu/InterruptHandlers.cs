using Incro.Runtime;
using IncroOS.Drivers;
using IncroOS.Output;

namespace IncroOS.X86
{
    // kernel/cpu/interrupt_handler.c. None of these allocate: a handler can interrupt the allocator.
    internal static unsafe class InterruptHandlers
    {
        private static void Log(string interruptName)
        {
            Serial.WriteString("[INTERRUPT] ");
            Serial.WriteString(interruptName);
            Serial.WriteString(" interrupt fired!\n");

            VgaTerminal.WriteString("[INTERRUPT] ");
            VgaTerminal.WriteString(interruptName);
            VgaTerminal.WriteString(" interrupt fired!\n");
        }

        // Every vector the kernel does not handle (idt.c's unhandled_interrupt)
        [Interrupt]
        public static void Unhandled(InterruptFrame* frame)
        {
            Serial.WriteString("unhandled interrupt fired!\n");
            VgaTerminal.WriteString("unhandled interrupt fired!\n");
            while (true)
            {
            }
        }

        [Interrupt]
        public static void DivideByZero(InterruptFrame* frame)
        {
            Log("Divide by Zero");
            while (true)
            {
            }
        }

        [Interrupt]
        public static void PageFault(InterruptFrame* frame, ulong errorCode)
        {
            Log("Page Fault");
            while (true)
            {
            }
        }

        // The PIT
        [Interrupt]
        public static void Irq0(InterruptFrame* frame)
        {
            Pic.SendEoi(0);
        }

        // The keyboard: key releases (bit 7) are not handled yet
        [Interrupt]
        public static void Irq1(InterruptFrame* frame)
        {
            byte scancode = Cpu.In8(0x60);
            if ((scancode & 0x80) != 0)
            {
                Pic.SendEoi(1);
                return;
            }
            Keyboard.HandleInterrupt(scancode);
            Pic.SendEoi(1);
        }

        // Spurious: no EOI, it can cause issues
        [Interrupt]
        public static void Irq7(InterruptFrame* frame)
        {
            Serial.WriteString("IRQ7: Spurious interrupt received\n");
        }

        [Interrupt]
        public static void Irq14(InterruptFrame* frame)
        {
            Serial.WriteString("IRQ14: Primary ATA channel interrupt received\n");
            Pic.SendEoi(14);
        }

        [Interrupt]
        public static void Irq15(InterruptFrame* frame)
        {
            Serial.WriteString("IRQ15: Spurious interrupt received\n");
        }
    }
}
