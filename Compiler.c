#include <stdio.h>
#include <stdlib.h>
#include "basis.h"
#include "Syntactical.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return 1;
    }

    char *input = readFileContent(argv[1]);
    //printf("Input read: \n%s\n", input);
    setInput(input);

    while (getNextToken() != END);

    runSyntacticalFromTokens();  
    showTokens();
    done();

    free(input);
    return 0;
    
}
