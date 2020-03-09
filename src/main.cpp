#include "discpp.h"
#include "fftw3.h"

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <conio.h>
#include <complex>
#include <math.h>
#include <mutex>
#include <condition_variable>
#include<thread>

#include <unsupported/Eigen/MatrixFunctions>
#include <Eigen/Dense>
#include <Eigen/Core>

#include <assert.h>

using namespace std;

//define macros for fftw_complex
#define REAL 0
#define IMAG 1

const double T = 50;

const double PI = 3.14159265358979323846;
const int N = 255; //number of given points (must be odd)
const double dx = 2 * PI / (N - 1); //range between two points
const double range = dx * (N - 1); //total domain range
const double dt = dx * dx / PI; //time step
const int maxTime = T/dt; //number of steps in simulation 
const double h = 1;  //h-bar
const double m = 1; //particle mass
const int c = 1;//speed of light
const complex<double> I(0, 1); //sqruare root of -1
const double G = 1; //nonlinearity coeficient
const double u = 5;


float x[N], y[N]; //points to plot

Dislin g; //plotting object
int t = 0; //actual time

const unsigned numberOfRings = 24; //nuber of rings

//corelation between rings cooficients
complex<double> J[N][numberOfRings];

const complex<double> J0 = 1.1;
complex<double> J1[N];
complex<double> J2[N];
double w = 1;

const double vortex = 5.0;

const int resolutionPoints = 4000;
const int resolution = maxTime/resolutionPoints; //for how many steps of simulation one calculation of topological charge
//const int resolution = 500;
Eigen::Matrix<complex<double>, numberOfRings, 1> functionVec[N];
Eigen::Matrix<complex<double>, numberOfRings, numberOfRings> C[N]; //matrix of cooficients

complex<double> function1[N]; //wave function on first ring
complex<double> function2[N]; //wave function on second ring
complex<double> function3[N]; //wave function on third ring

//converting fftw_complex to  comlex<double>
complex<double> toComplex(fftw_complex a) {
	return complex<double>(a[REAL], a[IMAG]);
}

complex<double>* fixOrder(fftw_complex* a) {
	complex<double>* b;
	b = new complex<double>[N];
	for (int i = 0; i < N / 2; ++i) {
		b[i] = toComplex(a[N / 2 + i + 1]);
	}
	for (int i = 0; i <= N / 2; ++i) {
		b[N / 2 + i] = toComplex(a[i]);
	}
	return b;
}

complex<double>* fixOrder(complex<double>* a) {
	complex<double>* b;
	b = new complex<double>[N];
	for (int i = 0; i < N / 2; ++i) {
		b[i] = a[N / 2 + i + 1];
	}
	for (int i = 0; i <= N / 2; ++i) {
		b[N / 2 + i] = a[i];
	}
	return b;
}
complex<double>* fixOrder(Eigen::Matrix<complex<double>, numberOfRings, 1> * a, int ring) {
	complex<double>	*b = new complex<double>[N];
	for (int i = 0; i < N / 2; ++i) {
		b[i] = a[N / 2 + i + 1][ring];
	}
	for (int i = 0; i <= N / 2; ++i) {
		b[N / 2 + i] = a[i][ring];
	}
	return b;
}

double* fixOrder(double* a) {
	double* b;
	b = new double[N];
	for (int i = 0; i < N / 2; ++i) {
		b[i] = a[N / 2 + i + 1];
	}
	for (int i = 0; i <= N / 2; ++i) {
		b[N / 2 + i] = a[i];
	}
	return b;
}
float* fixOrder(float* a) {
	float* b;
	b = new float[N];
	for (int i = 0; i < N / 2; ++i) {
		b[i] = a[N / 2 + i + 1];
	}
	for (int i = 0; i <= N / 2; ++i) {
		b[N / 2 + i] = a[i];
	}
	return b;
}

//ploting function with lines
void show(double* a) {
	double* temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = temp[i];
	}
	g.qplot(x, y, N);
}

//ploting  function with points
void showP(double* a) {
	double* temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = temp[i];
	}
	g.qplsca(x, y, N);
}

void showM(Eigen::Matrix<complex<double>, numberOfRings, 1> * a) {
	for (int i = 0; i < numberOfRings; ++i){
		g.disini();
		std::string tmpStr = "modul funkcji na okregu " + std::to_string(i+1) + " po przepropagowaniu o " + std::to_string(t) + " krokow przy dt= " + std::to_string(dt) + ", T=" + std::to_string(dt * t);
		g.titlin(tmpStr.c_str(), 3);
		complex<double> * temp = fixOrder(a, i);
		for (int i = 0; i < N; ++i) {
			y[i] = abs(temp[i]);
		}
		g.qplot(x, y, N);
		delete[] temp;
		}
}

