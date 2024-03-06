# A toy compiler
Compiler design and implementation project in C for a high-level language (with specifications provided) for the course CS F363 (Compiler Construction), Spring '24, BITS Pilani.
## Modules
The compiler comprises of a fully functional preprocessor, lexical analyzer and predictive parser (syntax checker) with error recovery mechanism. <br>

<ins>**Preprocessor**</ins> - Removes comments from the source file (lines beginning with ```%```) and produces a clean file. <br>
<ins>**Lexer**</ins> - Converts the input source code into a stream of valid tokens (lexemes) acceptable as per the language specifications, and identifies lexical errors wherever applicable. Refer to the DFA to see how the tokens are accepted. Also refer to "tokens.txt" inside the "lexer" folder (or the Language Specifications document) to see the list of valid tokens. <br>
<ins>**Parser**</ins> - Parses the stream of tokens as per the grammar rules of the language using table-driven top-down predictive parsing and generates a parse tree, and identifies syntax errors wherever applicable. Refer to "grammar.txt" inside the "parser" folder (or the Modified LL(1) Grammar document) to see the grammar rules.
## Architecture
This uses a standard Pipe and Filter architecture as usually used for compilers. The input is a stream of data which flows from one component to another directly via a "pipe" (which has a single source of input and output), and is processed at different stages by various "filters" (independent components). Each component performs only one function, which establishes a loosely coupled system.

![Screenshot](architecture.png)
## Language specifications
The language supports primitive data types that are used as integers and real numbers. The language also supports record type and union type. The operations on variables of record type are addition and subtraction. These operations can be applied for two operands of record type. The scalar multiplication and division of record variables are also supported. Record type definitions are defined in any function but are available for any other function as well. The language supports modular code in terms of function, which uses call by value mechanism of parameter passing. The function may return many values of different types or may not return any value. The scope ofthe variables is local i.e. the variables are not visible outside the function where they are declared. The variables with prefix 'global' are visible outside the function and can be used within any function. <br> <br>
Sample code:
```
% Program1.txt
_statistics input parameter list [int c2dbc,int d7,int b2d567] output parameter list [real d2, real c3bcd];
  type real: c3 : global; c3 <---3;
  d2 <--- (c2dbc + d7 + b2d567)/c3;
  c3bcd <--- d2*3.23;
  return [d2,c3bcd];
end
```
A semicolon is used as the separator between two statements and a % sign is used to start the comment. The white spaces and comments are non-executable and should be cleaned as a preprocessing step of the lexical analyzer. <br> <br>
The function call is through the statements of following type:
```
type real : c4;
type real : d3cd6 ;
[c4, d3cd6] <--- call _statistics with parameters [2,3,5];
```
The infix expressions are used in assignment statements. The assignment operator is a bit unusual, a less than symbol followed by three continuous hyphen symbols. <br> <br>
The mathematical operations are many: addition, subtraction, multiplication and division which can be applied to both types of operands integer and real, provided both the operands are of the same type. The operations + and – also add and subtract records, while multiplication and division can be used to perform scalar multiplication and scalar division of record variables. <br> <br>
The program structure is modular such that all function definitions precede the main driver function. Function prototype declarations are not required. Each function definition must have declaration statements first and the return statement only at the end. A return statement is necessary for every function. All other statements such as assignment statements, conditional or iterative statements, input output statements etc. appear before the return statement. A function can have within it a record definition and the same should be available globally. <br>
### **Language Constructs:**
***Keywords*** <br>
The language supports keywords while, return, main, if, type, read, write, call, input, output, parameter, parameters, list, record, union, definetype, as and so on. A list of all keywords is given in the Language specification document. <br>

***Variable Identifiers*** <br>
The identifiers are the names with the following pattern: <br>
```
[b-d][2-7][b-d]*[2-7]*
```
The identifier can be of any length of size varying in the range from 2 to 20. <br> <br>
A sample list of valid identifiers is d2bbbb54, b5cdbcdbcd7654, c6dcdcbcc7722. <br>
The list of invalid identifiers is d2bdcbdcb5c, 2cdc765 and so on. <br>
An identifier cannot be declared multiple times in the same scope and it should be declared before its use. <br>
Also, an identifier declared globally cannot be declared anywhere else in function definitions. <br>

