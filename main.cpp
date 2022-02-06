#include <iostream>
#include <fstream>
#include <math.h>
#include <time.h>

const float divisor = 20;

const float zoom = 256;

const int width = 1024;
const int height = 1024;
const int iters = 4;
float col [width][height][3] = {};

float randFloat() {
	return (float) rand()/RAND_MAX;
}

float clamp(float w, float a, float b) {
	return (w<a)?a:((w>b)?b:w);
}

bool bulbdec(float x, float y) {
	float a = 0.25;
	float b = -0.25;
	float t = (x+b)*(x+b)+y*y;
	bool card = (t*t+4.0*a*(x+b)*t-4.0*a*a*y*y)<0.0;
	bool cir = ((x+1.0)*(x+1.0)+y*y)<0.0625;
	return (card || cir);
}

bool m(float x, float y, float maxits) {
	float r = x;
	float i = y;
	float r2;
	float i2;
	for (int its = 0; its < maxits; its++) {
		r2 = r*r-i*i+x;
		i2 = 2*r*i+y;
		r=r2;
		i=i2;
		if (r*r+i*i>4.0) { 			
			return true; 
		}
	}
	return false;
}

void mandelbrot(float px, float py, float z) {
	float x = (px+randFloat()-.5)/z;
	float y = (py+randFloat()-.5)/z;

	if (!bulbdec(x, y)) {
		if (m(x, y, 5000)) {
			float r = x;
			float i = y;
			float r2;
			float i2;
			for (int its = 0; its < 5000; its++) {
				r2 = r*r-i*i+x; 
				i2 = 2*r*i+y;
				r=r2; i=i2;
				col[(int)clamp(r*z+width/2.0, 0, width)][(int)clamp(i*z+height/2.0, 0, height)][0] += 0.8;	
				if (r*r+i*i>4) { break; }
			}
		}
		if (m(x, y, 500)) {
			float r = x;
			float i = y;
			float r2;
			float i2;
			for (int its = 0; its < 500; its++) {
				r2 = r*r-i*i+x; 
				i2 = 2*r*i+y;
				r=r2; i=i2;
				col[(int)clamp(r*z+width/2.0, 0, width)][(int)clamp(i*z+height/2.0, 0, height)][1] += 1.3;	
				if (r*r+i*i>4) { break; }
			}
		}
		if (m(x, y, 50)) {
			float r = x;
			float i = y;
			float r2;
			float i2;
			for (int its = 0; its < 50; its++) {
				r2 = r*r-i*i+x; 
				i2 = 2*r*i+y;
				r=r2; i=i2;
				col[(int)clamp(r*z+width/2.0, 0, width)][(int)clamp(i*z+height/2.0, 0, height)][2] += 1.8;	
				if (r*r+i*i>4) { break; }
			}
		}
	}
}

int main () 
{
	srand( (unsigned)time( NULL ) ); //init random

	std::cout << "Resolution: " << width << "x" << height << "\n";
	std::cout << "Passes: " << iters << "\n";
	std::cout << "Total: " << iters*width*height << " samples\n\n";

	for (float i = 0; i < iters; i++) {
		for (float y = 0; y < height; y++) {
			std::cerr << "\rRendering... " << (i*height+y)/(iters*height)*100 << "% " << std::flush;
			for (float x = 0; x < width; x++) {
				mandelbrot(x-width/2.0, y-height/2.0, zoom);
			}
		}
	}

	//output image
	std::ofstream fout("output.ppm");
	if (fout.fail()) return -1;
	fout << "P3\n" << width << " " << height << "\n255\n";

	for (int j = height - 1; j >= 0; --j)
	{
		std::cerr << "\rCreating image... " << (height-j)/(float)height*100 << "% " << std::flush;
		for (int i = 0; i < width; ++i)
		{
			int y = height - 1 - j;
			float r = col[i][y][0]/(float)iters/divisor;
			float g = col[i][y][1]/(float)iters/divisor;
			float b = col[i][y][2]/(float)iters/divisor;
			int ir = static_cast<int>(255.999 * clamp(r, 0, 1));
			int ig = static_cast<int>(255.999 * clamp(g, 0, 1));
			int ib = static_cast<int>(255.999 * clamp(b, 0, 1));

			fout << ir << " " << ig << " " << ib << "\n";
		}
	}
	std::cerr << "\n\nDONE.\n";
	fout.close();

	return 0;
}