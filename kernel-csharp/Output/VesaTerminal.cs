using IncroOS.Drivers;

namespace IncroOS.Output
{
    internal static class VesaColor
    {
        public const uint Black = 0x000000;
        public const uint Blue = 0x0000FF;
        public const uint Green = 0x00FF00;
        public const uint Cyan = 0x00FFFF;
        public const uint Red = 0xFF0000;
        public const uint Magenta = 0xFF00FF;
        public const uint Brown = 0xA52A2A;
        public const uint LightGrey = 0xD3D3D3;
        public const uint DarkGrey = 0xA9A9A9;
        public const uint LightBlue = 0xADD8E6;
        public const uint LightGreen = 0x90EE90;
        public const uint LightCyan = 0xE0FFFF;
        public const uint LightRed = 0xFFA07A;
        public const uint LightMagenta = 0xEE82EE;
        public const uint Yellow = 0xFFFF00;
        public const uint White = 0xFFFFFF;
    }

    // kernel/output/vesa_terminal.c: text drawn into the framebuffer 8 pixels at a time, mirrored to COM1
    internal static class VesaTerminal
    {
        private static int x;
        private static int y;

        public static void Initialize() => Vesa.Initialize();

        // As a boot service (kernel.c casts vesa_terminal_initialize to an int function)
        public static int InitializeService()
        {
            Initialize();
            return 0;
        }

        public static void PutChar(byte c, uint color)
        {
            if (c == (byte)'\n')
            {
                x = 0;
                y += 8;
                return;
            }
            Vesa.DrawChar(x, y, c, color);
            x += 8;
        }

        public static void WriteString(string data, uint color)
        {
            for (int i = 0; i < data.Length; i++)
            {
                PutChar((byte)data[i], color);
            }
            Serial.WriteString(data);
        }
    }
}
