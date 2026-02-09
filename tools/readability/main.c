#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

#define IN_WORD 1
#define OUT_WORD 0

int parser(char *buffer, size_t strlength, float *letters, float *words, float *sentences);
void print(int index);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Argument required\n");
        return 1;
    }

    FILE *in = fopen(argv[1], "r");
    if (in == NULL) {
        fprintf(stderr, "File not found\n");
        return 1;
    }
    fseek(in, 0, SEEK_END);
    long size = ftell(in);
    rewind(in);

    char *buffer = malloc(size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "ALlocation failed\n");
        fclose(in);
        return 1;
    }
    size_t n = fread(buffer, 1, size, in);

    while (n > 0 && buffer[n - 1] == '\n') {
        n--;
    }

    buffer[n] = '\0';
    size_t strlength = strlen(buffer);

    fclose(in);

    float nletters = 0;
    float nwords = 1;
    float nsentences = 0;

    parser(buffer, strlength, &nletters, &nwords, &nsentences);

    int index = (int)round(0.0588 * nletters * 100 / nwords - 0.296 * nsentences * 100 / nwords - 15.8);
    free(buffer);

    print(index);

    return 0;
}

int parser(char *buffer, size_t strlength, float *letters, float *words, float *sentences) {
    int state;
    for (size_t i = 0; i < strlength; i++) {
        if (isalpha(buffer[i])) {
            (*letters)++;
        }
        if (isblank(buffer[i]) || buffer[i] == '\n') {
            state = OUT_WORD;
        } else if (state == OUT_WORD) {
            (*words)++;
            state = IN_WORD;
        }
        if (i != 0) {
            if ((buffer[i - 1] != '!' && buffer[i] == '!')
            || (buffer[i - 1] != '?' && buffer[i] == '?')
            || (buffer[i - 1] != '.' && buffer[i] == '.')) {
                (*sentences)++;
            }
        }
    }
    return 0;
}

void print(int index) {
    printf("Coleman-Liau formula: Based on average number of words in a sentence and average number of letters in words\n\n");

    if (index < 1) {
        printf("Kindergarten\n");
        return;
    } else if (index <= 6) {
        printf("1st to 6th grade, primary school\n");
        return;
    }

    const char *grades[] = {
        "7th grade\nAge range: 12-13 years old",
        "8th grade\nAge range: 13-14 years old",
        "9th grade\nAge range: 14-15 years old",
        "10th grade\nAge range: 15-16 years old",
        "11th grade\nAge range: 16-17 years old",
        "12th grade\nAge range: 17-18 years old"
    };

    if (index >= 7 && index <= 12) {
        printf("%s\n", grades[index - 7]);
    } else if (index < 16) {
        printf("%dth grade\n", index);
    } else {
        printf("Grade 16+\n");
    }
}
