; calculate RootDirSectors = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec - 1) / BPB_BytsPerSec), It is always 0
    ; RootEntCnt is always 0 too
    ; clear out ebx and eax, lets save the result here to ebx and we can always pop them
    ;xor ebx, ebx
    ;xor eax, eax
    ;mov eax, dword [wRootEntries]
    ;imul eax, 32
    ;mov ecx, dword [wBytesPerSector]
    ;sub ecx, 1
    ;add eax, ecx
    ;xor ecx, ecx
    ;mov ecx, dword [wBytesPerSector]
    ;mov ebx, eax
    ;div ebx, ecx
    ;; save the result
    ;push ebx

FirstSectorofCluster:
    ; FirstSectorofCluster = ((N – 2) * BPB_SecPerClus) + FirstDataSector;
    xor ecx, ecx
    xor eax, eax
    lea edi, [esi-2]
    movzx eax, byte [BPB_SecPerClus]
    imul edi, eax
    add edi, eax
    ; save result in edi
    push edi

DataSec:
    ; DataSec = TotSec – (BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors) ; RootDirSectors
    xor eax, eax
    xor edx, edx
    movzx eax, byte [BPB_NumFATs]
    movzx edx, byte [BPB_FATSz32]
    imul eax, edx
    movzx edx, word [BPB_RsvdSecCnt]
    add eax, edx
    movzx edx, word [BPB_TotSec32]
    sub eax, edx
    push eax

CountofCluster:
    ; CountofClusters = DataSec / BPB_SecPerClus
    pop eax
    mov eax, eax
    movzx eax, byte [BPB_SecPerClus]
    div eax
