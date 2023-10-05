;Attribute Bitmasks
Attrib_Read_Only			EQU 0x01		;File cannot be written to
Attrib_Hidden				EQU 0x02		;File normally hidden
Attrib_System				EQU 0x04		;File is component of OS
Attrib_Volume_ID			EQU 0x08		;Optional directory volume label
Attrib_Directory			EQU 0x10		;Subdirectory
Attrib_Archive				EQU 0x20		;File has been modified
Attrib_Device				EQU 0x40		;Never used
Attrib_Reserved				EQU 0x80		;Reserved