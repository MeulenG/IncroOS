using System.Runtime.InteropServices;
using Incro.Runtime;
using IncroOS.Drivers;
using IncroOS.Lib;
using IncroOS.Memory;
using IncroOS.Output;
using IncroOS.X86;

namespace IncroOS
{
    // kernel/kernel.c in C#: the whole kernel, compiled by IncroCompiler. Kernel_Entry_X86.asm calls kMain
    // with interrupts off, as for the C kernel; it never returns.
    internal static unsafe class Kernel
    {
        [UnmanagedCallersOnly(EntryPoint = "kMain")]
        private static void Main()
        {
            Serial.Initialize();
            Serial.WriteString("\n\n=== IncroOS Kernel Starting ===\n");
            VgaTerminal.Initialize();

            BootServices.Start();
            BootServices.Run("GDT Initialization\n", &Gdt.Initialize, &Gdt.Validate, true);
            BootServices.Run("IDT Initialization\n", &Idt.FillSlots, null, true);
            BootServices.Run("PIC Remapping\n", &Pic.Initialize, null, true);
            BootServices.Run("PMM Initialization\n", &Pmm.Initialize, null, true);
            BootServices.Run("VMM Initialization\n", &Vmm.Initialize, null, true);
            BootServices.Run("KMALLOC Initialization\n", &KMalloc.Initialize, &KMalloc.Validate, true);
            BootServices.Run("VESA Initialization\n", &VesaTerminal.InitializeService, null, true);
            BootServices.Run("ATA Initialization\n", &Ata.Initialize, null, true);
            VesaTerminal.WriteString("All boot services completed successfully!\n", VesaColor.LightGreen);

            ReadKernelFile();

            while (true)
            {
                Cpu.Hlt();
            }
        }

        // The disk test from kernel.c: the MBR, the FAT32 boot sector, the root directory, then KRNLDR.BIN
        private static void ReadKernelFile()
        {
            byte* buffer = stackalloc byte[512];
            Ata.ReadSector(0, 0, 1, buffer);
            Print.Format("Check buffer: %d\n", buffer[510]);
            Print.Format("Check buffer: %d\n", buffer[511]);

            uint partitionStart = *(uint*)(buffer + 454);
            Print.Format("Partition Start: %u\n", partitionStart);

            Ata.ReadSector(0, partitionStart, 1, buffer);
            var bpb = (BiosParameterBlock*)(buffer + 11);

            uint fatStartSector = bpb->ReservedSectors;
            uint dataStartSector = bpb->ReservedSectors + (uint)bpb->NumFats * bpb->FatSize32;
            Print.Format("FAT Start Sector: %u\n", fatStartSector);
            Print.Format("Data Start Sector: %u\n", dataStartSector);
            Print.Format("Root Cluster: %u\n", bpb->RootCluster);

            uint rootLba;
            Fat32.ClusterToLba(bpb->RootCluster, &rootLba, bpb);
            Print.Format("Root Directory LBA: %u\n", rootLba);

            byte* directory = stackalloc byte[4096];
            Ata.ReadSector(0, rootLba, 8, directory);

            uint foundCluster = 0;
            uint fileSize = 0;
            if (Fat32.FindFile(directory, "KRNLDR  BIN", &foundCluster, &fileSize) == 0)
            {
                Print.Format("Found KRNLDR.BIN at cluster %u\n", foundCluster);
            }
            else
            {
                Print.Format("KRNLDR.BIN not found\n");
                return;
            }

            var file = (byte*)KMalloc.Allocate(fileSize);
            if (file == null)
            {
                Print.Format("Failed to allocate memory for file buffer\n");
                return;
            }
            if (Fat32.ReadFile(foundCluster, bpb, file) != 0)
            {
                Print.Format("Failed to read file starting at cluster %u\n", foundCluster);
                return;
            }
            Print.Format("Successfully read file starting at cluster %u\n", foundCluster);
        }
    }
}
