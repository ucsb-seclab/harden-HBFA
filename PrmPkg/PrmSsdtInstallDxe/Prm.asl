/** @file
  The definition block in ACPI table for PRM Operation Region

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

DefinitionBlock (
    "Prm.aml",
    "SSDT",
    2,
    "OEMID ",
    "PRMOPREG",
    0x1000
    )
{
    Scope (\_SB)
    {
        //
        // PRM Test Device
        //
        Device (PRMT)
        {
            Name (_HID, "80860223")
            Name (_CID, "80860223")
            Name (_DDN, "PRM Test Device")
            Name (_STA, 0xF)

            // PrmSamplePrintModule handler GUIDs
            Name (BUF1, ToUUID("d5f2ad5f-a347-4d3e-87bc-c2ce63029cc8"))
            Name (BUF2, ToUUID("a9e7adc3-8cd0-429a-8915-10946ebde318"))
            Name (BUFN, ToUUID("b688c214-4081-4eeb-8d26-1eb5a3bcf11a"))

            //PRM operation region format
            OperationRegion (PRMR, PlatformRtMechanism, 0, 1)
            Field (PRMR, BufferAcc, NoLock, Preserve) //Make it ByteAcc for parameter validation
            {
                PRMF, 8
            }

            /*
            * Control method to invoke PRM OperationRegion handler
            * Arg0 contains a buffer representing a _DSM GUID
            */
            Method (RUNS, 1)
            {
                /* Local0 is the PRM data buffer */
                Local0 = buffer (26){}

                /* Create byte fields over the buffer */
                CreateByteField (Local0, 0x0, PSTA)
                CreateQWordField (Local0, 0x1, USTA)
                CreateByteField (Local0, 0x9, CMD)
                CreateField (Local0, 0x50, 0x80, GUID)

                /* Fill in the command and data fields of the data buffer */
                CMD = 0 // run command
                GUID = Arg0

                /* Invoke PRM OperationRegion Handler and store the result into Local0 */
                Local0 = (PRMF = Local0)

                /* Return status */
                Return (PSTA)
            }

            /*
            * Control method to lock a PRM transaction
            * Arg0 contains a buffer representing a _DSM GUID
            */
            Method (LCKH, 1)
            {
                /* Local0 is the PRM data buffer */
                Local0 = buffer (26){}

                /* Create byte fields over the buffer */
                CreateByteField (Local0, 0x0, STAT)
                CreateByteField (Local0, 0x9, CMD)
                CreateField (Local0, 0x50, 0x80, GUID)
                CMD = 1 // Lock command
                GUID = Arg0
                Local0 = (PRMF = Local0)

                /* Note STAT contains the return status */
                Return (STAT)
            }

            /*
            * Control method to unlock a PRM transaction
            * Arg0 contains a buffer representing a _DSM GUID
            */
            Method (ULCK, 1)
            {
                /* Local0 is the PRM data buffer */
                Local0 = buffer (26){}

                /* Create byte fields over the buffer */
                CreateByteField (Local0, 0x0, STAT)
                CreateByteField (Local0, 0x9, CMD)
                CreateField (Local0, 0x50, 0x80, GUID)
                CMD = 2 // Unlock command
                GUID = Arg0
                Local0 = (PRMF = Local0)

               /* Note STAT contains the return status */
                Return (STAT)
            }

            /*
            * Control method to test invoke PRM handler
            */
            Method (TST2, 0)
            {
              \_SB.PRMT.RUNS(BUFN)
            }

            /*
            * Control method to test lock a PRM transaction
            * If Local0 is not zero, just return the STAT
            */
            Method (LCKN, 0)
            {
              Return(\_SB.PRMT.LCKH(BUF1))
            }

            /*
            * Control method to test unlock a PRM transaction
            * If Local0 is not zero, just return the STAT
            */
            Method (ULKN, 0)
            {
              Return(\_SB.PRMT.ULCK(BUF2))
            }
        }
    }
}
