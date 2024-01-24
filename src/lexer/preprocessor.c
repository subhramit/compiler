#include <stdio.h>

int main(int argc, char* argv[])
{
    if(argc!=2) {
        printf("Wrong input format. Please specify the name of the correct file.\n");
        return 1;
    }
    else{
        FILE* inputfile = fopen(argv[1], "r"); //should be changed to "rb" if our language has a different file extension

        if(inputfile != NULL)
        {
            printf("Success\n");
            FILE* outputfile = fopen("output.txt", "w");
            char ch;
            while ((ch = fgetc(inputfile)) != EOF) {
                if(ch!='%') fputc(ch, outputfile);
                else //
    }
            fclose(inputfile);
            fclose(outputfile);
        }
        else {
            printf("Error opening the specified file!\n");
            return 2;
        }
    }

    return 0;
}