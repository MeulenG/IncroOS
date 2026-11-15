using IncroOS.Drivers;

namespace IncroOS.Memory
{
    // kernel/memory/pmm.c: physical pages in a bitmap, one bit per 4 KiB page, at 0x140000. Assumes 4 GiB of
    // memory until the bootloader passes the real size.
    internal static unsafe class Pmm
    {
        public const ulong PageSize = 4096;
        private const ulong PagesPerByte = 8;

        private static byte* pageBitmap;
        private static ulong totalPages;
        private static ulong usedPages;

        private static bool IsAllocated(ulong page) => (pageBitmap[page / PagesPerByte] & (1 << (int)(page % PagesPerByte))) != 0;

        private static void SetAllocated(ulong page) => pageBitmap[page / PagesPerByte] |= (byte)(1 << (int)(page % PagesPerByte));

        private static void SetFree(ulong page) => pageBitmap[page / PagesPerByte] &= (byte)~(1 << (int)(page % PagesPerByte));

        public static int Initialize()
        {
            ulong totalMemory = 4UL * 1024 * 1024 * 1024;
            totalPages = totalMemory / PageSize;
            ulong bitmapSize = (totalPages + PagesPerByte - 1) / PagesPerByte;
            pageBitmap = (byte*)0x140000;

            for (ulong i = 0; i < bitmapSize; i++)
            {
                pageBitmap[i] = 0;
            }

            // below 1 MiB
            ulong reservedLow = 0x100000 / PageSize;
            for (ulong i = 0; i < reservedLow; i++)
            {
                SetAllocated(i);
                usedPages++;
            }

            // the kernel
            ulong kernelPages = 0x80000 / PageSize;
            for (ulong i = reservedLow; i < reservedLow + kernelPages; i++)
            {
                SetAllocated(i);
                usedPages++;
            }

            // the bitmap itself
            ulong bitmapPages = (bitmapSize + PageSize - 1) / PageSize;
            ulong bitmapStartPage = 0x140000 / PageSize;
            for (ulong i = bitmapStartPage; i < bitmapStartPage + bitmapPages; i++)
            {
                SetAllocated(i);
                usedPages++;
            }

            Serial.WriteString("[PMM] Physical Memory Manager initialized\n");
            return 0;
        }

        // First fit from page 0; 0 when there is none
        public static ulong AllocatePage()
        {
            for (ulong page = 0; page < totalPages; page++)
            {
                if (!IsAllocated(page))
                {
                    SetAllocated(page);
                    usedPages++;
                    return page * PageSize;
                }
            }
            return 0;
        }

        public static int FreePage(ulong address)
        {
            ulong page = address / PageSize;
            if (page >= totalPages)
            {
                return -1;
            }
            if (IsAllocated(page))
            {
                SetFree(page);
                usedPages--;
            }
            return 0;
        }

        public static ulong TotalPages => totalPages;

        public static ulong FreePages => totalPages - usedPages;

        public static ulong UsedPages => usedPages;
    }
}
