namespace IncroOS.Output
{
    // kernel/output/boot_service.c: runs each boot service, validates it, and reports on the VESA terminal.
    // kernel.c passes an array of { name, init, validate, critical }; here each service is one Run call,
    // with function pointers, so nothing is allocated before the heap exists.
    internal static unsafe class BootServices
    {
        public static void Start()
        {
            VesaTerminal.Initialize();
            VesaTerminal.WriteString("Boot service started...\n", VesaColor.LightGrey);
        }

        public static void Run(string name, delegate*<int> initialize, delegate*<int> validate, bool critical)
        {
            int result = initialize();
            if (result != 0)
            {
                VesaTerminal.WriteString("Boot service failed: ", VesaColor.LightRed);
                VesaTerminal.WriteString(name, VesaColor.LightRed);
                VesaTerminal.WriteString("\n", VesaColor.LightRed);
                if (critical)
                {
                    VesaTerminal.WriteString("Critical boot service failed. Halting system.\n", VesaColor.LightRed);
                    while (true)
                    {
                    }
                }
            }
            // only a service that initialized is validated, and only if it has a validator
            if (result == 0 && validate != null)
            {
                if (validate() != 0)
                {
                    VesaTerminal.WriteString("Boot service validation failed: ", VesaColor.LightRed);
                    VesaTerminal.WriteString(name, VesaColor.LightRed);
                    VesaTerminal.WriteString("\n", VesaColor.LightRed);
                    if (critical)
                    {
                        VesaTerminal.WriteString("Critical boot service validation failed. Halting system.\n", VesaColor.LightRed);
                        while (true)
                        {
                        }
                    }
                }
            }
            VesaTerminal.WriteString("[   ", VesaColor.LightGrey);
            if (result == 0)
            {
                VesaTerminal.WriteString("OK", VesaColor.LightGreen);
                VesaTerminal.WriteString("   ]", VesaColor.LightGrey);
                VesaTerminal.WriteString(" Boot service completed: ", VesaColor.LightGrey);
            }
            else
            {
                VesaTerminal.WriteString("FAIL", VesaColor.LightRed);
                VesaTerminal.WriteString("   ]", VesaColor.LightGrey);
                VesaTerminal.WriteString(" Boot service failed: ", VesaColor.LightRed);
            }
            VesaTerminal.WriteString(name, VesaColor.LightGrey);
            VesaTerminal.WriteString("\n", VesaColor.LightGrey);
        }
    }
}
