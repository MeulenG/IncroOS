using IncroOS.Drivers;

namespace IncroOS.Output
{
    // kernel/output/vga_terminal.c: the 80x25 text buffer at 0xB8000, mirrored to COM1
    internal static unsafe class VgaTerminal
    {
        private const ulong Width = 80;
        private const ulong Height = 25;
        private const byte LightGrey = 7;
        private const byte Black = 0;

        private static ulong row;
        private static ulong column;
        private static byte color;
        private static ushort* buffer;

        private static byte EntryColor(byte foreground, byte background) => (byte)(foreground | background << 4);

        private static ushort Entry(byte c, byte entryColor) => (ushort)(c | entryColor << 8);

        public static void Initialize()
        {
            row = 0;
            column = 0;
            color = EntryColor(LightGrey, Black);
            buffer = (ushort*)0xB8000;
            for (ulong y = 0; y < Height; y++)
            {
                for (ulong x = 0; x < Width; x++)
                {
                    buffer[y * Width + x] = Entry((byte)' ', color);
                }
            }
        }

        public static void SetColor(byte newColor) => color = newColor;

        private static void PutEntryAt(byte c, byte entryColor, ulong x, ulong y) => buffer[y * Width + x] = Entry(c, entryColor);

        public static void PutChar(byte c)
        {
            if (c == (byte)'\n')
            {
                column = 0;
                if (++row == Height)
                {
                    row = 0;
                }
                return;
            }
            PutEntryAt(c, color, column, row);
            if (++column == Width)
            {
                column = 0;
                if (++row == Height)
                {
                    row = 0;
                }
            }
        }

        public static void WriteString(string data)
        {
            for (int i = 0; i < data.Length; i++)
            {
                PutChar((byte)data[i]);
            }
            Serial.WriteString(data);
        }
    }
}
