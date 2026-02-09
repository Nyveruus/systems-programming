/* The purpose of this program is to apply a greyscale filter onto a 24-bit .bmp image
   Usage: ./main FILENAME
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define BYTE uint8_t
#define DWORD uint32_t
#define LONG int32_t
#define WORD uint16_t

typedef struct tagBITMAPFILEHEADER {
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
} __attribute__((__packed__)) BMP_HEADER;

// struct adapted from https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader

typedef struct
{
    DWORD  biSize;
    LONG   biWidth;
    LONG   biHeight;
    WORD   biPlanes;
    WORD   biBitCount;
    DWORD  biCompression;
    DWORD  biSizeImage;
    LONG   biXPelsPerMeter;
    LONG   biYPelsPerMeter;
    DWORD  biClrUsed;
    DWORD  biClrImportant;
} __attribute__((__packed__)) BMP_INFO_HEADER;

// struct adapted from https://learn.microsoft.com/en-us/previous-versions//dd183376(v=vs.85)

typedef struct tagRGBTRIPLE {
    BYTE rgbtBlue;
    BYTE rgbtGreen;
    BYTE rgbtRed;
} __attribute__((__packed__)) RGBTRIPLE;

// struct adapted from https://learn.microsoft.com/en-us/previous-versions/aa922590(v=msdn.10)

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Argument required\n");
        return 1;
    }
    FILE *in = fopen(argv[1], "rb");
    if (in == NULL) {
        return 1;
    }
    BMP_HEADER header;
    BMP_INFO_HEADER info_header;

    fread(&header, sizeof(BMP_HEADER), 1, in);
    fread(&info_header, sizeof(BMP_INFO_HEADER), 1, in);

    if (header.bfType != 0x4d42 || info_header.biBitCount != 24) {
        fprintf(stderr, "Incorrect fileformat\nOnly 24 bit bmp supported\n");
        fclose(in);
        return 1;
    }

    int width = info_header.biWidth;
    int height = info_header.biHeight;
    int padding = (4 - (width * sizeof(RGBTRIPLE)) % 4) % 4;
    fseek(in, header.bfOffBits, SEEK_SET);

    RGBTRIPLE (*pixels)[width] = malloc(height * sizeof(RGBTRIPLE[width]));
    if (pixels == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(in);
        return 1;
    }

    for (int i = 0; i < height; i++) {
        fread(pixels[i], sizeof(RGBTRIPLE), width, in);
        fseek(in, padding, SEEK_CUR);
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            BYTE avg = (pixels[i][j].rgbtRed + pixels[i][j].rgbtGreen + pixels[i][j].rgbtBlue) / 3;
            pixels[i][j].rgbtRed = avg;
            pixels[i][j].rgbtGreen = avg;
            pixels[i][j].rgbtBlue = avg;
        }
    }

    FILE *out = fopen("greyscale.bmp", "wb");
    if (out == NULL) {
        fprintf(stderr, "Could not create output file\n");
        fclose(in);
        fclose(out);
        free(pixels);
        return 1;
    }

    fwrite(&header, sizeof(BMP_HEADER), 1, out);
    fwrite(&info_header, sizeof(BMP_INFO_HEADER), 1, out);

    for (int i = 0; i < height; i++) {
        fwrite(pixels[i], sizeof(RGBTRIPLE), width, out);
        for (int p = 0; p < padding; p++) {
            fputc(0x00, out);
        }
    }

    printf("Saved as greyscale.bmp\n");

    fclose(in);
    fclose(out);
    free(pixels);

    return 0;
}