void showM(fftw_complex* a) {
	complex<double>* temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = abs(temp[i]);
	}
	g.qplot(x, y, N);
	delete[] temp;
}

void showR(fftw_complex* a) {
	complex<double>* temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = temp[i].real();
	}
	g.qplot(x, y, N);
}

void showR(complex <double>* a) {
	complex<double>* temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = temp[i].real();
	}
	g.qplot(x, y, N);
}

void showIm(Eigen::Matrix<complex<double>, numberOfRings, 1> * a) {
	for (int i = 0; i < numberOfRings; ++i) {
		g.disini();
		std::string tmpStr = "imaginary part on ring number " + std::to_string(i + 1) + " after " + std::to_string(t) + " time steps with dt= " + std::to_string(dt) + ", T=" + std::to_string(dt * t);
		g.titlin(tmpStr.c_str(), 3);
		complex<double>* temp = fixOrder(a, i);
		for (int i = 0; i < N; ++i) {
			y[i] = temp[i].imag();
		}
		g.qplot(x, y, N);
		delete[] temp;
	}
}

void showIm(fftw_complex* a) {
	complex<double>* temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = temp[i].imag();
	}
	g.qplot(x, y, N);
}

void showM(complex<double>* a) {
	complex<double>* temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		//y[i] = sqrt(norm(temp[i]));
		y[i] = abs(temp[i]);
	}
	g.qplot(x, y, N);
}

void showIm(complex<double>* a) {
	complex<double>* temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = temp[i].imag();
	}
	g.qplot(x, y, N);
}

//int topologicalCharge(complex<double>* tab) {
//	tab = fixOrder(tab);
//	double argument = arg(tab[N-1]);
//	double first = argument;
//	int result = 0;
//	double temp;
//	for (int i = 0; i < N; ++i) {
//		temp = argument;
//		argument = arg(tab[i]);
//		if (argument > temp + 2)
//			--result;
//		if (temp > argument + 2)
//			++result;
//	}
//	if (argument > first + 2)
//		--result;
//	if (first > argument + 2)
//		++result;
//	delete[] tab;
//	return result;
//}

void topologicalCharge(complex<double>* tab, float *val) {
	tab = fixOrder(tab);
	double argument = arg(tab[N - 1]);
	double first = argument;
	int result = 0;
	double temp;
	for (int i = 0; i < N; ++i) {
		temp = argument;
		argument = arg(tab[i]);
		if (argument > temp + 2)
			--result;
		if (temp > argument + 2)
			++result;
	}
	if (argument > first + 2)
		--result;
	if (first > argument + 2)
		++result;
	delete[] tab;
	*val=result;
}

void topologicalCharge(Eigen::Matrix<complex<double>,numberOfRings,1>  *tab, float* val, int ring) {
	complex<double> *tab2 = fixOrder(tab,ring);
	double argument = arg(tab2[N - 1]);
	double first = argument;
	int result = 0;
	double temp;
	for (int i = 0; i < N; ++i) {
		temp = argument;
		argument = arg(tab2[i]);
		if (argument > temp + 2)
			--result;
		if (temp > argument + 2)
			++result;
	}
	if (argument > first + 2)
		--result;
	if (first > argument + 2)
		++result;
	delete[] tab2;
	*val = result;
}


////////////////////multithreading///////////////////


bool mainReady = true;
bool processed[numberOfRings] = { false };
bool processed1 = false, processed2 = false, processed3 = false;

int index = 0;



//empty functions for synchronization, must be excluded from optimalization
#pragma optimize( "", off )
void waitForMain() {
	while (!mainReady) {
			std::this_thread::yield();
	}
}
void waitForNotMain() {
	while (mainReady) {
		std::this_thread::yield();
	}
}
void waitForProcessed() {
	//while (!(processed1 && processed2 && processed3)) {}
	bool result = false;
	while (!result) {
		std::this_thread::yield();
		result = true;
		for (int i = 0; i < numberOfRings; ++i) {
			result &= processed[i];
		}
	}
}
void waitForNotProcessed() {
	//while (processed1 || processed2 || processed3) {}
	bool result = true;
	while (result) {
		std::this_thread::yield();
		result = false;
		for (int i = 0; i < numberOfRings; ++i) {
			result |= processed[i];
		}
	}
}
#pragma optimize( "", on )


