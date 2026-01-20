#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum {
    OFFSET_HEX,
    OFFSET_DECIMAL
} offset_format_type;

typedef enum {
    DATA_HEX,
    DATA_BINARY
} data_format_type;

typedef struct {
    int width;
    int show_ascii;
    int show_offset;
    offset_format_type offset_format;
    data_format_type data_format;
    int grouping;
    char *filename;
    char *output_filename;
} hexdump_options;

int parse_arguments(int argc, char *argv[], hexdump_options *opts);
void hexdump(FILE *in, FILE *out, hexdump_options *opts);
void print_offset(FILE *out, hexdump_options *opts, size_t offset);
void print_data(FILE *out, hexdump_options *opts, unsigned char *buffer, size_t bytes_read);
void print_ascii(FILE *out, hexdump_options *opts, unsigned char *buffer, size_t bytes_read);
void help(void);

//define enums
//define struct struct
//set default settings in struct opts
//receive arguments, start here
//long option definitions in parsing
//receive arguments, start here
//parse options and arguments
//read file handle and write file handle
//encapsulate all dumping functions in wrapper function
//print offset hex OR decimal OR none (hex default), print hex OR print binary (hex default), different grouping
// (single function: 2, 4 or 8, default 2 for hex, default 4 for binary), print ascii OR none (single function, default print ascii)

int main(int argc, char *argv[]) {
    hexdump_options opts = {
      .width = 16,
      .show_ascii = 1,
      .show_offset = 1,
      .offset_format = OFFSET_HEX,
      .data_format = DATA_HEX,
      .grouping = 2,
      .filename = NULL,
      .output_filename = NULL
    };

    if (parse_arguments(argc, argv, &opts) != 0) {
        return 1;
    }

    FILE *in = fopen(opts.filename, "rb");
    if (in == NULL) {
        return 1;
    }

    FILE *out = stdout;
    if (opts.output_filename != NULL) {
        out = fopen(opts.output_filename, "w");
        if (out == NULL) {
            fclose(in);
            return 1;
        }
    }

    hexdump(in, out, &opts);

    fclose(in);
    if (out != stdout) {
        fclose(out);
    }
    return 0;
}

int parse_arguments(int argc, char *argv[], hexdump_options *opts) {
    static struct option long_options[] = {
        {"width",             required_argument,  NULL, 'w'},
        {"no-ascii",          no_argument,        NULL, 'a'},
        {"no-offset",         no_argument,        NULL, 'O'},
        {"decimal-offset",    no_argument,        NULL, 'd'},
        {"binary",            no_argument,        NULL, 'b'},
        {"output",            required_argument,  NULL, 'o'},
        {"group",             required_argument,  NULL, 'g'},
        {"help",              no_argument,        NULL, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "w:aOdbo:g:h", long_options, NULL)) != -1) {
        switch (opt) {

            case 'w':
                opts->width = atoi(optarg);
                if (opts->width < 1 || opts->width > 255) {
                    fprintf(stderr, "Error: width must be within 1-255\n");
                    return 1;
                }
                break;

            case 'a':
                opts->show_ascii = 0;
                break;

            case 'O':
                opts->show_offset = 0;
                break;

            case 'd':
                opts->offset_format = OFFSET_DECIMAL;
                break;

            case 'b':
                opts->data_format = DATA_BINARY;
                break;

            case 'o':
                opts->output_filename = optarg;
                break;

            case 'g':
                opts->grouping = atoi(optarg);
                if (opts->grouping != 1 && opts->grouping != 2 && opts->grouping != 4 && opts->grouping != 8) {
                    fprintf(stderr, "Error: byte grouping must be 2, 4 or 8\n");
                    return 1;
                }
                break;

            case 'h':
                help();
                return 1;

            case '?':
                fprintf(stderr, "Error: unknown option or unknown argument\nUse -h for help\n");
                return 1;

            default:
                fprintf(stderr, "Error: use -h for help\n");
                return 1;
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "Error: file name must be provided\n");
        return 1;
    }
    opts->filename = argv[optind];
    return 0;
}

