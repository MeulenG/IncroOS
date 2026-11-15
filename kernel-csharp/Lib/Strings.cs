namespace IncroOS.Lib
{
    // libs/libkernel/string.c: strcmp, with the same debug output, between a NUL-terminated string in
    // memory and a C# string (whose end counts as its NUL)
    internal static unsafe class Strings
    {
        private static int CharAt(string s, int i) => i < s.Length ? s[i] : 0;

        public static int Compare(byte* str1, string str2)
        {
            int i = 0;
            while (str1[i] != 0 && CharAt(str2, i) != 0)
            {
                int ascii1 = (sbyte)str1[i];
                int ascii2 = (sbyte)CharAt(str2, i);
                if (ascii1 != ascii2)
                {
                    Print.Format("Strings differ at index %d: %c vs %c\n", i, str1[i], (byte)CharAt(str2, i));
                    return ascii1 - ascii2;
                }
                i++;
            }
            int difference = str1[i] - (byte)CharAt(str2, i);
            Print.Format("Returning difference: %d\n", difference);
            return difference;
        }
    }
}
