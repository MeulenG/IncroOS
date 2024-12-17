; *******************************************************
; GlobalDefines.inc
; - Define variables
; 
%ifndef GLOBALDEFINES_INC_
%define GLOBALDEFINES_INC_

%define NULL_DESC        0
%define CODE_DESC        0x8
%define DATA_SEGMENT     0x10


%define DATA_SEGMENT16   0x20
%define	CODE16_DESC      0x18


%define	CODE64_DESC      0x28
%define DATA_SEGMENT64   0x30

%define PAGESIZE 0x1000


%define MEMLOCATION_INITIALUPPERSTACK        0x7BFF
%define MEMLOCATION_INITIALLOWERSTACK		 0x100000
%define MEMLOCATION_BOOTCODE			     0x7C00


%define KERNEL_STACK_SIZE   0x10000
%define KERNEL_BASE_ADDRESS 0x100000


%define REAL_MODE


%endif