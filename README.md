# Hierarchical Virtual Memory Implementation </a> <a href="https://www.cprogramming.com/" target="_blank" rel="noreferrer"> <img src="https://raw.githubusercontent.com/devicons/devicon/master/icons/c/c-original.svg" alt="c" width="40" height="40"/> </a> <a href="https://www.w3schools.com/cpp/" target="_blank" rel="noreferrer"> <img src="https://raw.githubusercontent.com/devicons/devicon/master/icons/cplusplus/cplusplus-original.svg" alt="cplusplus" width="40" height="40"/>

## Overview
This project implements a virtual memory system using hierarchical page tables. The system allows processes to use more memory than is physically available by mapping virtual addresses to physical addresses. The implementation utilizes a hierarchical structure of page tables to efficiently manage address translation, frame eviction, and read/write operations in the virtual memory.

## Features
- Hierarchical Page Tables: The virtual memory system employs a tree-like structure of page tables to manage address translation.
- Efficient Address Translation: Address translation is accomplished by traversing the page tables, enabling the mapping of virtual addresses to physical addresses.
- Page Fault Handling: When a requested page is not present in physical memory, the system performs page fault handling, swapping in the required page from secondary storage.
- Frame Eviction Strategy: The system intelligently selects frames to evict when there is no available space in physical memory for new pages, based on a cyclic distance metric.

## How to Use
To use the virtual memory system, follow these steps:

1. Include the VirtualMemory.h header file in your project.
2. Implement the required map and reduce functions in your code, as they will be used for page eviction strategies.
3. Initialize the virtual memory system by calling `VMinitialize()`.

### Reading from Virtual Memory
To read a value from the virtual memory, use the `VMread(virtualAddress, value)` function. Provide the virtual address you want to read from and a reference to store the retrieved value. The function returns 1 on success and 0 if the virtual address is out of bounds.

### Writing to Virtual Memory
To write a value to the virtual memory, use the `VMwrite(virtualAddress, value)` function. Provide the virtual address you want to write to and the value to be written. The function returns 1 on success and 0 if the virtual address is out of bounds.

## Building and Testing
1. Compile the virtual memory system by using the provided Makefile. Run the `make` command to generate the library.
2. Test your implementation using different memory configurations and tree depths to ensure robustness.

## Contact
For any questions or assistance, feel free to reach out to me at Eitan.Stepanov@gmail.com.
