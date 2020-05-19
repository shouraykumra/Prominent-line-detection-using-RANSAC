//This algorithm works on canny.c that was provided by our professor,my code starts after 6th output file where seperated it with few dashes (RANSAC) it reads every image and outputs 5 lines that are most prominent. I took help from kmeans algorithm and other sources were



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dc_image.h"
#include <math.h>

#define CANNY_THRESH 30
#define CANNY_BLUR   4


#define MIN(a,b)  ( (a) < (b) ? (a) : (b) )
#define MAX(a,b)  ( (a) > (b) ? (a) : (b) )
#define ABS(x)    ( (x) <= 0 ? 0-(x) : (x) )

char *image_name(int o)    // this loop gives us access to all the images that we want to put in our code.
{
	if (o == 0)
	{
		return "building.png";
	}
	if (o == 1)
	{
		return "sidewalk.png";
	}
	if (o == 2)
	{
		return "puppy.png";
	}
	if (o == 3)
	{
		return "pentagon.png";
	}
	return NULL;
}

char *out_name(int o)    // this loop gives us access to all the output images that we want to save in the out file.
{
	if (o == 0)
	{
		return "out/building.png";
	}
	if (o == 1)
	{
		return "out/sidewalk.png";
	}
	if (o == 2)
	{
		return "out/puppy.png";
	}
	if (o == 3)
	{
		return "out/pentagon.png";
	}
	return NULL;
}



typedef struct Point {
	int r,g,b,x,y;   
} Point;


