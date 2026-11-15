using Incro.Runtime;
using IncroOS.Output;

namespace IncroOS.Drivers
{
    // kernel/drivers/ata.c: PIO reads from the primary ATA channel
    internal static unsafe class Ata
    {
        public static int Initialize()
        {
            // no device: the status register reads 0x00 or 0xFF
            byte status = Cpu.In8(0x1F7);
            if (status == 0x00 || status == 0xFF)
            {
                VesaTerminal.WriteString("No device present on primary channel.\n", VesaColor.Red);
                return -1;
            }
            // IDENTIFY
            Cpu.Out8(0x1F7, 0xEC);
            VesaTerminal.WriteString("IDENTIFY command sent to primary channel.\n", VesaColor.LightGrey);
            VesaTerminal.WriteString("Waiting for device to be ready...\n", VesaColor.LightGrey);
            if (Poll() != 0)
            {
                VesaTerminal.WriteString("Error during IDENTIFY command.\n", VesaColor.Red);
                return -1;
            }
            VesaTerminal.WriteString("Device detected on primary channel.\n", VesaColor.Green);
            // read and discard the identify data
            for (int i = 0; i < 256; i++)
            {
                Cpu.In16(0x1F0);
            }
            return 0;
        }

        public static int Poll()
        {
            VesaTerminal.WriteString("Polling drive for readiness...\n", VesaColor.LightGrey);
            for (uint i = 0; i < 100000; i++)
            {
                byte status = Cpu.In8(0x1F7);
                if ((status & 0x01) != 0 || (status & 0x20) != 0)
                {
                    VesaTerminal.WriteString("Drive error detected.\n", VesaColor.Red);
                    return -1;
                }
                if ((status & 0x80) == 0 && (status & 0x08) != 0)
                {
                    VesaTerminal.WriteString("Drive is ready.\n", VesaColor.Green);
                    return 0;
                }
            }
            VesaTerminal.WriteString("Drive poll timeout.\n", VesaColor.Red);
            return -1;
        }

        public static void ReadSector(byte drive, uint lba, byte sectorCount, byte* buffer)
        {
            Cpu.Out8(0x1F6, (byte)((drive == 0 ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F)));   // master or slave
            Cpu.Out8(0x1F1, 0x00);
            Cpu.Out8(0x1F2, sectorCount);
            Cpu.Out8(0x1F3, (byte)(lba & 0xFF));
            Cpu.Out8(0x1F4, (byte)((lba >> 8) & 0xFF));
            Cpu.Out8(0x1F5, (byte)((lba >> 16) & 0xFF));
            Cpu.Out8(0x1F7, 0x20);   // READ SECTORS
            for (byte i = 0; i < sectorCount; i++)
            {
                if (Poll() != 0)
                {
                    VesaTerminal.WriteString("Error during sector read.\n", VesaColor.Red);
                    return;
                }
                for (ushort j = 0; j < 256; j++)
                {
                    ushort data = Cpu.In16(0x1F0);
                    buffer[i * 512 + j * 2] = (byte)(data & 0xFF);
                    buffer[i * 512 + j * 2 + 1] = (byte)((data >> 8) & 0xFF);
                }
            }
        }
    }
}