void computeRing(int ringNumber, complex<double> *temp, double k, complex<double>* inC, fftw_complex* outC, 
	fftw_plan p, fftw_plan pInverse, fftw_complex *out,const int range2, bool &processed, float *topCharge) {
	
	double norma = 0;

	while (t < maxTime) {

		waitForMain();

		//topological charge calculation
		if (t % resolution == 0) {
			topologicalCharge(functionVec, &topCharge[index], ringNumber);
		}

		//split-step calculation
		for (int i = 0; i < N; ++i) {
			
			temp[i] = exp(dt * (((-I - G) * norm(functionVec[i][ringNumber])) + u)) * C[i].row(ringNumber) * functionVec[i];
			//temp[i] = exp(dt * (((-I - G) * norma) + u)) * C[i].row(ringNumber) * functionVec[i];
			//temp[i] = exp(dt * u*u/(G* norm(functionVec[i][ringNumber]) + u)) * C[i].row(ringNumber) * functionVec[i];
		}

		//if (ringNumber == 0)
		//	showM(temp);

		fftw_execute(p);  //fourier transformation

		for (int i = 0; i <= N / 2; ++i) {
			k = (double)i * range2;
			inC[i] = exp(-I * dt * k * k) * toComplex(outC[i]);
		}
		for (int i = 0; i < N / 2; ++i) {
			k = (-PI) / dx + (double)(i)* range2;
			inC[N / 2 + i + 1] = exp(-I * dt * k * k) * toComplex(outC[N / 2 + i + 1]);
		}

		//if (ringNumber == 0)
		//	showM(inC);

		fftw_execute(pInverse); //backward fourier transfrormation
		
		//if(ringNumber==0)
		//	showM(out);

		processed = true;
		waitForNotMain();

		for (int i = 0; i < N; ++i) {
			functionVec[i][ringNumber] = toComplex(out[i]) / (double)N;
		}

		processed = false;
	}
}


/////////////////////////////////////////////////


int main()
{
	//g.metafl("CONS"); //wyświetlanie wykresów w konsoli
	g.metafl("PNG");

	//values for corelation function
	for (int i = 0; i <= N / 2; ++i) { //TODO:
		J1[i] = J0 / sqrt(PI) / w * exp(-pow((i * dx) + PI / 2, 2) / w / w);
		J2[i] = J0 / sqrt(PI) / w * exp(-pow((i * dx) - PI / 2, 2) / w / w);
	}
	for (int i = N / 2 + 1; i < N; ++i) {
		J1[i] = J0 / sqrt(PI) / w * exp(-pow(((i - N) * dx) + PI / 2, 2) / w / w);
		J2[i] = J0 / sqrt(PI) / w * exp(-pow(((i - N) * dx) - PI / 2, 2) / w / w);
	}

	//starting data for t=0;
	for (int i = 0; i <= N / 2; ++i) {
		for (int j = 0; j < numberOfRings; ++j) {
			if (j == 0)
				functionVec[i][j] = sqrt(u / G) * exp(i * dx * I * vortex) + sin(i * dx) / 100;
			else
				functionVec[i][j] = 0;
				//functionVec[i][j] = sqrt(u / G) * exp(i * dx * I * vortex) + sin(i * dx) / 100;
		}

	}
	for (int i = N / 2 + 1; i < N; ++i) {
		for (int j = 0; j < numberOfRings; ++j) {
			if (j == 0)
				functionVec[i][j] = sqrt(u / G) * exp((i - N) * dx * I * vortex) + sin((i - N) * dx) / 100;
			else
				functionVec[i][j] = 0;
				//functionVec[i][j] = sqrt(u / G) * exp((i - N) * dx * I * vortex) + sin((i - N) * dx) / 100;
		}
	}

	//g.disini();
	//showIm(functionVec);
	
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < numberOfRings; ++j) {
			if (j - 1 >= 0) {
				if (j % 2 == 0)
					C[i](j, j - 1) = J2[i];
				else
					C[i](j, j - 1) = J1[i];
			}
			if (j + 1 <numberOfRings) {
				if (j % 2 == 0)
					C[i](j, j + 1) = J1[i];
				else
					C[i](j, j + 1) = J2[i];
			}
		}

		C[i] *= -dt * I;
		C[i] = C[i].exp();
		C[i] = C[i].array().isNaN().select(0, C[i]).eval(); //removing NaN values from exp result
		if (C[i] == Eigen::MatrixXcd::Zero(numberOfRings, numberOfRings)) {
			for (int j = 0; j < numberOfRings; ++j) {
				C[i](j, j) = complex<double>(1, 0);
			}		
		}
	}

	for (int i = -N / 2; i <= N / 2; ++i) {
		x[i + N / 2] = dx * i;
	}

	//g.disini();
	//g.titlin("funkcja J(x) korelacji okregow", 3);
	//showR(J1);
	//showR(J2);

	complex<double> temp[numberOfRings][N];

	complex<double> temp1[N];
	complex<double> temp2[N];
	complex<double> temp3[N];

	complex<double> nextStep1[N]; //for temporary result from fft
	complex<double> nextStep2[N];
	complex<double> nextStep3[N];

	double k; //wave number


	complex<double> inC[numberOfRings][N];
	fftw_complex outC[numberOfRings][N];
	fftw_complex out[numberOfRings][N];

	fftw_plan p[numberOfRings];
	fftw_plan pInverse[numberOfRings];
	for (int i = 0; i < numberOfRings; ++i) {
		p[i]= fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(temp[i]), outC[i], FFTW_FORWARD, FFTW_MEASURE);
	}
	for (int i = 0; i < numberOfRings; ++i) {
		pInverse[i] = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(inC[i]), out[i], FFTW_BACKWARD, FFTW_MEASURE);
	}

	////ploting starting data
	//showM(functionVec);

	//topological charge
	float topCharge[numberOfRings][maxTime / resolution + 1];
	float xAxis[maxTime / resolution + 1];
	for (int i = 0; i <= maxTime / resolution; ++i) {
		xAxis[i] = i * (resolution * dt);
	}

	int range2 = 2 * PI / range;