int main()
{
	int y,x;
	int rows, cols, chan;
	srand(time(NULL));
	//-----------------
	// Read the image
	//-----------------
	for (int o = 0; o < 4; o++)
		{
		char *path = image_name(o);
		char *out = out_name(o);
		byte ***img = LoadRgb(path, &rows, &cols, &chan);
		printf("img %p rows %d cols %d chan %d\n", img, rows, cols, chan);
	
		SaveRgbPng(img, "out/1_img.png", rows, cols);
	
	//-----------------
	// Convert to Grayscale
	//-----------------
		byte **gray = malloc2d(rows, cols);
		for (y=0; y<rows; y++){
			for (x=0; x<cols; x++) {
				gray[y][x] =  ((int)img[y][x][0] + (int)img[y][x][1] + (int)img[y][x][2] ) / 3;
			}
		}
	
		SaveGrayPng(gray, "out/2_gray.png", rows, cols);

	//-----------------
	// Box Blur   ToDo: Gaussian Blur is better
	//-----------------
		int k_x, k_y;
	// Box blur is separable, so separately blur x and y
		if (path=="puppy.png" || path=="building.png"){   //loops over to set 
			k_x=10, k_y=10;
		}
		else{
			k_x=4, k_y=4;
		}
	// blur in the x dimension
		byte **blurx = (byte**)malloc2d(rows, cols);
		for (y=0; y<rows; y++) {
			for (x=0; x<cols; x++) {
			
			// Start and end to blur
				int minx = x-k_x/2;      // k_x/2 left of pixel
				int maxx = minx + k_x;   // k_x/2 right of pixel
				minx = MAX(minx, 0);     // keep in bounds
				maxx = MIN(maxx, cols);
				
				// average blur it
				int x2;
				int total = 0;
				int count = 0;
				for (x2=minx; x2<maxx; x2++) {
					total += gray[y][x2];    // use "gray" as input
					count++;
				}
				blurx[y][x] = total / count; // blurx is output
			}
		}	
		
	// blur 	in the y dimension
		byte **blur = (byte**)malloc2d(rows, cols);
		for (y=0; y<rows; y++) {
			for (x=0; x<cols; x++) {
			
			// Start and end to blur
				int miny = y-k_y/2;      // k_x/2 left of pixel
				int maxy = miny + k_y;   // k_x/2 right of pixel
				miny = MAX(miny, 0);     // keep in bounds
				maxy = MIN(maxy, rows);
				
				// average blur it
				int y2;
				int total = 0;
				int count = 0;
				for (y2=miny; y2<maxy; y2++) {
					total += blurx[y2][x];    // use blurx as input
					count++;
				}
				blur[y][x] = total / count;   // blur is output
			}
		}	
		
		SaveGrayPng(blur, "out/3_blur.png", rows, cols);
		
	
	//-----------------
	// Take the "Sobel" (magnitude of derivative)
	//  (Actually we'll make up something similar)
	//-----------------
	
		byte **sobel = (byte**)malloc2d(rows, cols);
	
		for (y=0; y<rows; y++) {
			for (x=0; x<cols; x++) {
				int mag=0;
				
				if (y>0)      mag += ABS(blur[y-1][x] - blur[y][x]);
				if (x>0)      mag += ABS(blur[y][x-1] - blur[y][x]);
				if (y<rows-1) mag += ABS(blur[y+1][x] - blur[y][x]);
				if (x<cols-1) mag += ABS(blur[y][x+1] - blur[y][x]);
				
				sobel[y][x] = mag;
			}
		}
		
		SaveGrayPng(sobel, "out/4_sobel.png", rows, cols);
	
	//-----------------
	// Non-max suppression
	//-----------------
		byte **nonmax = malloc2d(rows, cols);    // note: *this* initializes to zero!
			
		for (y=1; y<rows-1; y++)
		{
			for (x=1; x<cols-1; x++)
			{
				// Is it a local maximum
				int is_y_max = (sobel[y][x] > sobel[y-1][x] && sobel[y][x]>=sobel[y+1][x]);
				int is_x_max = (sobel[y][x] > sobel[y][x-1] && sobel[y][x]>=sobel[y][x+1]);	
				if (is_y_max || is_x_max)
					nonmax[y][x] = sobel[y][x];
				else
					nonmax[y][x] = 0;
			}
		}
		
		SaveGrayPng(nonmax, "out/5_nonmax.png", rows, cols);
		
		//-----------------
		// Final Threshold
		//-----------------
		byte **edges = malloc2d(rows, cols);    // note: *this* initializes to zero!
		
		for (y=0; y<rows; y++) {
			for (x=0; x<cols; x++) {
				if (nonmax[y][x] > CANNY_THRESH)
					edges[y][x] = 255;   // write a code after this to save all the white points
				else
					edges[y][x] = 0;
			}
		}
		
		SaveGrayPng(edges, "out/6_edges.png", rows, cols);
		
		printf("Done!\n");
//--------------------------------------------------------------------------------------------------------------------------
		//RANSAC
		int nPoints=rows*cols;   //accessing all the points in the frame.
		Point *points= (Point*)malloc(nPoints * sizeof(Point));    
		byte ***edge=malloc3d(rows,cols,3);  // to save the white points
		//printf("edge= %p\n", edge);   //check for the location, if it is assigning a location or not.
	
		for (int N=0; N<5; N++){
			
			int maximum=0;   //sets the maximum value of the line so that it can be compared further.
			nPoints = 0;     // we will start from points = 0
			for (y=0; y<rows; y++){
				for (x=0; x<cols; x++){
					if(edges[y][x]==255){
						points[nPoints].x = x;
						points[nPoints].y = y;
						nPoints++;
	
					}
				//printf("x point is %d \n", x);   tried the points if colored or not.
				//printf("y point is %d \n", y);
	
				}
			}
			
		
// elem will generate the random point anywhere on the boundary.
	
			int i1, i2; 		//giving the two indexes so that the randoms points achieved can be replaced as we want to save the most prominent line.
			int h;
			for(h=0; h<1000; h++){    //Thousand iterations to check for every possibility
			// line of code to define distance is more than some value so that points dont overlap.
				int elem;
				elem=rand()%nPoints;
				int elem2 = (rand() % (elem+1) %(nPoints));

			//int elem3 = (rand() % (elem2)% (nPoints));

				int x1, x2, y1, y2, dist_1;
				x1=points[elem].y;    //points which will lie on the edges.
				y1=points[elem].x;
				x2=points[elem2].y;
				y2=points[elem2].x;
				int cout=0;          //reinitializing the new points
				float m=0;
				float c, d;
				float A,B;	
		// 	EQUATION OF THE LINE USED: (Ax+By+C)/sqrt(A^2,B^2)
		//      Breaking down the equation gives the following followings (professor helped to generate the following points)
				A=y2-y1;
				B=-(x2-x1);
				c=(y1-y2)*x1 + (x2-x1)*y1;
				float len;
				len=sqrt(A*A +B*B);
				A/=len;		//Dividing each point with sqrt(A^2 + B^2) to save us from core dump.
				B/=len;
				c/=len;
			
				for (int z=0; z<nPoints; z++){
					d= (float)ABS((A*points[z].y + B*points[z].x + c));
					if (d<10){
						cout++;
					}
				}
				if(cout>maximum){
					maximum=cout;
					i1= elem;
					i2=elem2;
				}
		
			}
	
	
	
			
			float c, d;
			float A,B;
			
			int x1, x2, y1, y2, dist_1;
			x1=points[i1].y;
			y1=points[i1].x;
			x2=points[i2].y;
			y2=points[i2].x;
			A=y2-y1;
			B=-(x2-x1);
			c=(y1-y2)*x1 + (x2-x1)*y1;     				//using same equation of the line as used above.
			float len;
			len=sqrt(A*A +B*B);
			A/=len;
			B/=len;
			c/=len;
				
			for (int z=0; z<nPoints; z++){
				d= (float)ABS((A*points[z].y + B*points[z].x + c));
				if (d<10){
					x=points[z].x;
					y=points[z].y;
					edges[y][x]=125;  //saving the points in a different color.
					
				}
			
			}
		// following for loops are for making points on the edges.
			for (y=points[i1].y; y<points[i1].y+8; y++) {
				for (x=points[i1].x; x<points[i1].x+8; x++) {
					edge[y][x][0] = 255;
					edge[y][x][1] = 0;
					edge[y][x][2] = 255;
				}
		
			}
		
			for (y=points[i2].y; y<points[i2].y+8; y++) {
				for (x=points[i2].x; x<points[i2].x+8; x++) {
					edge[y][x][0] = 255;
					edge[y][x][1] = 0;
					edge[y][x][2] = 255;
				}
		
			}

			
		}

	// following code checks the color, if it reads the desired color then it colors it into red, so final output is in red.
	// else if the does not gets the desired color then it will save it into white color and will start the loop over to get new line other than our previous.
		for (y=0; y<rows; y++) {
			for (x=0; x<cols; x++) {
				if (edges[y][x] == 125){
					edge[y][x][0]=255;
				}
				else if (edges[y][x]==255){
					edge[y][x][0]=255;
					edge[y][x][1]=255;
					edge[y][x][2]=255;
	
				}
			}
	
		}			
		SaveRgbPng(edge, out, rows, cols);
	}
	return 0;
}


100%


