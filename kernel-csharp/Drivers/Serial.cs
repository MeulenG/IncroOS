using Incro.Runtime;

namespace IncroOS.Drivers
{
    // kernel/drivers/serial.c: COM1 at 115200 baud, LF sent as CRLF
    internal static unsafe class Serial
    {
        private const ushort Com1 = 0x3F8;

        public static int Initialize()
        {
            Cpu.Out8(Com1 + 1, 0x00);   // disable all interrupts
            Cpu.Out8(Com1 + 3, 0x80);   // enable DLAB (set the baud rate divisor)
            Cpu.Out8(Com1 + 0, 0x01);   // divisor 1: 115200 baud
            Cpu.Out8(Com1 + 1, 0x00);   //   (high byte)
            Cpu.Out8(Com1 + 3, 0x03);   // 8 bits, no parity, one stop bit
            Cpu.Out8(Com1 + 2, 0xC7);   // enable the FIFO, clear it, 14-byte threshold
            Cpu.Out8(Com1 + 4, 0x0B);   // IRQs enabled, RTS/DSR set
            Cpu.Out8(Com1 + 4, 0x1E);   // loopback mode, to test the chip
            Cpu.Out8(Com1 + 0, 0xAE);   // send 0xAE, it has to come back

            if (Cpu.In8(Com1 + 0) != 0xAE)
            {
                return 1;
            }
            // not faulty: normal operation
            Cpu.Out8(Com1 + 4, 0x0F);
            return 0;
        }

        public static int PutChar(byte c)
        {
            while ((Cpu.In8(Com1 + 5) & 0x20) == 0)
            {
                // wait for the transmit holding register to be empty
            }
            Cpu.Out8(Com1 + 0, c);
            return 0;
        }

        public static int Write(byte* data, ulong size)
        {
            for (ulong i = 0; i < size; i++)
            {
                // CRLF for serial terminals
                if (data[i] == (byte)'\n')
                {
                    PutChar((byte)'\r');
                }
                PutChar(data[i]);
            }
            return 0;
        }

        public static int WriteString(string data)
        {
            for (int i = 0; i < data.Length; i++)
            {
                if (data[i] == '\n')
                {
                    PutChar((byte)'\r');
                }
                PutChar((byte)data[i]);
            }
            return 0;
        }

        // A NUL-terminated string from C (incro_panic's message)
        public static int WriteCString(byte* data)
        {
            ulong length = 0;
            while (data[length] != 0)
            {
                length++;
            }
            return Write(data, length);
        }
    }
}
