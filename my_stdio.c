#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#define BUFFER_SIZE 100
#define min(a, b) (a < b ? a : b)

typedef struct {
    FILE *file;
    char *mode;
    char *buffer;
    int buf_size; // initialized with BUFFER_SIZE value
    int pointer; // current position in buffer, -1 when initialized
    bool eof_was_read; // all the physical file was read to the buffer
    bool file_is_finished; // all the file was read by user
} MY_FILE;

MY_FILE *my_fopen(char *name, char *mode) {
    if (strlen(mode) != 1) {
        fprintf(stderr, "MY_FOPEN: Wrong open mode, should be 'w' or 'r'\n");
        return NULL;
    }
    if (mode[0] != 'w' && mode[0] != 'r') {
        fprintf(stderr, "MY_FOPEN: Wrong open mode, should be 'w' or 'r'\n");
        return NULL;
    }
    FILE *fd = fopen(name, mode);
    if (!fd) {
        fprintf(stderr, "MY_FOPEN: Error during opening the file\n");
        return NULL;
    }

    MY_FILE *myfile = (MY_FILE*) malloc(sizeof(MY_FILE));
    myfile->file = fd;
    myfile->mode = (char*) malloc(sizeof(char));
    myfile->mode[0] = mode[0];
    myfile->buf_size = BUFFER_SIZE;
    myfile->pointer = -1;
    myfile->eof_was_read = false;
    myfile->file_is_finished = false;
    myfile->buffer = (char*) malloc(BUFFER_SIZE);
    return myfile;
}

int my_fclose(MY_FILE *f) {
    int result = 0;
    if (f->mode[0] == 'w') {
        if (f->pointer > 0) {
            // if something was not written to the physical file from the buffer
            if (fwrite(f->buffer, 1, f->pointer, f->file) < f->pointer) {
                // error occured
                result = -1;
            }
        }
    }
    fclose(f->file);
    free(f->mode);
    free(f->buffer);
    free(f);
    return result;
}

int my_fread(void *p, size_t size, size_t nbelem, MY_FILE *f) {
    if (!f) {
        fprintf(stderr, "MY_FREAD: f == NULL\n");
        return -1;
    }
    if (f->mode[0] != 'r') {
        fprintf(stderr, "MY_FREAD: File was not opened in read mode\n");
        return -1;
    }
    if (p == NULL) {
        fprintf(stderr, "MY_FREAD: Pointer was not allocated\n");
        return -1;
    }
    if (size < 1 || nbelem < 1) {
        fprintf(stderr, "MY_FREAD: Incorrect nbelem or size\n");
        return -1;
    }

    int p_pointer = 0;
    int size_rest = nbelem * size;
    while (size_rest > 0 && !f->file_is_finished) {
        // until we read everything that user asked for or eof is reached
        if (f->pointer == -1 || f->pointer == f->buf_size) {
            // if file was just opened or buffer is full
            // in both cases we need to read some bytes
            f->pointer = 0;
            f->buf_size = 0;

            if (!f->eof_was_read) {
                // if f->eof_was_read we can not read more and leave the function
                int status = fread(f->buffer, 1, BUFFER_SIZE, f->file);
                f->buf_size = status;
                if (status < BUFFER_SIZE) {
                    // eof was reached or error occured
                    if (feof(f->file) == 0) {
                        // error occured
                        fprintf(stderr, "MY_FREAD: Error during fread()\n");
                        f->pointer = -1;
                        return -1;
                    } else {
                        // eof was reached
                        f->eof_was_read = true;
                    }
                }
            }
        }

        if (f->buf_size == 0) {
            f->file_is_finished = true;
        }

        int bytes_to_read = min(f->buf_size - f->pointer, size_rest);
        memcpy(p + p_pointer, f->buffer + f->pointer, bytes_to_read);
        f->pointer += bytes_to_read;
        p_pointer += bytes_to_read;
        size_rest -= bytes_to_read;
    }
    return nbelem - size_rest / size;
}

int my_fwrite(void *p, size_t size, size_t nbelem, MY_FILE *f) {
    if (!f) {
        fprintf(stderr, "MY_FWRITE: f == NULL\n");
        return -1;
    }
    if (f->mode[0] != 'w') {
        fprintf(stderr, "MY_FWRITE: File was not opened in write mode\n");
        return -1;
    }
    if (p == NULL) {
        fprintf(stderr, "MY_FWRITE: Pointer was not allocated\n");
        return -1;
    }
    if (size < 1 || nbelem < 1) {
        fprintf(stderr, "MY_FWRITE: Incorrect nbelem or size\n");
        return -1;
    }

    int p_pointer = 0;
    int size_rest = nbelem * size;
    while (size_rest > 0) {
        // until we write everything user asked for
        if (f->pointer == -1) {
            // file was just opened
            f->pointer = 0;
        }

        int bytes_to_write = min(f->buf_size - f->pointer, size_rest);
        memcpy(f->buffer + f->pointer, p + p_pointer, bytes_to_write);
        f->pointer += bytes_to_write;
        p_pointer += bytes_to_write;
        size_rest -= bytes_to_write;

        if (f->pointer == f->buf_size) {
            // if buffer is full
            if (fwrite(f->buffer, 1, f->buf_size, f->file) < f->buf_size) {
                fprintf(stderr, "MY_FWRITE: Error during fwrite()\n");
                return -1;
            }
            f->pointer = 0;
        }
    }
    return nbelem - size_rest / size;
}

