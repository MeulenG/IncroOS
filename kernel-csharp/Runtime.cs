using System.Runtime.InteropServices;
using Incro.Runtime;
using IncroOS.Drivers;
using IncroOS.Memory;

namespace IncroOS
{
    // The two hooks IncroCompiler's runtime (Incro.CoreLib) calls, implemented by the kernel itself, as
    // kernel/csharp/runtime.c does for the C kernel
    internal static unsafe class Runtime
    {
        // Zeroed memory for the C# heap: the collector asks for chunks and manages the objects inside
        [UnmanagedCallersOnly(EntryPoint = "incro_alloc")]
        private static void* Allocate(nuint size)
        {
            var memory = (byte*)KMalloc.Allocate(size);
            if (memory == null)
            {
                Halt("IncroOS: the kernel heap has no room for a C# object");
            }
            for (nuint i = 0; i < size; i++)
            {
                memory[i] = 0;
            }
            return memory;
        }

        // Where .NET would throw and nothing catches it; the message is UTF-8
        [UnmanagedCallersOnly(EntryPoint = "incro_panic")]
        private static void Panic(byte* message)
        {
            Serial.WriteString("[PANIC] ");
            Serial.WriteCString(message);
            Serial.WriteString("\n");
            while (true)
            {
                Cpu.Cli();
                Cpu.Hlt();
            }
        }

        private static void Halt(string message)
        {
            Serial.WriteString("[PANIC] ");
            Serial.WriteString(message);
            Serial.WriteString("\n");
            while (true)
            {
                Cpu.Cli();
                Cpu.Hlt();
            }
        }
    }
}
