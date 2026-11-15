using System.Runtime.InteropServices;

namespace IncroOS.Drivers
{
    // kernel/drivers/vesa.c: the linear framebuffer the bootloader set up, described by the VBE mode info
    // it left at 0x7000. 32 bits per pixel.
    internal static unsafe class Vesa
    {
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct ModeInfo
        {
            public ushort Attributes;
            public byte WindowA;
            public byte WindowB;
            public ushort Granularity;
            public ushort WindowSize;
            public ushort SegmentA;
            public ushort SegmentB;
            public uint WinFuncPtr;
            public ushort Pitch;          // bytes per line
            public ushort Width;
            public ushort Height;
            public byte WChar;
            public byte YChar;
            public byte Planes;
            public byte Bpp;
            public byte Banks;
            public byte MemoryModel;
            public byte BankSize;
            public byte ImagePages;
            public byte Reserved0;
            public byte RedMask;
            public byte RedPosition;
            public byte GreenMask;
            public byte GreenPosition;
            public byte BlueMask;
            public byte BluePosition;
            public byte ReservedMask;
            public byte ReservedPosition;
            public byte DirectColorAttributes;
            public uint Framebuffer;     // physical address of the linear framebuffer
        }

        private static uint framebuffer;
        private static ushort width;
        private static ushort height;
        private static byte bpp;

        public static int Initialize()
        {
            var info = (ModeInfo*)0x7000;
            framebuffer = info->Framebuffer;
            width = info->Width;
            height = info->Height;
            bpp = info->Bpp;
            return 0;
        }

        public static int PutPixel(uint x, uint y, uint color)
        {
            if (x >= width || y >= height)
            {
                return -1;
            }
            uint offset = (y * width + x) * (uint)(bpp / 8);
            *(uint*)(ulong)(framebuffer + offset) = color;
            return 0;
        }

        public static int DrawChar(int x, int y, byte c, uint color)
        {
            for (int row = 0; row < 8; row++)
            {
                // the font has U+0000 to U+007F
                if (c >= 128)
                {
                    return -1;
                }
                byte rowData = Font.Glyphs[c * 8 + row];
                for (int col = 0; col < 8; col++)
                {
                    if ((rowData & (1 << col)) != 0)
                    {
                        PutPixel((uint)(x + col), (uint)(y + row), color);
                    }
                }
            }
            return 0;
        }

        public static int DrawString(int x, int y, string text, uint color)
        {
            for (int i = 0; i < text.Length; i++)
            {
                DrawChar(x, y, (byte)text[i], color);
                x += 8;
            }
            return 0;
        }
    }
}
