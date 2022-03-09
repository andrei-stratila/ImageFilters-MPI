#include <string.h>
#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "filters.h"
#define PGM 5
#define PNM 6

int imageType = -1;
int width, height;

typedef struct Color {
	unsigned char r, g, b, value;
} Color;

typedef enum {RED, GREEN, BLUE, VALUE} ColorEnum; 

Color **matrix = NULL;
Color **afterFilterMatrix = NULL;

void getSizeImage(char *line, int lineSize) {
	int i = 0;
	while(line[i] != ' ' && i < lineSize)
		width = width * 10 + (line[i++] - '0');
	i++;
	while(line[i] != '\n' && i < lineSize)
		height = height * 10 + (line[i++] - '0');
	
}

/*
	Aloca o matrice ca o zona continua in memorie
	Prima oara n * m elemente de tip Color, apoi pointerii
	pentru fiecare linie din matrice
*/
void callocMatrixColor(Color ***color, int n, int m) {
    Color *p = (Color *)calloc(n * m, sizeof(Color));
    (*color) = (Color **)calloc(n, sizeof(Color*));

    for (int i = 0; i < n; i++) 
       (*color)[i] = &(p[i * m]);
}

void freeMatrixColor(Color ***color) {
    free(&((*color)[0][0]));
    free(*color);
}


void initMatrix() {
	callocMatrixColor(&matrix, height + 2, width + 2);
	callocMatrixColor(&afterFilterMatrix, height + 2, width + 2);
}

void readData(char *inputFilename) {
	FILE *fin = fopen(inputFilename, "r");
	
	char *buffer = NULL;
	size_t line_buf_size = 0;
	
	//Type
	getline(&buffer, &line_buf_size, fin);
	imageType = strcmp(buffer, "P6\n") ? PGM : PNM;
	
	//Comment
	getline(&buffer, &line_buf_size, fin);
	
	//Size
	int lineSize = getline(&buffer, &line_buf_size, fin);
	getSizeImage(buffer, lineSize);
	
	initMatrix();

	//MaxValue
	getline(&buffer, &line_buf_size, fin);

	//Values
	unsigned char colorValue = 0;
	for (int i = 1; i <= height; i++) {
		for (int j = 1; j <= width; j++) {
			if (imageType == PNM) {
				fread(&colorValue, sizeof(unsigned char), 1, fin);
				matrix[i][j].r = colorValue;
				fread(&colorValue, sizeof(unsigned char), 1, fin);
				matrix[i][j].g = colorValue;
				fread(&colorValue, sizeof(unsigned char), 1, fin);
				matrix[i][j].b = colorValue;
			}
			else {
				fread(&colorValue, sizeof(unsigned char), 1, fin);
				matrix[i][j].value = colorValue;
			}
		}
	}
	fclose(fin);
}

void writeData(char *outputFilename, int rank) {
	FILE *fout = fopen(outputFilename, "wb");
	if (imageType == PNM)
		fprintf(fout, "P6\n");
	else 
		fprintf(fout, "P5\n");	
	//fprintf(fout, "# Created by GIMP version 2.10.14 PNM plug-in\n");
	fprintf(fout, "%d %d\n", width, height);
	fprintf(fout, "255\n");
	unsigned char colorValue = 0;
	for (int i = 1; i <= height; i++) {
		for (int j = 1; j <= width; j++) {
			if (imageType == PNM) {
				colorValue = matrix[i][j].r;
				fwrite(&colorValue, sizeof(unsigned char), 1, fout);
				colorValue = matrix[i][j].g;
				fwrite(&colorValue, sizeof(unsigned char), 1, fout);
				colorValue = matrix[i][j].b;
				fwrite(&colorValue, sizeof(unsigned char), 1, fout);
			}
			else {
				colorValue = matrix[i][j].value;
				fwrite(&colorValue, sizeof(unsigned char), 1, fout);
			}
		}
	}
	fclose(fout);
}



unsigned char applyFilterToPixel(ColorEnum type, int filterIndex, int image_i, int image_j) {
	float res = 0;
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			switch(type) {
				case RED:
					res = res + (filtersValue[filterIndex] * matrix[image_i + i][image_j + j].r * filtersMatrix[filterIndex][i + 1][j + 1]);
					break;
				case GREEN:
					res = res + (filtersValue[filterIndex] * matrix[image_i + i][image_j + j].g * filtersMatrix[filterIndex][i + 1][j + 1]);
					break;
				case BLUE:
					res = res + (filtersValue[filterIndex] * matrix[image_i + i][image_j + j].b * filtersMatrix[filterIndex][i + 1][j + 1]);
					break;
				case VALUE:
					res = res + (filtersValue[filterIndex] * matrix[image_i + i][image_j + j].value * filtersMatrix[filterIndex][i + 1][j + 1]);
					break;
				default:
				 	break;
			}
		}
	}
	res = res > 255 ? 255 : res;
	res = res < 0 ? 0 : res;
	return (unsigned char)res;
}



