#  Virtual Memory Simulator

### Overview
A Virtual Machine (VM) Simulator simulates the operation of a virtual memory system. 
The simulator will accept a command-line argument for the page replacement algorithm and commands to read/write from/to a virtual address space. It will manage pages between disk and main memory and maintain the page table accordingly.

### Parameters of the Virtual Memory System

#### A Define a VM, Main Memory, and Disk Memory
- **Address Storage:** Each address stores a single integer.
- **Virtual Memory:** 128 addresses (0 - 127).
- **Main Memory:** 32 addresses (0 - 31).
- **Pages:** Each page contains 8 addresses.
  - Virtual memory: 16 pages.
  - Main memory: 4 pages.
- **Page Information:** Each page contains a Valid bit, Dirty bit, and Page Number.
- **Page Numbering:** Sequentially starting at the lowest address. 
  - Page 0: addresses 0-7.
  - Page 1: addresses 8-15.
  - Disk page number matches virtual page number.

#### Initialize Memory Locations and Pages
- All memory locations initialized to -1.
- All virtual pages initially on disk (Valid bits = 0).

### User Interface of VM Simulator
The simulator accepts and processes commands in a loop, starting with a `> ` prompt to indicate readiness for a new command. Commands execute and print necessary data, followed by a new `> ` prompt.

### Commands

#### 1. `read <virtual_addr>`
Prints the contents of a memory address.
- **Arguments:** virtual address to be read.
- **Page Fault:** Print "A Page Fault Has Occurred\n" if a page fault occurs.

#### 2. `write <virtual_addr> <num>`
Writes data to a memory location.
- **Arguments:** virtual address and integer to be written.
- **Page Fault:** Print "A Page Fault Has Occurred\n" if a page fault occurs.

#### 3. `showmain <ppn>`
Prints the contents of a physical page in main memory.
- **Arguments:** physical page number.
- **Output:** Eight addresses and their contents.

#### 4. `showptable`
Prints the contents of the page table (16 entries).
- **Fields:** Virtual page number, Valid bit, Dirty bit, Page Number.
- **Format:** Each entry on a separate line, separated by colons (`:`).

#### 5. `quit`
Quits the program. No output needed.

### Page Replacement Algorithm (Handling Page Faults)
When a page fault occurs, the disk page must be copied to main memory.
- **Available Pages:** Copy to available main memory page with the lowest number.
- **All Pages in Use:** Choose a victim page for eviction using FIFO or LRU algorithms.
- **Dirty Bit:** If the dirty bit is 1, copy the victim page back to disk before eviction.

### Page Replacement Algorithms
- **FIFO:** First-In-First-Out replacement.
- **LRU:** Least Recently Used replacement. `show*` commands do not count as use.
- **Default:** FIFO if no argument is provided.

### Execution Examples
- **Default:** `$ ./a.out`
- **FIFO:** `$ ./a.out FIFO`
- **LRU:** `$ ./a.out LRU`


### Example Usage
```plaintext
$ ./a.out FIFO
> showptable
0:0:0:0
1:0:0:1
2:0:0:2
...
> read 9
A Page Fault Has Occurred
-1
> write 9 201
> read 9
201
> showmain 0
0: -1
1: 201
...
> showptable
0:0:0:0
1:1:1:0
...
> quit
$
```

### More Info
For additional details, please refer to the specification document provided as specification.pdf.