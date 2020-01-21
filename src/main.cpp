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

const double PI = 3.14159265358979323846;
const int N = 255; //number of given points (must be odd)
const double dx = 2 * PI / (N - 1); //range between two points
const double range = dx * (N - 1); //total domain range
const int maxTime = 500000; //number of steps in simulation 
const double h = 1;  //h-bar
const double m = 1; //particle mass
const double dt =  2* 0.5 * dx * dx / 2 / PI; //time step
const int c = 1;//speed of light
const complex<double> I(0, 1); //sqruare root of -1
const double G = 1; //nonlinearity coeficient
const double u = 3;

float x[N], y[N]; //points to plot

Dislin g; //plotting object
int t = 0; //actual time

const unsigned numberOfRings = 3; //nuber of rings

//corelation between rings cooficients
complex<double> J[N][numberOfRings];

const complex<double> J0 = 1.2;
complex<double> J1[N];
complex<double> J2[N];
double w = 0.1;

const int resolution = 100; //for how many steps of simulation one calculation of topological charge
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

	while (t < maxTime) {

		waitForMain();

		//topological charge calculation
		if (t % resolution == 0) {
			topologicalCharge(functionVec, &topCharge[index], ringNumber);
		}

		//split-step calculation

		for (int i = 0; i < N; ++i) {
			//temp[i] = exp(dt * (((-I - G) * norm(function[i])) + u)) *
			//	(c1[i] * function1[i] + c2[i] * function2[i] + c3[i] * function3[i]);
			temp[i] = exp(dt * (((-I - G) * norm(functionVec[i][ringNumber])) + u)) * C[i].row(ringNumber) * functionVec[i];
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
		//std::cout << ringNumber<< " ";

		processed = false;
	}
}


/////////////////////////////////////////////////