void updateMatrix(int start, int end) {
	for (int i = start; i <= end; i++) {
		for (int j = 1; j <= width; j++) {
			matrix[i][j].r = afterFilterMatrix[i][j].r;
			matrix[i][j].g = afterFilterMatrix[i][j].g;
			matrix[i][j].b = afterFilterMatrix[i][j].b;
			matrix[i][j].value = afterFilterMatrix[i][j].value;
		}
	}
}

void applyFilterToImage(int filterIndex, int start, int end) {
	for (int i = start; i <= end; i++) {
		for (int j = 1; j <= width; j++) {
			if (imageType == PNM) {
				afterFilterMatrix[i][j].r = applyFilterToPixel(RED, filterIndex, i, j);
				afterFilterMatrix[i][j].g = applyFilterToPixel(GREEN, filterIndex, i, j);
				afterFilterMatrix[i][j].b = applyFilterToPixel(BLUE, filterIndex, i, j);
			}
			else {
				afterFilterMatrix[i][j].value = applyFilterToPixel(VALUE, filterIndex, i, j);
			}
		}
	}
}

int getFilterIndex(char *filterName) {
	if (!strcasecmp(filterName, "smooth"))
		return SMOOTH_FILTER;
	if (!strcasecmp(filterName, "blur"))
		return BLUR_FILTER;
	if (!strcasecmp(filterName, "sharpen"))
		return SHARPEN_FILTER;
	if (!strcasecmp(filterName, "mean"))
		return MEAN_FILTER;
	if (!strcasecmp(filterName, "emboss"))
		return EMBOSS_FILTER;
	printf("%s\n", filterName);
	printf("Error! Wrong filter name.\n");
	exit(0);
}

int main(int argc, char **argv) {
 	readData(argv[1]);

	int rank, numProc;
	MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProc);
 	
 	// Construirea unui nou tip de data MPI pentru structura Color
    const int tag = 13;
    const int nItems = 4;
    int blocklengths[4] = {1, 1, 1, 1};
    MPI_Datatype types[4] = {MPI_UNSIGNED_CHAR , MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR};
    MPI_Datatype mpi_Color_type;
    MPI_Aint offsets[4];
    offsets[0] = offsetof(Color, r);
    offsets[1] = offsetof(Color, g);
    offsets[2] = offsetof(Color, b);
    offsets[3] = offsetof(Color, value);
    MPI_Type_create_struct(nItems, blocklengths, offsets, types, &mpi_Color_type);
    MPI_Type_commit(&mpi_Color_type);

    // Segmentarea matricii pentru fiecare proces 
	float chunkSize = (float)height/numProc;
	int start = rank * ceil(chunkSize) + 1;
	int end = (rank + 1) * ceil(chunkSize) > height ? height : (rank + 1) * ceil(chunkSize);
	
	// Aplicarea filtrelor pe rand
    for (int i = 3; i < argc; i++) {
    	// Broadcast matrice imagine inainte de aplicarea filtrului
    	MPI_Bcast(&(matrix[0][0]), (height + 2) * (width + 2), mpi_Color_type, 0, MPI_COMM_WORLD);

		applyFilterToImage(getFilterIndex(argv[i]), start, end);
		
		if (rank != 0) {
			// Trimiterea matricii catre procesul cu rank 0 dupa aplicarea filtrului
			MPI_Send(&(afterFilterMatrix[0][0]), (height + 2) * (width + 2), mpi_Color_type, 0, tag, MPI_COMM_WORLD);
		}
		else {
			// Actualizarea matricei dupa filtru pentru procesul cu rank 0
			updateMatrix(start, end);
			MPI_Status status;
			// Primirea rezultatelor de la restul proceselor si actualizarea matricei imagine
			for (int i = 1; i < numProc; i++) {
				MPI_Recv(&(afterFilterMatrix[0][0]), (height + 2) * (width + 2), mpi_Color_type, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
				int start = status.MPI_SOURCE * ceil(chunkSize) + 1;
				int end = (status.MPI_SOURCE + 1) * ceil(chunkSize) > height ? height : (status.MPI_SOURCE + 1) * ceil(chunkSize);
				updateMatrix(start, end);
			}
		}
	}
	
	MPI_Type_free(&mpi_Color_type);
	MPI_Finalize();
	
	if (rank == 0)
		writeData(argv[2], rank);

	freeMatrixColor(&matrix);
	freeMatrixColor(&afterFilterMatrix);
	return 0;
}
