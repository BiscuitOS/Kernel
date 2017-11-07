Advanced Programmable Interrupt Controller(APIC)
-----------------------------------------------------

The Advanced Programmable Interrupt Controller(APIC), referred to in the
following section as the local APIC, was introduced into the IA-32
processor families. The local APIC performs two primary function for
processor:

  * It receives interrupts from the processor's interrupt pins, from
    internal sources and from an external I/O APIC (or other external
    interrupt controller). It sends these to the processor core for
    handing.

  * In multiple processor (MP) systems, it sends and receives interprocessor
    interrupt (IPI) messages to and from other logical processors on
    the system bus. IPI messages can be used to distribute interrupts 
    among the processors in the system or to execute system wide 
    function (such as, booting up processor or distributing work among a
    group of processors).

The external I/O APIC is part of Intel's system chip set. Its primary
function is to receive external interrupt events from the system and 
its associated I/O devices and relay them to the local APIC as interrupt
messages. In MP systems, the I/O APIC also provides a mechanism for
distributing external interrupts to the local APICs of selected 
processors or group of processors on the system bus.

This chapter provides a description of the local APIC and its programming
interfcace. It also provides an overview of the interface between the 
local APIC and I/O APIC. Contact Intel for detailed information about
the I/O APIC.

When a local APIC has send an interrupt to its processor core for handing,
the processor uses the interrupt and exception handing mechanism 
described in "Interrupt and Execption Handing."