int main()
{
	g.metafl("CONS"); //wyświetlanie wykresów w konsoli
	//g.metafl("PNG");

	//values for corelation function
	for (int i = 0; i <= N / 2; ++i) { //TODO:
		J1[i] = J0 / sqrt(PI) / w * exp(-pow((i * dx) + PI / 2, 2) / w / w);
		J2[i] = J0 / sqrt(PI) / w * exp(-pow((i * dx) - PI / 2, 2) / w / w);

		//J1[i] = J0 / sqrt(PI) / w * exp(-(i * dx) * (i * dx) / w / w); //two rings
		//J[i] = J0;
	}
	for (int i = N / 2 + 1; i < N; ++i) {
		J1[i] = J0 / sqrt(PI) / w * exp(-pow(((i - N) * dx) + PI / 2, 2) / w / w);
		J2[i] = J0 / sqrt(PI) / w * exp(-pow(((i - N) * dx) - PI / 2, 2) / w / w);

		//J1[i] = J0 / sqrt(PI) / w * exp(-((i - N) * dx) * ((i - N) * dx) / w / w); // two rings

		//J[i] = J0;
	}
	double x0 = 0.4;
	//starting data for t=0;
	for (int i = 0; i <= N / 2; ++i) {
		for (int j = 0; j < numberOfRings; ++j) {
			functionVec[i][j] = sqrt(u / G) * exp(i * dx * I) + sin(3 * i * dx) / 100;
		}

	}
	for (int i = N / 2 + 1; i < N; ++i) {
		for (int j = 0; j < numberOfRings; ++j) {
			functionVec[i][j] = sqrt(u / G) * exp((i - N) * dx * I) + sin(3 * (i - N) * dx) / 100;
		}
	}
	
	////////////////////////

		////if(c1[i]!= C[i](0, 0))
		//	cout << i << " " << c1[i] << " " << C[i](0, 0) << endl;
		////if (c2[i] != C[i](0, 1))
		//	cout << i << " " << c2[i] << " " << C[i](0, 1) << endl;
		////if (c3[i] != C[i](0, 2))
		//	cout << i << " " << c3[i] << " " << C[i](0, 2) << endl;
		////if (c4[i] != C[i](1, 0))
		//	cout << i << " " << c4[i] << " " << C[i](1, 0) << endl;
		////if (c5[i] != C[i](1, 1))
		//	cout << i << " " << c5[i] << " " << C[i](1, 1) << endl;
		////if (c6[i] != C[i](1, 2))
		//	cout << i << " " << c6[i] << " " << C[i](1, 2) << endl;
		////if (c7[i] != C[i](2, 0))
		//	cout << i << " " << c7[i] << " " << C[i](2, 0) << endl;
		////if (c8[i] != C[i](2, 1))
		//	cout << i << " " << c8[i] << " " << C[i](2, 1) << endl;
		////if (c9[i] != C[i](2, 2))
		//	cout << i << " " << c9[i] << " " << C[i](2, 2) << endl;

	for (int i = 0; i < N; ++i) {
		//C[i] << 0, J1[i], 0,
		//		J1[i], 0, J2[i],
		//		0, J2[i], 0;
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

		//C[i] << 0,    J1[i],   0,      0,
		//		J1[i],  0,    J2[i],   0,
		//		0,    J2[i],   0,    J1[i],
		//		0,      0,    J1[i],   0;

		//C[i] << 0, J1[i],
		//		J1[i], 0;

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

	g.disini();
	g.titlin("funkcja J(x) korelacji okregow", 3);
	showR(J1);
	showR(J2);

	//complex<double> in1[N];
	//complex<double> in2[N];
	//complex<double> in3[N];
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

	complex<double> inC1[N];
	complex<double> inC2[N];
	complex<double> inC3[N];
	fftw_complex outC1[N];
	fftw_complex outC2[N];
	fftw_complex outC3[N];
	fftw_complex out1[N];
	fftw_complex out2[N];
	fftw_complex out3[N];

//	fftw_plan_with_nthreads(4);

	fftw_plan p1 = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(temp1), outC1, FFTW_FORWARD, FFTW_MEASURE);
	fftw_plan p2 = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(temp2), outC2, FFTW_FORWARD, FFTW_MEASURE);
	fftw_plan p3 = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(temp3), outC3, FFTW_FORWARD, FFTW_MEASURE);
	fftw_plan pInverse1 = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(inC1), out1, FFTW_BACKWARD, FFTW_MEASURE);
	fftw_plan pInverse2 = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(inC2), out2, FFTW_BACKWARD, FFTW_MEASURE);
	fftw_plan pInverse3 = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(inC3), out3, FFTW_BACKWARD, FFTW_MEASURE);

	//ploting starting data
	showM(functionVec);

	//topological charge
	float topCharge[numberOfRings][maxTime / resolution + 1];

	float topCharge1[maxTime / resolution + 1];
	float topCharge2[maxTime / resolution + 1];
	float topCharge3[maxTime / resolution + 1];
	float xAxis[maxTime / resolution + 1];
	for (int i = 0; i <= maxTime / resolution; ++i) {
		xAxis[i] = i * (resolution * dt);
	}

	int range2 = 2 * PI / range;

