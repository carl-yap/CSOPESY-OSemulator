# CSOPESY_OSemulator

A lightweight operating system emulator written in modern C++20, demonstrating core OS concepts including process scheduling, I/O handling, and immediate-mode console UI.

## Prerequisites

- A C++ compiler (e.g., `g++`, `clang++`)

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
| SchedulerFCFS.cpp	  | FCFS scheduling implementation       |
| ConsoleManager.cpp	| Console display & process            |
| MarqueeConsole.cpp	| Immediate-mode text display          |

## Future Work

- Create a `.bat` file to open the emulator with one step
