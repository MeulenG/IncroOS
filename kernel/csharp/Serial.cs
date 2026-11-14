using Incro.Runtime;

namespace IncroOS.Kernel
{
    // Transmit side of the COM1 driver in C#: port I/O through Incro.Runtime.Cpu, which IncroCompiler
    // turns into in/out instructions. serial_init in kernel/drivers/serial.c has already set the UART up.
    public static class Serial
    {
        private const ushort Com1 = 0x3F8;
        private const ushort LineStatus = Com1 + 5;
        private const byte TransmitEmpty = 0x20;

        // Converts LF to CRLF for serial terminals, like the C driver
        public static void Write(string text)
        {
            for (int i = 0; i < text.Length; i++)
            {
                if (text[i] == '\n')
                {
                    Put('\r');
                }
                Put(text[i]);
            }
        }

        private static void Put(char c)
        {
            while ((Cpu.In8(LineStatus) & TransmitEmpty) == 0)
            {
                // wait until the transmit holding register is empty
            }
            Cpu.Out8(Com1, (byte)c);
        }
    }
}
