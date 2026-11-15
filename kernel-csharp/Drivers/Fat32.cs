using System.Runtime.InteropServices;
using IncroOS.Lib;

namespace IncroOS.Drivers
{
    // The BIOS parameter block of a FAT32 volume, from byte 11 of its first sector
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    internal struct BiosParameterBlock
    {
        public ushort BytesPerSector;
        public byte SectorsPerCluster;
        public ushort ReservedSectors;
        public byte NumFats;
        public ushort RootEntryCount;
        public ushort TotalSectors16;
        public byte MediaType;
        public ushort FatSize16;
        public ushort SectorsPerTrack;
        public ushort NumHeads;
        public uint HiddenSectors;
        public uint TotalSectors32;

        // FAT32
        public uint FatSize32;
        public ushort ExtFlags;
        public ushort FsVersion;
        public uint RootCluster;
        public ushort FsInfo;
        public ushort BackupBootSector;
    }

    // kernel/drivers/fat32.c
    internal static unsafe class Fat32
    {
        public static int ClusterToLba(uint cluster, uint* lba, BiosParameterBlock* bpb)
        {
            uint firstDataSector = bpb->ReservedSectors + (uint)bpb->NumFats * bpb->FatSize32;
            *lba = bpb->HiddenSectors + firstDataSector + (cluster - 2) * bpb->SectorsPerCluster;
            return (int)*lba;
        }

        // Looks for an 8.3 name ("KRNLDR  BIN") in one sector of directory entries
        public static int FindFile(byte* directory, string name, uint* cluster, uint* size)
        {
            byte* filename = stackalloc byte[12];
            for (int i = 0; i < 512 / 32; i++)
            {
                byte* entry = directory + i * 32;
                if (entry[0] == 0x00)
                {
                    return -1;   // no more entries
                }
                if (entry[0] == 0xE5)
                {
                    continue;    // deleted
                }
                if (entry[11] == 0x0F)
                {
                    continue;    // long file name
                }

                for (int j = 0; j < 11; j++)
                {
                    filename[j] = entry[j];
                }
                filename[11] = 0;

                Print.Format("Checking file: %s against %s\n", Arg.C(filename), name);
                int result = Strings.Compare(filename, name);
                Print.Format("strcmp result: %d\n", result);
                if (result == 0)
                {
                    // high and low halves of the first cluster
                    *cluster = (uint)(*(ushort*)(entry + 20) << 16 | *(ushort*)(entry + 26));
                    *size = *(uint*)(entry + 28);
                    return 0;
                }
            }
            return -1;
        }

        public static int ReadFile(uint startCluster, BiosParameterBlock* bpb, byte* output)
        {
            uint current = startCluster;
            uint lba;
            uint bytesPerSector = bpb->BytesPerSector;
            uint sectorsPerCluster = bpb->SectorsPerCluster;
            byte* fat = stackalloc byte[512];

            // 0x0FFFFFF8 to 0x0FFFFFFF end the cluster chain
            while (current < 0x0FFFFFF8)
            {
                ClusterToLba(current, &lba, bpb);
                Ata.ReadSector(0, lba, (byte)sectorsPerCluster, output);

                // the next cluster in the chain, from the FAT
                uint fatSectorOffset = current * 4 / bytesPerSector;
                uint fatEntryLba = bpb->HiddenSectors + bpb->ReservedSectors + fatSectorOffset;
                Ata.ReadSector(0, fatEntryLba, 1, fat);
                current = *(uint*)(fat + current * 4 % bytesPerSector) & 0x0FFFFFFF;

                output += bytesPerSector * sectorsPerCluster;
            }
            return 0;
        }
    }
}
