; *******************************************************
; DataSection.inc
; - Data Section
; 
%ifndef DATASECTION_INC_
%define DATASECTION_INC_

dReserved0					dd		0 	;FirstDataSector

dReserved1					dd		0 	;ReadCluster

dReserved2					dd 		0 	;ReadCluster

syskrnldr					db 		"KRNLDR  SYS"

bPhysicalDriveNum			DB		0

msgA20                      DB  0x0A, 0x0D, "Enabling A20 Gate", 0x00

msggdt                      DB  0x0A, 0x0D, "Installing GDT", 0x00

msgidt                      DB  0x0A, 0x0D, "Installing IDT", 0x00

msgKernelLoaded             DB  0x0A, 0x0D, "Loading Kernel", 0x00

msgVideoMode                DB  0x0A, 0x0D, "Video Mode Set", 0x00

PrepareMsg64:				DB  0x0A, 0x0D,"Preparing To Load 64-Bit Operating System...",  0x00

PrepareMsgKernel64:			DB  0x0A, 0x0D,"Handing Control To The Kernel, Stand By...",  0x00

msgStart                    DB  0x0D,0x0A, "Boot Loader starting (0x7C00)....", 0x00

msgStageTwo                 DB  0x0D, 0x0A, "Jumping to stage2 (0x7E00)", 0x00


ReadPacket:                 times 16 db 0

LoadingMsg                  DB 0x0D, 0x0A, "Stage 2 Sucessfully Loaded", 0x00

msgpmode:                   DB  0x0A, 0x0A, 0x0A, "               <[Puhaa-OS Protected Mode]>         "
                            DB  0x0A, 0x0A   		"            Welcome To Protected Mode", 0

msglongmode:                DB  0x0A, 0x0A, 0x0A, "               <[Puhaa-OS Long Mode]>"
                            DB  0x0A, 0x0A   		"            Welcome To 64-Bit Mode Long Mode", 0

ErrorMsg                    DB  "Hahahaha, Fuck you, but at Stage2"

ErrorFixCS:  DB "FixCS error", 0x00

ErrorReadSector: DB "ReadSector error", 0x00
%endif