//	assert(fftw_init_threads());

	Eigen::initParallel();

	thread *ring[numberOfRings];
	for (int i = 0; i < numberOfRings; ++i) {
		ring[i] =new thread(computeRing, i, temp[i], k, inC[i], outC[i], p[i], pInverse[i], out[i], range2, ref(processed[i]), topCharge[i]);
	}
	
	//////////////////////main loop//////////////
	while (t <= maxTime) {

		waitForProcessed();

		if (t % resolution == 0)
			++index;
		mainReady = false;
		waitForNotProcessed();

		++t;

		mainReady = true;

		////ploting results during code execution
		//if (t % 1 == 0 && t > 0) {
		//}
	}

	for (int i = 0; i < numberOfRings; ++i) {
		ring[i]->join();
		fftw_destroy_plan(p[i]);
		fftw_destroy_plan(pInverse[i]);
	}
	
  	//showM(functionVec);

	//ploting topological charge
	for (int i = 0; i < numberOfRings; ++i) {
		topCharge[i][maxTime / resolution - 1] = -6;
		topCharge[i][maxTime / resolution] = 6;

		//setting file name
		std::string fileName = "results/" +std::to_string(numberOfRings)+"rings/w-"+ std::to_string(w) + "_J0-" + std::to_string(J0.real()) + "_G-" + std::to_string(G) + "_u-" +
			std::to_string(u) + "_T-" + std::to_string(dt * t) + "_N-" + std::to_string(N) + "_vortex-" + std::to_string(vortex) + "_ring_" + std::to_string(i+1) + ".png";
		g.setfil(fileName.c_str());

		g.disini();
		//plot title
		std::string tmpStr = "topological charge ring " + std::to_string(i+1);

		g.titlin(tmpStr.c_str(), 3);

		g.qplot(xAxis, topCharge[i], maxTime / resolution +1);
	}

	//ploting topological charge difference with neighbers
	
	//for (int i = 0; i < numberOfRings; ++i) {
	//	////setting file name
	//	//std::string fileName = "results/" + std::to_string(numberOfRings) + "rings/sums_w-" + std::to_string(w) + "_J0-" + std::to_string(J0.real()) + "_G-" + std::to_string(G) + "_u-" +
	//	//	std::to_string(u) + "_T-" + std::to_string(dt * t) + "_N-" + std::to_string(N) + "_vortex-" + std::to_string(vortex) + "_ring_" + std::to_string(i + 1) + ".png";
	//	//g.setfil(fileName.c_str());
	//	//g.disini();
	//	////plot title
	//	//std::string tmpStr = "topological charge difference with neighbers on ring " + std::to_string(i + 1);
	//	//g.titlin(tmpStr.c_str(), 3);

	//	//float topChargeDiff[resolutionPoints + 1];
	//	//for (int j = 0; j < resolutionPoints + 1; ++j) {
	//	//	topChargeDiff[j] = topCharge[i][j];
	//	//	if (i > 0)
	//	//		topChargeDiff[j] -= topCharge[i - 1][j];
	//	//	if (i < numberOfRings - 1)
	//	//		topChargeDiff[j] -= topCharge[i + 1][j];
	//	//}
	//	////topChargeDifference
	//	//g.qplot(xAxis, topChargeDiff, resolutionPoints + 1);
	//}

	_getch();
	return 0;
}