***Function Identifiers*** <br>
Function identifier names start with an underscore and must have the following pattern:
```
[_][a-z|A-Z][a-z|A-Z]*[0-9]*
```
i.e. a function name can have one or more number of English alphabet following the underscore. Also any number of digits can follow towards the trail. A function identifier is of maximum size of 30. <br>

***Data Types*** <br>
The language supports the following types: <br> <br>
<ins>**Integer type**</ins>: The keyword used for representing integer data type is int and will be supported by the underlying architecture. A statically available number of the pattern ````[0-9][0-9]*```` is of integer type. <br> <br>
<ins>**Real type**</ins>: The keyword used for representing integer data type is real and will be supported by the underlying architecture. A statically available real number has the pattern ````[0-9][0-9]*[.][0-9][0-9]```` and is of type real. The language also supports exponent and mantissa form of real number representation. The regular expression for the same is ````[0-9][0-9]*[.][0-9][0-9][E][+|-|∈][0-9][0-9]```` restricting to exactly two digits in the exponent part. <br> <br>
<ins>**Record type**</ins>: This is the constructed data type of the form of the Cartesian product of types of its constituent fields. For example, the following record is defined to be of type 'finance' and its actual type is ***<int, real, int>***, preserving the types and sequence of fields appearing in the record type definition: <br>
```
record #finance
  type int: value;
  type real:rate;
  type int: interest;
endrecord
```
A record type must have at least two fields in it, while there can be any more fields as well. <br>
A variable identifier of type finance is declared as follows:
```
type record #finance : d5bb45;
```
The names of fields start with any alphabet and can have names as words of English alphabet (only small case). The fields are accessed using a dot in an expression as follows:
```
d5bb45.value <--- 30;
d5bb45.rate <--- 30.5;
```
and so on. The types of these constructed names using variable name and record fields are same as field types defined in the record type definition correspondingly. Nested record definitions are supported in this language. However, the definition of the record type in nested positions cannot be used as standalone definitions. <br> <br>
A test case handling addition operation on two records and use of record variables in parameters list is depicted below. The record type #book declared in _main function is also visible in function _recordDemo1. The language supports name equivalence and not structural equivalence, which means that similar structured record definitions are treated different. For example, #new and #book are the two record types with similar structure (sequence and type of variables) but different names.
```
_recordDemo1 input parameter list [record #book d5cc34, record #book d2cd]
output parameter list[record #book d3];
  record #new
    type int : value;
    type real: cost;
  endrecord;
  d3<--- d5cc34 + d2cd;
  return [d3];
end

_main
  record #book
    type int : edition;
    type real: price;
  endrecord;
  type record #book b2;
  type record #book c2;
  type record #book d2;
  type record #new b7bc34;
  b2.edition <--- 3;
  b2.price <--- 24.95;
  c2.edition <--- 2;
  c2.price <--- 98.80;
  % following is a valid statement as the types of d2, b2 and c2 are same
  d2<--- b2+ c2;
  % Note that d2<--- b2 + b7bc34; is not type correct as the language follows name equivalence.
  % Note that the types of b2 and b7bc34 are not same.
  [d2]<--- call _recordDemo with parameters[b2,c2];
  write(d2);
end
```
An assignment statement with variables from two different record types is not allowed in the language. Also, once a record type is defined, its re-occurrence anywhere in the program is not allowed and is treated as an error. <br> <br>
A variable of record type can only be multiplied or divided by a scalar (integer or real) i.e. two record type variables cannot be multiplied together nor can be divided by the other. Two operands (variables) of record type can be added, subtracted from one provided the types of the operands match and both the operands are of record type. An addition/subtraction means addition/subtraction of corresponding field values, for example:
```
type record #finance : d5;
type record #finance : c4;
type record #finance : c3;
c3 <--- c4 + d5;
```
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