int my_feof(MY_FILE *f) {
    if (f->mode[0] == 'w') {
        // eof is impossible in writing mode
        return 0;
    }
    return (f->file_is_finished ? 1 : 0);
}

int my_fprintf(MY_FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    int i, d;
    char c, *s;
    for (i = 0; i < strlen(format); ++i) {
        // iterate over the format string and either copy it to f or replace %s, %d, %c by provided args
        if (format[i] != '%') {
            my_fwrite(format + i, 1, 1, f);
        } else {
            ++i;
            if (i == strlen(format)) {
                // if we reach the end of string right after we read % - so string is smth like "...%" which is incorrect
                fprintf(stderr, "MY_FPRINTF: Incorrect format string\n");
                return -1;
            }
            switch(format[i]) {
                case 'c' :
                    c = (char) va_arg(args, int);
                    // type int used because of the realization of va_arg
                    // http://stackoverflow.com/questions/28054194/char-type-in-va-arg
                    if (my_fwrite(&c, 1, 1, f) == -1) {
                        fprintf(stderr, "MY_FPRINTF: Error during my_fwrite()\n");
                        return -1;
                    }
                    break;
                case 's' :
                    s = va_arg(args, char *);
                    if (my_fwrite(s, 1, strlen(s), f) == -1) {
                        fprintf(stderr, "MY_FPRINTF: Error during my_fwrite()\n");
                        return -1;
                    }
                    break;
                case 'd' :
                    d = va_arg(args, int);
                    fprintf(stderr, "%d read\n", d);
                    int d_pow = 0, d_char, status = 0;
                    // we want d_pow: 10^d_pow < d < 10^(d_pow + 1)
                    while (d >= (int) pow(10, d_pow + 1)) {
                        ++d_pow;
                    }
                    while (status != -1 && d_pow >= 0) {
                        d_char = d / (int) pow(10, d_pow);
                        // d_char is the biggest digit
                        d %= (int) pow(10, d_pow);
                        // store the rest in d
                        --d_pow;
                        c = '0' + d_char;
                        status = my_fwrite(&c, 1, 1, f);
                    }
                    if (status == -1) {
                        fprintf(stderr, "MY_FPRINTF: Error during my_fwrite()\n");
                        return -1;
                    }
                    break;
                default :
                    fprintf(stderr, "MY_FPRINTF: Incorrect format string\n");
                    return -1;
            }
        }
    }
    va_end(args);
    return 0;
}

int my_fscanf(MY_FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    int i, j, status, *d;
    char *c, *s;
    for (i = 0; i < strlen(format); ++i) {
        switch(i % 3) {
            case 0:
                if (format[i] != '%') {
                    fprintf(stderr, "MY_FSCANF: Incorrect format string, expected \"%%q %%q ...\" where q in {c, s, d}\n");
                    return -1;
                }
                break;
            case 1:
                switch(format[i]) {
                    case 'c' :
                        c = va_arg(args, char*);
                        if (my_fread(c, 1, 1, f) == -1) {
                            fprintf(stderr, "MY_FSCANF: Error during my_fread()\n");
                            return -1;
                        }
                        break;
                    case 's' :
                        s = va_arg(args, char*);
                        j = 0;
                        status = my_fread(s, 1, 1, f);
                        // read char by char and stop when \n is reached
                        while (status != -1 && s[j] != '\n') {
                            ++j;
                            status = my_fread(s + j, 1, 1, f);
                        }
                        if (status == -1) {
                            fprintf(stderr, "MY_FSCANF: Error during my_fread()\n");
                            return -1;
                        }
                        break;
                    case 'd' :
                        d = va_arg(args, int*);
                        j = 0;
                        // write result here and then put it in *d
                        int result = 0;
                        status = my_fread(c, 1, 1, f);
                        if (c[0] < '0' || c[0] > '9') {
                            fprintf(stderr, "MY_FSCANF: input which is supposed to be an integer is not an integer\n");
                            return -1;
                        }
                        // read char by char and stop when one if them
                        while (c[0] >= '0' && c[0] <= '9') {
                            result += atoi(c) * (int) pow(10, j);
                            ++j;
                            status = my_fread(c, 1, 1, f);
                        }
                        if (status == -1) {
                            fprintf(stderr, "MY_FSCANF: Error during my_fread()\n");
                            return -1;
                        }
                        *d = result;
                        break;
                    default :
                        fprintf(stderr, "MY_FPRINTF: Incorrect format string\n");
                        return -1;
                }
                break;
            case 2:
                if (format[i] != ' ') {
                    fprintf(stderr, "MY_FSCANF: Incorrect format string, expected \"%%q %%q...\" where q in {c, s, d}\n");
                    return -1;
                }
        }
    }
    return 0;
}








