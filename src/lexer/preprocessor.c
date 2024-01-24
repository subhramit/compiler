#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {

    if(argc!=2) {
        printf("Wrong input format. Please specify the name of the correct file.\n");
        return 1;
    }

    FILE* inputfile = fopen(argv[1], "r"); //should be changed to "rb" if our language has a different file extension

    if(inputfile == NULL) {
        printf("Error opening the specified file!\n");
        return 2;
    }

    printf("No errors 1 and 2\n");
    char buf[1024]; //buffer to temporarily hold lines
    FILE* outputfile = fopen("output.txt", "w");
    
    while(fgets(buf, sizeof(buf), inputfile) != NULL) {
        char *delimiterSym = strchr(buf, '%'); //strchr finds the position of % in the line
        if (delimiterSym != NULL) { //not a null pointer means % found
            *delimiterSym = '\0'; //since '%' found, truncate line by replacing it with \0, which denotes end of string
        }
        fputs(buf, outputfile); //copy the modified line to the destination file
    }
    fclose(inputfile);
    fclose(outputfile);
    
    return 0;
}