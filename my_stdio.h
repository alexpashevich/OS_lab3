typedef struct _MY_FILE MY_FILE;

/* Opens an access to a file, where name is the path to the file and mode is either ”r” for read or ”w” for write.
Returns a pointer to a MY FILE structure upon success and NULL otherwise.*/
MY_FILE *my_fopen(char *name, char *mode);

/*Closes the access to the file associated to f. Returns 0 upon success and -1 otherwise.*/
int my_fclose(MY_FILE *f);

/*Reads at most nbelem elements of size size from file access f, that has to have been opened with mode ”r”, 
and places them at the address pointed by p. 
Returns the number of elements actually read, 0 if an end-of-file has been encountered before any element has been 
read and -1 if an error occurred.*/
int my_fread(void *p, size_t size, size_t nbelem, MY_FILE *f);

/*Writes at most nbelem elements of size size to file access f, that has to have been opened with mode ”w”, 
taken at the address pointed by p. Returns the number of elements actually written and -1 if an error occured.*/
int my_fwrite(void *p, size_t taille, size_t nbelem, MY_FILE *f);

int my_feof(MY_FILE *f);