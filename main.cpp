#include <iostream>
#include <fstream> //file
#include <math.h>
#include <time.h>

struct color {
	float r;
	float g;
	float b;
};

struct vec2 {
	float x;
	float y;
};

struct ivec2 {
	int x;
	int y;
};

//brocken metropolis hastings
const bool METRO_HASTE = false;

const float PI = 3.14159265359;

const int WIDTH = 360;
const int HEIGHT = 360;
const int PASSES = 64;
const int BUD_MAX_ITS = 5000;
const float DIVISOR = METRO_HASTE?96:15;

const float divs [3] = {1, 10, 100};


const float ZOOM = 0.75;
const float POS_X = 0.5;
const float POS_Y = 0.0;


/*
const float ZOOM = 1;
const float POS_X = 0;
const float POS_Y = 0;
*/

int colr [WIDTH][HEIGHT] = {};
int colg [WIDTH][HEIGHT] = {};
int colb [WIDTH][HEIGHT] = {};
float orbitX [BUD_MAX_ITS] = {};
float orbitY [BUD_MAX_ITS] = {};
float orbit2X [BUD_MAX_ITS] = {};
float orbit2Y [BUD_MAX_ITS] = {};
int reached = 0;
int reached2 = 0;

vec2 iRes;

float randFloat() {
	return (float) rand()/RAND_MAX;
}

float clamp(float w, float a, float b) {
	return fmax(fmin(w, b), a);
}

//cardioid/period 3 bulb detection
//reduces computation by ~25 times
bool bulbdec(float x, float y) {
	float a = 0.25;
	float b = -0.25;
	float t = (x+b)*(x+b)+y*y;
	bool card = (t*t+(x+b)*t-a*y*y)<0.0;
	if (card) { return true; }
	bool cir = ((x+1.0)*(x+1.0)+y*y)<0.0625;
	return cir;
}



bool m(float x, float y, int MAX_ITS, bool prime) {
	float r = x;
	float i = y;
	float r2;	
	float i2;
	for (int its = 0; its < MAX_ITS; its++) {
		r2 = r*r-i*i+x;
		i2 = 2*r*i+y;
		r=r2;
		i=i2;
		if (r*r+i*i>4.0) {	
			if (prime) { 
				reached2 = its; 
			} else { 
				reached = its; 
			}
			return true; 
		}
		if (prime) {
			orbit2X[its] = r;
			orbit2Y[its] = i;
		} else {
			orbitX[its] = r;
			orbitY[its] = i;
		}
	}
	return false;
}

color fetchColors(int xp, int yp) {
	float pass = PASSES;
	color retc;
	retc.r = (float)colr[xp][yp]/pass/DIVISOR;
	retc.g = (float)colg[xp][yp]/pass/DIVISOR;
	retc.b = (float)colb[xp][yp]/pass/DIVISOR;
	return retc;
}

ivec2 project(float xp, float yp) {
	ivec2 later;
	later.x = (((xp+POS_X)/ZOOM/2*iRes.y)+iRes.x)/2+0.5;
	later.y = (((yp+POS_Y)/ZOOM/2*iRes.y)+iRes.y)/2+0.5;
	return later;
}

bool inBounds(ivec2 p) {
	return (p.x>=0 && p.x<WIDTH)?((p.y>=0 && p.y<HEIGHT)?true:false):false;
}

bool inComplexBounds(float xp, float yp) {
	float d = fabs(xp-POS_X)+fabs(yp-POS_Y); // taxicab distance
	return (d < (2*ZOOM));
}

