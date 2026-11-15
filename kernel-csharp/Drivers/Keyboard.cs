using System;
using IncroOS.Output;

namespace IncroOS.Drivers
{
    // kernel/drivers/keyboard.c: scan code set 1 to ASCII, echoed to the VESA terminal
    internal static class Keyboard
    {
        private static ReadOnlySpan<byte> ScancodeToAscii => new byte[]
        {
            0, 0x1B, (byte)'1', (byte)'2', (byte)'3', (byte)'4', (byte)'5', (byte)'6',
            (byte)'7', (byte)'8', (byte)'9', (byte)'0', (byte)'-', (byte)'=', (byte)'\b', (byte)'\t',
            (byte)'q', (byte)'w', (byte)'e', (byte)'r', (byte)'t', (byte)'y', (byte)'u', (byte)'i',
            (byte)'o', (byte)'p', (byte)'[', (byte)']', (byte)'\n', 0, (byte)'a', (byte)'s',
            (byte)'d', (byte)'f', (byte)'g', (byte)'h', (byte)'j', (byte)'k', (byte)'l', (byte)';',
            (byte)'\'', (byte)'`', 0, (byte)'\\', (byte)'z', (byte)'x', (byte)'c', (byte)'v',
            (byte)'b', (byte)'n', (byte)'m', (byte)',', (byte)'.', (byte)'/', 0, (byte)'*',
            0, (byte)' ', 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
        };

        // IRQ1 passes the scan code of a key press here
        public static void HandleInterrupt(byte scancode)
        {
            VesaTerminal.PutChar(ScancodeToAscii[scancode], VesaColor.White);
        }
    }
}
