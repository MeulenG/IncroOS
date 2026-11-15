using IncroOS.Drivers;
using IncroOS.Output;

namespace IncroOS.Lib
{
    internal enum ArgKind : byte
    {
        None,
        Integer,
        Text,
        CText,
    }

    // One kprintf argument. C passes varargs in 64-bit slots and %d/%u/%x read the low 32 bits unless
    // there is an l, so an integer is kept as those 64 bits.
    internal unsafe struct Arg
    {
        public ArgKind Kind;
        public ulong Bits;
        public string Text;

        public static implicit operator Arg(int value) => new Arg { Kind = ArgKind.Integer, Bits = (ulong)(long)value };
        public static implicit operator Arg(uint value) => new Arg { Kind = ArgKind.Integer, Bits = value };
        public static implicit operator Arg(long value) => new Arg { Kind = ArgKind.Integer, Bits = (ulong)value };
        public static implicit operator Arg(ulong value) => new Arg { Kind = ArgKind.Integer, Bits = value };
        public static implicit operator Arg(byte value) => new Arg { Kind = ArgKind.Integer, Bits = value };
        public static implicit operator Arg(char value) => new Arg { Kind = ArgKind.Integer, Bits = value };
        public static implicit operator Arg(string value) => new Arg { Kind = ArgKind.Text, Text = value };

        // A NUL-terminated string in memory, for %s
        public static Arg C(byte* text) => new Arg { Kind = ArgKind.CText, Bits = (ulong)text };
    }

    // libs/libkernel/print.c: kprintf, to COM1 and the VESA terminal. %d %u %x (with l for 64 bits), %s,
    // %c, %p and %%; up to three arguments.
    internal static unsafe class Print
    {
        private static void EmitChar(byte c)
        {
            if (c == (byte)'\n')
            {
                Serial.PutChar((byte)'\r');
            }
            Serial.PutChar(c);
            VesaTerminal.PutChar(c, VesaColor.White);
        }

        private static void EmitString(string s)
        {
            for (int i = 0; i < s.Length; i++)
            {
                EmitChar((byte)s[i]);
            }
        }

        private static void EmitCString(byte* s)
        {
            while (*s != 0)
            {
                EmitChar(*s++);
            }
        }

        private static void EmitDecimal(long value)
        {
            if (value == 0)
            {
                EmitChar((byte)'0');
                return;
            }
            if (value < 0)
            {
                EmitChar((byte)'-');
                value = -value;
            }
            byte* digits = stackalloc byte[20];
            int i = 0;
            while (value > 0)
            {
                digits[i++] = (byte)('0' + value % 10);
                value /= 10;
            }
            while (i > 0)
            {
                EmitChar(digits[--i]);
            }
        }

        private static void EmitUnsigned(ulong value)
        {
            if (value == 0)
            {
                EmitChar((byte)'0');
                return;
            }
            byte* digits = stackalloc byte[20];
            int i = 0;
            while (value > 0)
            {
                digits[i++] = (byte)('0' + value % 10);
                value /= 10;
            }
            while (i > 0)
            {
                EmitChar(digits[--i]);
            }
        }

        private static void EmitHex(ulong value, int minWidth)
        {
            byte* digits = stackalloc byte[16];
            int i = 0;
            do
            {
                digits[i++] = (byte)"0123456789abcdef"[(int)(value & 0xF)];
                value >>= 4;
            }
            while (value > 0);

            while (i < minWidth)
            {
                EmitChar((byte)'0');
                minWidth--;
            }
            while (i > 0)
            {
                EmitChar(digits[--i]);
            }
        }

        private static Arg Next(ref int used, Arg a, Arg b, Arg c)
        {
            int n = used++;
            return n == 0 ? a : n == 1 ? b : c;
        }

        public static void Format(string format, Arg a = default, Arg b = default, Arg c = default)
        {
            int used = 0;
            int i = 0;
            while (i < format.Length)
            {
                char ch = format[i];
                if (ch != '%')
                {
                    EmitChar((byte)ch);
                    i++;
                    continue;
                }
                i++;
                bool isLong = false;
                if (i < format.Length && format[i] == 'l')
                {
                    isLong = true;
                    i++;
                }
                char specifier = i < format.Length ? format[i] : '\0';
                switch (specifier)
                {
                    case 'd':
                    {
                        ulong bits = Next(ref used, a, b, c).Bits;
                        EmitDecimal(isLong ? (long)bits : (int)(uint)bits);
                        break;
                    }
                    case 'u':
                    {
                        ulong bits = Next(ref used, a, b, c).Bits;
                        EmitUnsigned(isLong ? bits : (uint)bits);
                        break;
                    }
                    case 'x':
                    {
                        ulong bits = Next(ref used, a, b, c).Bits;
                        EmitHex(isLong ? bits : (uint)bits, 0);
                        break;
                    }
                    case 's':
                    {
                        Arg arg = Next(ref used, a, b, c);
                        if (arg.Kind == ArgKind.CText)
                        {
                            EmitCString((byte*)arg.Bits);
                        }
                        else
                        {
                            EmitString(arg.Text);
                        }
                        break;
                    }
                    case 'c':
                        EmitChar((byte)Next(ref used, a, b, c).Bits);
                        break;
                    case 'p':
                        EmitString("0x");
                        EmitHex(Next(ref used, a, b, c).Bits, 16);
                        break;
                    case '%':
                        EmitChar((byte)'%');
                        break;
                    default:
                        // an unknown specifier is emitted as it is
                        EmitChar((byte)'%');
                        EmitChar((byte)specifier);
                        break;
                }
                i++;
            }
        }
    }
}
