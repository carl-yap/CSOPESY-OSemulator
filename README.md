# CSOPESY_OSemulator

A lightweight operating system emulator written in modern C++20, demonstrating core OS concepts including process scheduling, I/O handling, and immediate-mode console UI. Additionally, it can perform demand paging allocation.

created by Group 2: BACOSA, Gabriel | MANIPOL, Marion | TAN, Edward | YAP, Carl

## Prerequisites

- A C++ compiler (e.g., `g++`, `clang++`)
- A local copy of the project: `git clone https://github.com/carl-yap/CSOPESY-OSemulator.git`

## Compilation

To compile the project, run the following command in the project's root directory:

```bash
g++ -o os_emulator *.cpp -std=c++20 -Wall -Wextra -pedantic
```

## Running the Program

After successful compilation, execute the emulator with:

``` bash
./os_emulator
```

## Project Structure

| File	              | Purpose                              |
|---------------------|--------------------------------------|
| MainLoop.cpp	      | Entry point (main scheduler loop)    |
| Process.cpp	        | Representation of a runnable program |
| FCFSScheduler.cpp	  | FCFS scheduling implementation       |
| RRScheduler.cpp     | Round Robin scheduling impl.         |
| ConsoleManager.cpp	| Console display & process            |
| MainConsole.cpp     | Main menu display & command interpret|
| ProcessConsole.cpp  | Process info & screen-smi            |
| MarqueeConsole.cpp	| Immediate-mode text display          |
| MemoryAllocator.h 	| Implementation of Flat & Pagin       |
| DemandPagingAlloc..	| Immediate-mode text display          |
| SymbolTable.cpp	    | Management of variables in a process |
| Commands.cpp       	| Simulation of subroutines (READ, WRITE, etc.)    |

## Future Work

- Create a `.bat` file to open the emulator with one step
