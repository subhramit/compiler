/*
#                    GROUP - 8
# 2020B1A70630P                       Aditya Thakur
# 2021A7PS2001P                       Amal Sayeed
# 2020A7PS2005P                       Ohiduz Zaman
# 2020A7PS2682P                       Priyansh Patel
# 2020A7PS2002P                       Rachoita Das
# 2020B1A70611P                       Subhramit Basu Bhowmick
*/

#include <stdio.h>
#include <string.h>
#include "preprocessor.h"

void removeComments(char* input) {


    FILE* inputfile = fopen(input, "r"); //should be changed to "rb" if our language has a different file extension

    if(inputfile == NULL) {
        printf("Error opening the specified file!\n");
        return 2;
    }

    printf("No errors 1 and 2\n");
    char buf[2048]; //buffer to temporarily hold lines
    FILE* outputfile = fopen("output.txt", "w");
    char* commentSym;
    
    while(fgets(buf, sizeof(buf), inputfile) != NULL) { //loop through each line

        commentSym = strchr(buf, '%'); //strchr finds the position of % in the line
        if (commentSym != NULL) { //not a null pointer means % found
            *commentSym = '\n'; //change line
            *(commentSym+1) = '\0'; //truncate the rest of the line
        }

        fputs(buf, outputfile); //copy the modified line
    }

    fclose(inputfile);
    fclose(outputfile);
    
    return 0;
}