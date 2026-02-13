// imitation of cat utility

#include <stdio.h>

void copy(FILE *i, FILE *o);

int main(int argc, char *argv[]) {
    FILE *fp;
    if (argc == 1) {
        copy(stdin, stdout);
    } else {
        while (--argc > 0) {
            if ((fp = fopen(*++argv, "r")) == NULL) {
                perror("cat");
                return 1;
            }
            copy(fp, stdout);
            fclose(fp);
        }
    }
}

void copy(FILE *i, FILE *o) {
    int c;
    while ((c = fgetc(i)) != EOF)
        putc(c, o);
}
