#include <stdlib.h>
#include "my_stdio.h"
int main (int argc, char *argv[])
{
    MY_FILE *f1, *f2;
    char c;
    // for the sake of simplicity we don’t
    // print any error messages
    if (argc != 3)
        exit (-1);
    f1 = my_fopen(argv[1], "r");
    if (f1 == NULL)
        exit (-2);
    f2 = my_fopen(argv[2], "w");
    if (f2 == NULL)
        exit (-3);
    my_fprintf(f2, "Input file: %s\n", argv[1]);
    my_fscanf(f1, "%c", &c);
    while (!my_feof(f1)) {
        my_fprintf(f2, "Character %c read, its ASCII code is %d\n", c, (int)c);
        my_fscanf(f1, "%c", &c);
    }
    my_fclose(f1);
    my_fclose(f2);
    return 0;
}