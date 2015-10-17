#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
// #include "my_stdio.h"

#define BUFFER_SIZE 100
#define min(a, b) (a < b ? a : b)

typedef struct {
    FILE *file;
    char *mode;
    char *buffer;
    int buf_size; // 0 when initialized, -1 if error during read/write occured
    int pointer; // -1 when initialized
    bool file_is_finished;
    bool eof_was_read;
} MY_FILE;

/* Opens an access to a file, where name is the path to the file and mode is either ”r” for read or ”w” for write.
Returns a pointer to a MY FILE structure upon success and NULL otherwise.*/
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
    // CHECK: not sure
    MY_FILE *myfile = (MY_FILE*) malloc(sizeof(MY_FILE));
    myfile->file = fd;
    myfile->mode = (char*) malloc(sizeof(char));
    memcpy(myfile->mode, mode, 1);
    myfile->buf_size = BUFFER_SIZE;
    myfile->pointer = -1;
    myfile->eof_was_read = false;
    myfile->file_is_finished = false;
    myfile->buffer = (char*) malloc(sizeof(char) * BUFFER_SIZE);
    return myfile;
}

/*Closes the access to the file associated to f. Returns 0 upon success and -1 otherwise.*/
int my_fclose(MY_FILE *f) {
    free(f->mode);
    free(f->buffer);
    free(f);
    // what else???
    return 0;
}

/*Reads BUFFER_SIZE bytes into the buffer. Also set f.pointer, f.buf_size, f.eof_was_read and f.file_is_finished.
Returns number of bytes read or 0 if the file is over. 
Returns -1 if error during system read occures and set f.buf_size to -1.*/
// int read_new_buffer(MY_FILE *f) {
//     if (f->eof_was_read) {
//         return 0;
//     }
//     int status = fread(f->buffer, 1, BUFFER_SIZE, f->file);
//     f->buf_size = status;
//     if (status < BUFFER_SIZE) {
//         // eof was reached or error occured
//         if (feof(f->file) != 0) {
//             // error occured
//             f->buf_size = -1;
//             status = -1;
//         } else {
//             // eof was reached
//             f->eof_was_read = true;
//             if (status == 0) {
//                 f->file_is_finished = true;
//             }
//         }
//     }
//     f->pointer = 0;
//     return status;
// }

/*Reads at most nbelem elements of size size from file access f, that has to have been opened with mode ”r”, 
and places them at the address pointed by p. 
Returns the number of elements actually read, 0 if an end-of-file has been encountered before any element has been 
read and -1 if an error occurred.*/
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
        if (f->pointer == -1 || f->pointer == f->buf_size) {
            f->pointer = 0;
            f->buf_size = 0;

            if (!f->eof_was_read) {
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
    // maybe divide returned value by nbelem
    return nbelem * size - size_rest;
}

/*Writes at most nbelem elements of size size to file access f, that has to have been opened with mode ”w”, 
taken at the address pointed by p. Returns the number of elements actually written and -1 if an error occured.*/
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
        if (f->pointer == -1) {
            f->pointer = 0;
        }

        int bytes_to_write = min(f->buf_size - f->pointer, size_rest);
        memcpy(p + p_pointer, f->buffer + f->pointer, bytes_to_write);
        f->pointer += bytes_to_write;
        p_pointer += bytes_to_write;
        size_rest -= bytes_to_write;

        if (f->pointer == f->buf_size) {
            if (fwrite(f->buffer, 1, f->buf_size, f->file) < f->buf_size) {
                fprintf(stderr, "MY_FWRITE: Error during fwrite()\n");
                return -1;
            }
            f->pointer = 0;
        }
    }
    return nbelem * size - size_rest;
}
/*Work only for mode = "r", return 1 if the file was readen till the end and 0 otherwise*/
int my_feof(MY_FILE *f) {
    if (f->mode[0] == 'w') {
        return 0;
    }
    return (f->file_is_finished ? 1 : 0);
}





