#include <stdlib.h>
#include <stdio.h>
#include "my_stdio.h"

int main (int argc, char *argv[]) {
    MY_FILE *f1;
    MY_FILE *f2;
    char c;
    int result;

    // for the sake of simplicity we don't
    // print any error messages
    if (argc != 3)
        exit (-1);

    f1 = my_fopen(argv[1], "r");
    if (f1 == NULL)
        exit (-2);

    f2 = my_fopen(argv[2], "w");
    if (f2 == NULL)
        exit (-3);

    result = my_fread(&c, 1, 1, f1);
    // fprintf(stderr, "reading\n");
    while (result == 1) {
        // fprintf(stderr, "writing\n");
        result = my_fwrite(&c, 1, 1, f2);
        if (result == -1)
            exit(-4);
        result = my_fread(&c, 1, 1, f1);
        // fprintf(stderr, "reading\n");
    }
    if (result == -1)
        exit(-5);
    my_fclose(f1);
    my_fclose(f2);
    fprintf(stderr, "success\n");
    return 0;
}
