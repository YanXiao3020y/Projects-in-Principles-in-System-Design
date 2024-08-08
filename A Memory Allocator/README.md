## A Memory Allocator

### Objective

Implement a simple memory allocator for a
fixed-size heap using C. The goal is to manage memory allocation and
deallocation with an implicit free list, supporting various operations
like allocating memory, freeing memory, and inspecting the heap\'s
state. 

### Heap Structure

-   **Size**: 127 bytes

-   **Address Range**: 0 to 126

-   **Block Layout**: Each memory block consists of:

    -   **Header**: 1 byte indicating the block size and allocation
        status.

    -   **Footer**: 1 byte mirroring the header.

-   **Initial State**: The heap starts with one large free block
    covering the entire range (0 to 126).

### Supported Operations

1.  **malloc \<size\>**: Allocates memory and returns a pointer to the
    start of the payload. If the requested size is less than an existing
    block, the block may be split.

2.  **free \<index\>**: Frees a previously allocated block identified by
    the pointer to its payload. Coalescing with adjacent free blocks is
    required.

3.  **blocklist**: Displays information about all blocks, including
    their payload size, pointer to the start of the payload, and
    allocation status (allocated or free).

4.  **writemem \<index\>, \<string\>**: Writes a string of characters
    starting at the specified memory address. The written data should be
    overwritten if the block is freed.

5.  **printmem \<index\>, \<count\>**: Prints the memory content
    starting at the specified address for the given number of bytes in
    decimal format.

6.  **quit**: Exits the program.

### Memory Management Strategies

-   **First Fit**: Allocates the first block large enough for the
    payload, splitting it if necessary.

-   **Best Fit**: Finds the smallest block that fits the payload,
    potentially avoiding wastage but may be slower due to full list
    searches.

### Requirements

-   **Splitting**: If a block is larger than needed but splitting would
    result in a block smaller than 3 bytes, allocate the entire block.

-   **Coalescing**: Combine adjacent free blocks into a single block
    upon freeing.

### More Info
For additional details, please refer to the specification document provided as specification.pdf.