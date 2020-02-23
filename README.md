# Lab3
third assignment for OS

In this lab/programming assignment you will implement/simulate the operation of an Operating System’s Virtual Memory Manager that maps the virtual address spaces of multiple processes onto physical frames using page table translation. The assignment will assume multiple processes, each with its own virtual address space of 64 virtual pages. As the sum of all virtual pages in all virtual address space may exceed the number of physical pages of the simulated system, paging needs to be implemented. The number of physical page frames varies and is specified by a program option, you have to support up to 128 frames, tests will only use 128 or less. Implementation is to be done in C/C++.
The input to your program will be a comprised of:

1. the number of processes (processes are numbered starting from 0)

2. a specification for each process’ address space is comprised of

    i. the number of virtual memory areas / segments (aka VMAs)
  
    ii. specification for each said VMA comprises of 4 numbers:
“starting_virtual_page ending_virtual_page write_protected[0/1] filemapped[0/1]”

## DATA STRUCTURES:

To approach this assignment, read in the specification and create processes, each with its list of vmas and a page_table that represents the translations from virtual pages to physical frames for that process.

A page table naturally must contain exactly 64 page table entries (PTE). Please use constants rather than hardcoding “64”. A PTE is comprised of the PRESENT/VALID, WRITE_PROTECT, MODIFIED, REFERENCED and PAGEDOUT bits and an index to the physical frame (in case the pte is present). This information can and should be implemented as a single 32-bit value or as a bit structures (easier). It cannot be a structure of multiple integer values that collectively are larger than 32-bits. (see http://www.cs.cf.ac.uk/Dave/C/node13.html (BitFields) or http://www.catonmat.net/blog/bit-hacks-header-file/ as an example, I highly encourage you to use this technique, let the compiler do the hard work for you).
Assuming that the maximum number of frames is 128, which equals 7 bits and the mentioned 5 bits above, you effectively have 32-12 = 20 bits for your own usage in the pagetable. You can use these bits at will (e.g. remembering whether a PTE is file mapped or not). What you can NOT do is run at the beginning of the program through the page table and mark each PTE with bits based on filemap or writeprotect. This is NOT how OSes do that due to hierarchical page stable structures (not implemented in this lab though). You can only set those bits on the first page fault to that virtual page.

You must define a global frame_table that each operating system maintains to describe the usage of each of its physical frames and where you maintain reverse mappings to the process and the vpage that maps a particular frame (In this assignment a frame can only be mapped by at most one PTE at a time).

## SIMULATION and IMPLEMENTATION:

During each instruction you simulate the behavior of the hardware and hence you must check that the page is present. A special case is the ‘c’ (context switch) instruction which simply changes the current process and current page table pointer.

If the page is not present, as indicated by the associated PTE’s valid/reference bit, the hardware would raise a page fault exception. Here you just simulate this by calling your (operating system’s) pagefault handler. In the pgfault handler you first determine that the vpage can be accessed, i.e. it is part of one of the VMAs, maybe you can find a faster way then searching each time the VMA list as long as it does not involve doing that before the instruction simulation. If not, a SEGV output line must be created and you move on to the next instruction. If it is part of a VMA then the page must be instantiated, i.e. a frame must be allocated, assigned to the PTE belonging to the vpage of this instruction and then populated with the proper content. The population depends whether this page was previously paged out (in which case the page must be brought back from the swap space (“IN”) or (“FIN” in case it is a memory mapped file). If the vpage was never swapped out and is not file mapped, then by definition it still has a zero filled content and you issue the “ZERO” output.

That leaves the allocation of frames. All frames initially are in a free “list” (you can choose a different FIFO data structure of course). Once you run out of free frames, you must implement paging. We explore the implementation of several page replacement algorithms. Page replacement implies the identification of a victim frame according to the algorithm’s policy. This should be implemented as a subclass of a general Pager class with one virtual function “frame_t* select_victim_frame();” that returns a victim frame. Once a victim frame has been determined, the victim frame must be unmapped from its user (address space:vpage), i.e. its entry in the owning process’s page_table must be removed (“UNMAP”), however you must remember the state of the bits. If the page was modified, then the page frame must be paged out to the swap device (“OUT”) or in case it was file mapped written back to the file (“FOUT”). Now the frame can be reused for the faulting instruction. First the PTE must be reset (note once the PAGEDOUT flag is set it will never be reset as it indicates there is content on the swap device) and then the PTE’s frame must be set.

At this point it is guaranteed, that the vpage is backed by a frame and the instruction can proceed in hardware (with the exception of the SEGV case above) and you have to set the REFERENCED and MODIFIED bits based on the operation. In case the instruction is a write operation and the PTE’s write protect bit is set (which it inherited from the VMA it belongs to) then a SEGPROT output line is to be generated. The page is considered referenced but not modified in this case.

Your code must actively maintain the PRESENT (aka valid), MODIFIED, REFERENCED, and PAGEDOUT bits and the frame index in the pagetable’s pte. The frame table is NOT updated by the simulated hardware as hardware has no access to the frame table. Only the pagetable entry (pte) is updated just as in real operating systems and hardware. The frame table can only be accessed as part of the “simulated page fault handler”.


The following page replacement algorithms are to be implemented (letter indicates option (see below)):

- FIFO

- Random

- Clock

- NRU

- Aging

- Working Set

The page replacement should be generic and the algorithms should be special instances of the page replacement class to avoid “switch/case statements” in the simulation of instructions. Use object oriented programming and inheritance.

When a virtual page is replaced, it must be first unmapped and then optionally paged out or written back to file. While you are not effectively implementing these operations you still need to track them and create entries in the output.

Since all replacement algorithms are based on frames, i.e. you are looping through the entire or parts of the frame table, and the reference and modified bits are only maintained in the page tables of processes, you need access to the PTEs. To be able to do that you should keep track of the reverse mapping from frame to PTE that is using it. Provide this reverse mapping inside each frame’s frame table entry.

Note (again): you MUST NOT set any bits in the PTE before instruction simulation start, i.e. the pte (i.e. all bits) should be initialized to “0” before the instruction simulation starts. This is also true for assigning FILE or WRITEPROTECT bits from the VMA. This is to ensure that in real OSs the full page table (hierarchical) is created on demand. Instead, on the first page fault on a particular pte, you have to search the vaddr in the VMA list. At that point you can store bits in the pte based on what you found in the VMA and what bits are not occupied by the mandatory bits.

In addition your program needs to compute and print the summary statistics related to the VMM if requested by an option. This means it needs to track the number of instructions, segv, segprot, unmap, map, pageins (IN, FIN), pageouts (OUT, FOUT), and zero operations for each process. In addition you should compute the overall execution time in cycles, where maps and unmaps each cost 400 cycles, page-in/outs each cost 3000 cycles, file in/outs cost 2500 cycles, zeroing a page costs 150 cycles, a segv costs 240 cycles, a segprot costs 300 cycles and each access (read or write) costs 1 cycles and a context switch costs 121 cycles and process exit costs 175 cycles.

Note, the cost calculation can overrun 2^32 and you must account for that, so use at least a 64-bit counters (unsigned long long). We will test your program with 1 Million instructions. Also the end calculations are tricky, so do them incrementally. Don’t add up 32-bit numbers and then assign to 64-bit. Add 32-bit numbers incrementally to the 64-bit counters, if you use 32-bit.
