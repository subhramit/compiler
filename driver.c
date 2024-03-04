/*
#                    GROUP - 8
# 2020B1A70630P                       Aditya Thakur
# 2021A7PS2001P                       Amal Sayeed
# 2020A7PS2005P                       Ohiduz Zaman
# 2020A7PS2682P                       Priyansh Patel
# 2020A7PS2002P                       Rachoita Das
# 2020B1A70611P                       Subhramit Basu Bhowmick
*/

// Drives the flow of execution to solve the given problem.
// Will contain the driver main()

#include <stdio.h>
#include <stdlib.h>
#include "src/parser/parserDef.h"
#include "src/lexer/lexer.h"
#include "src/parser/parserDef.h"
#include "src/parser/parser.h"
#include <time.h>


int main(int argc, char* argv[]) {

    if(argc!=3) {
        printf("Wrong execution format. Use: ./stage1exe <input_file.txt> <output_file.txt>\n");
        return -1;
    }

    int choice = -1;

    while(choice) {

        printf("Welcome to our compiler! Select your option:\n 0: To exit\n 1: To remove comments and print on console\n 2: To print token list on console\n 3: To parse and print the parse tree\n 4: To print total time taken on console\n");
        scanf("%d", &choice);

        switch(choice) {

            case 0: printf("Thank you!\n");
                    return 0;
                    break;

            case 1: removeComments(argv[1], "cleanedFile.txt");
                    printCleanFile("cleanedFile.txt");
                    break;

            case 2: FILE* lexIn = fopen(argv[1], "r");
                    linkedList* tokens = LexInput(lexIn);
                    printTokensOnConsole(tokens);
                    fclose(lexIn);
                    break;

            case 3: //parseInputSourceCode(argv[1], table T);
                    //printParseTree(parseTree PT, argv[2]);
                    break;
            
            case 4: clock_t start_time, end_time;
                    double total_CPU_time, total_CPU_time_in_seconds;

                    start_time = clock();

                    // invoke your lexer and parser here
                    //end
                    end_time = clock();
                    total_CPU_time = (double) (end_time - start_time);
                    total_CPU_time_in_seconds = total_CPU_time / CLOCKS_PER_SEC;

                    // Print both total_CPU_time and total_CPU_time_in_seconds
                    printf("Total CPU time (in clock cycles): %lf\n",total_CPU_time);
                    printf("Total CPU time (in seconds): %lf\n",total_CPU_time_in_seconds);
                    break;
            
            default: printf("Please enter a correct option!\n");
                     return 0;
        }
    }

    return 0;
}