//wrapper
void hexdump(FILE *in, FILE *out, hexdump_options *opts) {
//access to file in and file out file handles. Use fprintf for out in all cases.
//access to parsed options: width of line (data), ascii on or off, offset on or off, offset option hex (default) or decimal,
//data option hex (default) or binary, data grouping int 2 (default), 4, or 8. If bin and not specified, grouping 4 is used. Use case

    unsigned char buffer[256];
    size_t offset = 0;
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, opts->width, in)) > 0) {

        if (opts->show_offset) {
            print_offset(out, opts, offset);
        }

        print_data(out, opts, buffer, bytes_read);

        if (opts->show_ascii) {
            print_ascii(out, opts, buffer, bytes_read);
        }

        fprintf(out, "\n");
        offset += bytes_read;
    }
    return;
}

void print_offset(FILE *out, hexdump_options *opts, size_t offset) {
    switch (opts->offset_format) {
        case OFFSET_HEX:
            fprintf(out, "%08zX: ", offset);
            break;

        case OFFSET_DECIMAL:
            fprintf(out, "%08zu: ", offset);
            break;
    }
    return;
}

void print_data(FILE *out, hexdump_options *opts, unsigned char *buffer, size_t bytes_read_line) {
    for (int i = 0; i < bytes_read_line; i++) {
        //grouping
        if (i > 0 && i % opts->grouping == 0) {
            fprintf(out, " ");
        }

        switch (opts->data_format) {
            case DATA_HEX:
                fprintf(out, "%02X", buffer[i]);
                break;

            case DATA_BINARY:
                //TODO
                //for loop over bits in a byte. bitwise shift to the right, bitwise AND 1 to select leftmost bits progressively.
                //each iteration print bit as integer
                break;
        }
        if (opts->grouping == 1 || (i + 1) % opts->grouping == 0) {
            fprintf(out, " ");
        }
    }

    //TODO PADDING. Will require an if statement of some sorts to detect if expected amount of bytes have been printed, compare bytes read line and width in opts, when padding consider missing space from grouping
    //essentialy simulate printing the data with same logic but rather than data, it's spaces'
}

void print_ascii(FILE *out, hexdump_options *opts, unsigned char *buffer, size_t bytes_read) {
    fprintf(out, " ");
    for (size_t i = 0; i < bytes_read; i++) {
        fprintf(out, "%c", isprint(buffer[i]) ? buffer[i] : '.');
    }

    //TODO PADDING
    return;
}

void help(void) {
    printf("Usage: ./hex [options] file\n");
    printf("-w, --width integer\n");
    printf("-a, --no-ascii\n");
    printf("-O, --no-offset\n");
    printf("-d, --decimal-offset\n");
    printf("-b, --binary\n");
    printf("-o, --output\n");
    printf("-g, --group\n");
    printf("-h, --help\n");
    return;
}

/*
 unsigned char *dump(unsigned char absolute_path[]) {
 unsigned char buffer[100];
 printf("Image name: ");
 fgets(buffer, 100, stdin);
 buffer[strcspn(buffer, "\n")] = '\0';

 if (strlen(absolute_path) + strlen(buffer) >= 100) {
     fprintf(stderr, "Path too long\n");
     return NULL;
     }

     strcat(absolute_path, buffer);
     FILE *in = fopen(absolute_path, "rb");
     if (in == NULL) {
         fprintf(stderr, "File not found\n");
         return NULL;
         }

         fseek(in, 0, SEEK_END);
         long size = ftell(in);
         rewind(in);

         unsigned char *file = malloc(size);
         if (file == NULL) {
             fprintf(stderr, "Memory allocation failed\n");
             fclose(in);
             return NULL;
             }

             long bytes_read = fread(file, 1, size, in);
             if (bytes_read != size) {
                 fprintf(stderr, "Failed to read entire file\n");
                 fclose(in);
                 free(file);
                 return NULL;
                 }

                 unsigned char buff[17];
                 long offset = 0;

                 for (long i = 0; i < size; i++) {
                     if (i % 16 == 0) {
                         if (i > 0) {

                             buff[16] = '\0';
                             printf(" %s\n", buff);
                             }
                             printf("%08lX: ", offset);
                             offset += 16;
                             }

                             printf("%02X ", file[i]);

                             if (isprint(file[i])) {
                                 buff[i % 16] = file[i];
                                 } else {
                                     buff[i % 16] = '.';
                                     }
                                     }

                                     int remaining = size % 16;

                                     if (remaining == 0) {
                                         remaining = 16;
                                         }

                                         for (int i = remaining; i < 16; i++) {
                                             printf("   ");
                                             }

                                             buff[remaining] = '\0';
                                             printf(" %s\n", buff);

                                             fclose(in);
                                             return file;
                                             }
                                             */
