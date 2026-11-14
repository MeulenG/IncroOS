// The two hooks C# compiled by IncroCompiler calls into the kernel. Everything else (type tests,
// strings, interface calls) is generated into the C# object itself.

#include <stddef.h>
#include <stdint.h>

#include "../drivers/serial.h"
#include "../memory/kmalloc.h"

void incro_panic(const char *message);

// Every C# object, array and string comes from here. IncroCompiler relies on the memory being zeroed
// (fields start out as 0 and null), and kmalloc does not zero it. There is no garbage collector yet,
// so nothing is freed.
void *incro_alloc(size_t size)
{
    uint8_t *memory = kmalloc(size);
    if (memory == NULL) {
        incro_panic("IncroOS: the kernel heap has no room for a C# object");
    }
    for (size_t i = 0; i < size; i++) {
        memory[i] = 0;
    }
    return memory;
}

// Called where .NET would throw: a failed cast, a call through a missing method, or a C# throw
// (there is no exception handling yet). The message is UTF-8, e.g.
// "Unhandled exception. System.InvalidOperationException: ...". Does not return.
void incro_panic(const char *message)
{
    serial_writestring("[PANIC] ");
    serial_writestring(message);
    serial_writestring("\n");
    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}