void renderOld(int pixelX, int pixelY) {
	vec2 uv;
	//uv.x = (2.0*(pixelX+randFloat()-0.5)-iRes.x)/iRes.y*2;
	//uv.y = (2.0*(pixelY+randFloat()-0.5)-iRes.y)/iRes.y*2;
	uv.x = randFloat()*4-2;
	uv.y = randFloat()*4-2;

	//colr[pixelX][pixelY] += bulbdec(uv.x, uv.y)?1:(m(uv.x, uv.y, 5000)?1:0);
	if (!bulbdec(uv.x, uv.y)) {
		for (int rgb = 0; rgb < 3; rgb++) {
			if (m(uv.x, uv.y, BUD_MAX_ITS/divs[rgb], false)) {
				if (rgb == 0) {
					for (int gorb = 0; gorb < reached; gorb++) {
						ivec2 incr = project(orbitX[gorb], orbitY[gorb]);
						if (inBounds(incr)) colr[incr.x][incr.y] += 1;
					}	
				} else if (rgb == 1) {
					for (int gorb = 0; gorb < reached; gorb++) {
						ivec2 incr = project(orbitX[gorb], orbitY[gorb]);
						if (inBounds(incr)) colg[incr.x][incr.y] += 1;
					}
				} else {
					for (int gorb = 0; gorb < reached; gorb++) {
						ivec2 incr = project(orbitX[gorb], orbitY[gorb]);
						if (inBounds(incr)) colb[incr.x][incr.y] += 1;
					}
				}
			}
		}
	}
}

void loopOld() {
	for (int pn = 0; pn < PASSES; pn++) {
		for (int y = 0; y < HEIGHT; y++) { 
			std::cerr << "\rRendering... " << pn*HEIGHT+y << std::flush;
			for (int x = 0; x < WIDTH; x++) {
				/*
				colr[x][y] += (int)clamp(floorf(randFloat()*256), 0, 255);
				*/

				renderOld(x, y);
			}
		}
	}
}

vec2 mutate1(float xp, float yp) {
	vec2 next;

	float r1 = (1/ZOOM) * 0.0001;
	float r2 = (1/ZOOM) * 0.1;
	float d = randFloat()*PI*2;
	float m = r2*expf(-logf(r2 / r1) * randFloat());

	next.x = xp + m * cos(d);
	next.y = yp + m * sin(d);

	return next;
}

vec2 mutate2() {
	vec2 next;
	next.x = randFloat()*4.0-2.0;
	next.y = randFloat()*4.0-2.0;

	return next;
}

float F(bool prime) {
	if (prime) {
		for (int o = 0; o < BUD_MAX_ITS; o++) {
			if (inComplexBounds(orbit2X[o], orbit2Y[o])) {
				return 1;
			}
		}
	} else {
		for (int o = 0; o < BUD_MAX_ITS; o++) {
			if (inComplexBounds(orbitX[o], orbitY[o])) {
				return 1;
			}
		}
	}
	return 0;
}

float transitionProbability(bool prime) {
	float mits = BUD_MAX_ITS;
	if (prime) {
		float l2 = reached2 - 1;
		return (1.0 - (mits - l2) / mits);
	} else {
		float l1 = reached - 1;
		return (1.0 - (mits - l1) / mits);
	}
}

void renderNew(vec2 uv, bool prime, bool training) {
	//colr[pixelX][pixelY] += bulbdec(uv.x, uv.y)?1:(m(uv.x, uv.y, 5000)?1:0);
	if (!bulbdec(uv.x, uv.y)) {
		if (m(uv.x, uv.y, BUD_MAX_ITS, prime)) {
			if ((!prime) && (!training)) {
				for(int gorb = 0; gorb < reached; gorb++) {
					ivec2 incr = project(orbitX[gorb], orbitY[gorb]);
					if (inBounds(incr)) {
						colr[incr.x][incr.y] += 1;
					}
				}
			}
		}
	}
}

void loopNew() {
	vec2 uv;
	vec2 uv2;

	uv = mutate2();

	// warm up the MH
	for (int traini = 0; traini < 1024; traini++) {
		if (randFloat() < 0.8) {
			uv2 = mutate1(uv.x, uv.y);
		} else {
			uv2 = mutate2();
		}

		renderNew(uv, false, true);
		renderNew(uv2, true, true);

		float f1 = F(false);
		float f2 = F(true);
		float tp = transitionProbability(false)/transitionProbability(true);
		float alpha = tp * f1 / f2;
		if (alpha < randFloat()) {
			uv = uv2;
		}
	}

	for (int pn = 0; pn < PASSES; pn++) {
		for (int y = 0; y < HEIGHT; y++) { 
			std::cerr << "\rRendering... " << pn*HEIGHT+y << std::flush;
			for (int x = 0; x < WIDTH; x++) {
				if (randFloat() < 0.8) {
					uv2 = mutate1(uv.x, uv.y);
				} else {
					uv2 = mutate2();
				}

				renderNew(uv, false, false);
				renderNew(uv2, true, false);

				float f1 = F(false);
				float f2 = F(true);
				float tp = transitionProbability(false)/transitionProbability(true);
				float alpha = tp * f1 / f2;
				if (alpha < randFloat()) {
					uv = uv2;
				}
			}
		}
	}
}

