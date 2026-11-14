using System.Runtime.InteropServices;

namespace IncroOS.Kernel
{
    // Where a log line goes. Each sink is a class behind this interface, so a log calls them through
    // interface dispatch without knowing which ones it has.
    public interface ILogSink
    {
        void Write(string line);
    }

    // The VESA terminal, which also mirrors its text to COM1
    public sealed class ScreenSink : ILogSink
    {
        private readonly uint color;

        public ScreenSink(uint color)
        {
            this.color = color;
        }

        // kernel/output/vesa_terminal.c; the string arrives as NUL-terminated UTF-8
        [DllImport("kernel")]
        private static extern void vesa_terminal_writestring(string data, uint color);

        public void Write(string line) => vesa_terminal_writestring(line, color);
    }

    // COM1 only, through the C# serial driver
    public sealed class SerialSink : ILogSink
    {
        public void Write(string line) => Serial.Write(line);
    }

    // A log with a source name and a set of sinks. The sinks array, every line and the objects
    // themselves live on the kernel heap (incro_alloc in runtime.c).
    public sealed class KernelLog
    {
        private readonly string source;
        private readonly ILogSink[] sinks;

        public KernelLog(string source, params ILogSink[] sinks)
        {
            this.source = source;
            this.sinks = sinks;
        }

        public void Write(string message)
        {
            string line = "[" + source + "] " + message + "\n";
            foreach (var sink in sinks)
            {
                sink.Write(line);
            }
        }
    }
}
