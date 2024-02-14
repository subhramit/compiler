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
    char buf[2048]; //buffer to temporarily hold lines
    FILE* outputfile = fopen("output.txt", "w");
    char* commentSym;
    
    while(fgets(buf, sizeof(buf), inputfile) != NULL) {
        char *commentSym = strchr(buf, '%'); //strchr finds the position of % in the line

        if (commentSym != NULL && buf==commentSym) { //not a null pointer means % found, and buf is the beginning of the line
            *commentSym = '\n'; //if the line is a comment, just insert new line
            *(commentSym+1)= '\0'; //truncate the rest of the line
            fputs(buf, outputfile); //copy the modified line
        }
        else if (commentSym != NULL) { //if the comment occure in te middle of code
            *commentSym = '\0'; //truncate the rest of the line
            fputs(buf, outputfile); //copy the modified line
            fputs("\n", outputfile); //change line
        }
        else {
            fputs(buf, outputfile); //simply copy the modified line to the destination file
        }
    }

    fclose(inputfile);
    fclose(outputfile);
    
    return 0;
}