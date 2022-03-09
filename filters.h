#define SMOOTH_FILTER 0
#define BLUR_FILTER 1
#define SHARPEN_FILTER 2
#define MEAN_FILTER 3
#define EMBOSS_FILTER 4

float filtersValue[5] = {0.11111111111, 0.0625, 0.33333333333, 1, 1};
int filtersMatrix[5][3][3] = {
	//Smooth
	{
	    {1, 1, 1},
	    {1, 1, 1},
	    {1, 1, 1}
	},
	//Blur
	{
	    {1, 2, 1},
	    {2, 4, 2},
	    {1, 2, 1}
	},
	//Sharpen
	{
	    { 0, -2,  0},
	    {-2, 11, -2},
	    { 0, -2,  0}
	},
	//Mean
	{
	    {-1, -1, -1},
	    {-1,  9, -1},
	    {-1, -1, -1}
	},
	//Emboss -> rotated
	{
	    {0, -1, 0},
	    {0,  0, 0},
	    {0,  1, 0}
	}
};