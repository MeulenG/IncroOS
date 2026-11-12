using System.Runtime.InteropServices;

namespace IncroOS.Kernel
{
    // Hello World in C#, compiled ahead of time by IncroCompiler and linked into the kernel.
    // There is no .NET runtime underneath: string literals are C strings and calls are plain
    // System V calls, so C functions are reached through [DllImport] and C reaches C# through
    // [UnmanagedCallersOnly(EntryPoint = ...)].
    public static class HelloWorld
    {
        private const uint White = 0xFFFFFF; // VESA_COLOR_WHITE

        // kernel/output/vesa_terminal.c, also mirrors the text to COM1
        [DllImport("kernel")]
        private static extern void vesa_terminal_writestring(string data, uint color);

        // Called from kMain once the boot services are up
        [UnmanagedCallersOnly(EntryPoint = "csharp_hello_world")]
        public static void Run()
        {
            vesa_terminal_writestring("Hello, World! (from C#)\n", White);
        }
    }
}