//	assert(fftw_init_threads());

	Eigen::initParallel();
	//thread ring1(computeRing, 0, temp1, k, inC1, outC1, p1, pInverse1, out1, range2, ref(processed1), topCharge1);
	//thread ring2(computeRing, 1, temp2, k, inC2, outC2, p2, pInverse2, out2, range2, ref(processed2), topCharge2);
	//thread ring3(computeRing, 2, temp3, k, inC3, outC3, p3, pInverse3, out3, range2, ref(processed3), topCharge3);

	thread *ring[numberOfRings];
	for (int i = 0; i < numberOfRings; ++i) {
		//thread nextRing(computeRing, i, temp[i], k, inC[i], outC[i], p[i], pInverse[i], out[i], range2, ref(processed[i]), topCharge[i]);
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
		//	//g.disini();
		//	//std::string tmpStr = "modul funkcji na pierwszym okregu po " + std::to_string(t) + " krokach przy dt= " + std::to_string(dt);
		//	//g.titlin(tmpStr.c_str(), 3);
		//	//showM(function1);
		//	//g.disini();
		//	//tmpStr = "modul funkcji na drugim okregu po " + std::to_string(t) + " krokach przy dt= " + std::to_string(dt);
		//	//g.titlin(tmpStr.c_str(), 3);
		//	//showM(function2);
		//	//g.disini();
		//	//tmpStr = "modul funkcji na trzecim okregu po " + std::to_string(t) + " krokach przy dt= " + std::to_string(dt);
		//	//g.titlin(tmpStr.c_str(), 3);
		//	//showM(function3);
		//	showM(functionVec);
		//}
	}

	for (int i = 0; i < numberOfRings; ++i) {
		ring[i]->join();
		fftw_destroy_plan(p[i]);
		fftw_destroy_plan(pInverse[i]);
	}

	//fftw_destroy_plan(p1);
	//fftw_destroy_plan(p2);
	//fftw_destroy_plan(p3);
	//fftw_destroy_plan(pInverse1);
	//fftw_destroy_plan(pInverse2);
 //	fftw_destroy_plan(pInverse3);

	//plotting resoults
		//showR(function);

	//g.disini();
	std::string tmpStr = "modul funkcji na pierwszym okregu po przepropagowaniu o " + std::to_string(t) + " krokow przy dt= " + std::to_string(dt) + ", T=" + std::to_string(dt * t);
	//g.titlin(tmpStr.c_str(), 3);
	//showM(function1);
	//g.disini();
	//tmpStr = "modul funkcji na drugim okregu po przepropagowaniu o " + std::to_string(t) + " krokow przy dt= " + std::to_string(dt) + ", T=" + std::to_string(dt * t);
	//g.titlin(tmpStr.c_str(), 3);
	//showM(function2);
	//g.disini();
	//tmpStr = "modul funkcji na trzecim okregu po przepropagowaniu o " + std::to_string(t) + " krokow przy dt= " + std::to_string(dt) + ", T=" + std::to_string(dt*t);
	//g.titlin(tmpStr.c_str(), 3);
	//showM(function3);

  	showM(functionVec);

	//ploting topological charge
	for (int i = 0; i < numberOfRings; ++i) {
		topCharge[i][maxTime / resolution - 1] = -4;
		topCharge[i][maxTime / resolution] = 4;
		g.disini();
		tmpStr = "topological charge ring" + std::to_string(i+1);
		g.titlin(tmpStr.c_str(), 3);
		g.qplot(xAxis, topCharge[i], maxTime / resolution + 1);
	}

	//topCharge1[maxTime / resolution - 1] = -4;
	//topCharge2[maxTime / resolution - 1] = -4;
	//topCharge3[maxTime / resolution - 1] = -4;

	//topCharge1[maxTime / resolution] = 4;
	//topCharge2[maxTime / resolution] = 4;
	//topCharge3[maxTime / resolution] = 4;




	//////compering with theoretical solution///////
	complex<double> test[N];
	double tCzas = dt * (maxTime);

	//for (int i = 0; i <= N / 2; ++i) {
	//	test[i] = 1 / cosh(tCzas - i * dx / x0) * exp(I*(complex<double>)(i*dx / x0));
	//}
	//for (int i = N / 2 + 1; i < N; ++i) {
	//	test[i] = 1 / cosh(tCzas - (i - N)*dx / x0) * exp(I*(complex<double>)((i - N)*dx / x0));
	//}
	double N0 = 1;
	for (int i = 0; i <= -1 / 2; ++i) {
		test[i] = m / 2.0 / G * (pow(3 * N0 * G / (m), 2.0 / 3.0) - pow((i * dx / x0), 2.0));
	}
	for (int i = N / 2 + 1; i < N; ++i) {
		test[i] = m / 2.0 / G * (pow(3 * N0 * G / (m), 2.0 / 3.0) - pow(((i - N) * dx / x0), 2.0));
	}

	//showR(test);
	//showM(test);

	complex<double> roznica[N];
	for (int i = 0; i < N; ++i) {
		roznica[i] = function1[i] - test[i];
	}
	//showR(roznica);
	g.disini();
	g.titlin("roznica miedzy numerycznymi, a teoretycznymi wartosciami", 3);
	//showM(roznica);
	_getch();
	return 0;
}