float goodness(bool prime) {
	return F(prime)*transitionProbability(prime);
}

void plot(bool prime) {
	if (prime) {
		for(int gorb = 0; gorb < reached2; gorb++) {
			ivec2 incr = project(orbit2X[gorb], orbit2Y[gorb]);
			if (inBounds(incr)) {
				colr[incr.x][incr.y] += 1;
				colg[incr.x][incr.y] += 1;
				colb[incr.x][incr.y] += 1;
			}
		}
	} else {
		for(int gorb = 0; gorb < reached; gorb++) {
			ivec2 incr = project(orbitX[gorb], orbitY[gorb]);
			if (inBounds(incr)) {
				colr[incr.x][incr.y] += 1;
				colg[incr.x][incr.y] += 1;
				colb[incr.x][incr.y] += 1;
			}
		}
	}
}

void loopFinal() {
	vec2 uv;
	vec2 uv2;
	bool option;
	float g1;
	float g2;

	option = true;
	uv = mutate2();
	if (bulbdec(uv.x, uv.y)) {
		g1 = 0;
	} else {
		m(uv.x, uv.y, 5000, false);
		g1 = goodness(false);
	}

	for (int pn = 0; pn < PASSES; pn++) {
		
		

		for (int y = 0; y < HEIGHT; y++) {
			
			std::cerr << "\rRendering... " << pn*HEIGHT+y << std::flush;

			for (int x = 0; x < WIDTH; x++) {
				if (option) {
					if (randFloat() < 0.8) 
						uv2 = mutate1(uv.x, uv.y);
					else 
						uv2 = mutate2();
					if (bulbdec(uv2.x, uv2.y)) {
						g2 = 0;
					} else {
						m(uv2.x, uv2.y, 5000, true);
						g2 = goodness(true);
					}

					if (g1/g2 < randFloat()) {
						if (pn == 0) {
							if (y != 0) {
								plot(false);
							}
						} else {
							plot(false);
						}
						option = false;
					}
				} else {
					if (randFloat() < 0.8) 
						uv = mutate1(uv2.x, uv2.y);
					else 
						uv = mutate2();
					if (bulbdec(uv.x, uv.y)) {
						g1 = 0;
					} else {
						m(uv.x, uv.y, 5000, false);
						g1 = goodness(false);
					}

					if (g2/g1 < randFloat()) {
						if (pn == 0) {
							if (y != 0) {
								plot(true);
							}
						} else {
							plot(true);
						}
						option = true;
					}
				}
			}
		}
	}
}

int main () 
{
	srand( (unsigned)time( NULL ) ); //init random

	std::cout << "Dimensions: " << WIDTH << " x " << HEIGHT << "\n";
	std::cout << "Total lines: " << PASSES*HEIGHT << "\n\n";

	iRes.x = WIDTH; iRes.y = HEIGHT;

	if (METRO_HASTE) 
		//loopNew();
		loopFinal();
	else 
		loopOld();

	/*
	ivec2 cid = project(0, 0);
	if (inBounds(cid)) {
		colr[cid.x][cid.y] = 255*PASSES;
	}
	*/

	std::ofstream fout("output.ppm");
	if (fout.fail()) return -1;
	fout << "P3\n" << WIDTH << " " << HEIGHT << "\n255\n";

	std::cout << "\n\nCreating image...\n";
	for (int j = HEIGHT - 1; j >= 0; --j)
	{
		for (int i = 0; i < WIDTH; ++i)
		{
			int y = HEIGHT - 1 - j;
			color writeColor = fetchColors(y, i);
			int ir = static_cast<int>(255.999 * clamp(writeColor.r * 0.8, 0, 1));
			int ig = static_cast<int>(255.999 * clamp(writeColor.g * 1.2, 0, 1));
			int ib = static_cast<int>(255.999 * clamp(writeColor.b * 2.2, 0, 1));

			fout << ir << " " << ig << " " << ib << "\n";
		}
	}
	std::cerr << "\nDONE.\n";
	fout.close();

	return 0;
}