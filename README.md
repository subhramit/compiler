# A toy compiler
Compiler design and implementation project in C for a high-level language (with specifications provided) for the course CS F363 (Compiler Construction), Spring '24, BITS Pilani.
## Modules
The compiler has a fully functional lexical analyzer, predictive parser, syntax checker and error recovery mechanism.
## Architecture
This uses a standard Pipe and Filter architecture as usually used for compilers. The input is a stream of data which flows from one component to another directly via a "pipe" (which has a single source of input and output), and is processed at different stages by various "filters" (independent components). Each component performs only one function, which establishes a loosely coupled system.

![Screenshot](architecture.png)
## Dependencies
The compiler has been developed and tested using **GCC 11.4.0** on **Ubuntu 22.04.3**. The project uses **GNU make** to build on **Linux**.
## To run the compiler
1. Run ````make```` to build the executable
2. Run ````./stage1exe <Input File> <Output File>````
3. Select your required option among the following: <br>
 **0:** To exit <br>
 **1:** To remove comments from the source file and print on the console <br>
 **2:** To perform lexical analysis and print token list on console <br>
 **3:** To perform syntax analysis and print the parse tree into the specified output file <br>
 **4:** To print total time taken (in CPU clock cycles and ms) for lexical and syntax analysis on console

## Credits
- [Aditya Thakur](https://github.com/cry0genic)
- [Amal Sayeed](https://github.com/amal-sayeed)
- [Ohiduz Zaman](https://github.com/Ohiduz)
- [Priyansh Patel](https://github.com/PriyanshPatelBits)
- [Rachoita Das](https://github.com/Rachoita-Das)
- [Subhramit Basu](https://github.com/subhramit)
## License
This piece of software is licensed under an [MIT License](https://opensource.org/licenses/MIT).  For more details, refer to the LICENSE file.
