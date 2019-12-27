#include "discpp.h"
#include "fftw3.h"
#include "Matrix.h"
#include "Matrix.cpp"

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <conio.h>
#include <complex>
#include <math.h>
#include <mutex>
#include <condition_variable>
#include<thread>

using namespace std;

//define macros for fftw_complex
#define REAL 0
#define IMAG 1

const double PI = 3.14159265358979323846;
const int N = 255; //number of given points (must be odd)
const double dx = 2 * PI / (N - 1); //range between two points
const double range = dx * (N - 1); //total domain range
const int maxTime = 50000; //number of steps in simulation 
const double h = 1;  //h-bar
const double m = 1; //particle mass
const double dt =  2*0.5 * dx * dx / 2 / PI; //time step
const int c = 1;//speed of light
const complex<double> I(0, 1); //sqruare root of -1
const double G = 1; //nonlinearity coeficient
const double u = 3;

float x[N], y[N]; //points to plot

Dislin g; //plotting object
int t = 0; //actual time

//corelation between rings cooficients
const complex<double> J0 = 2.6;
complex<double> J1[N];
complex<double> J2[N];
double w = 0.1;


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

void showM(fftw_complex* a) {
	complex<double>* temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = abs(temp[i]);
	}
	g.qplot(x, y, N);
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




////////////////////multithreading///////////////////

mutex ring1Mutex;
mutex ring2Mutex;
mutex ring3Mutex;
bool mainReady = true;
bool processed1 = false, processed2 = false, processed3 = false;
condition_variable cv1;
condition_variable cv2;
condition_variable cv3;

int index = 0;
const int resolution=100; //for how many steps of simulation one calculation of topological charge


bool getProcessed1() {
	return processed1;
}
bool getProcessed2() {
	return processed2;
}
bool getProcessed3() {
	return processed3;
}
bool getMainReady() {
	return mainReady;
}


void computeRing(complex<double> *function, complex<double> *temp,const complex<double> *c1, 
	const complex<double> *c2,const complex<double> *c3, double k, complex<double>* inC, fftw_complex* outC, 
	fftw_plan p, fftw_plan pInverse, complex<double> *nextStep, fftw_complex *out,const int range2,
	mutex& mut,condition_variable &cv, bool &processed, float *topCharge) {
	
	//unique_lock<mutex> lk(mut);
	//lk.unlock();

	while (t < maxTime) {

		while (!mainReady) {}

		//lk.lock();
		//cv.wait(lk, [] {return mainReady; });

		//topological charge calculation
		if (t % resolution == 0) {
			topologicalCharge(function, &topCharge[index]);
		}

		//split-step calculation

		for (int i = 0; i < N; ++i) {
			temp[i] = exp(dt * (((-I - G) * norm(function[i])) + u)) *
				(c1[i] * function1[i] + c2[i] * function2[i] + c3[i] * function3[i]);
		}
		fftw_execute(p);  //fourier transformation

		for (int i = 0; i <= N / 2; ++i) {
			k = (double)i * range2;
			inC[i] = exp(-I * dt * k * k) * toComplex(outC[i]);
		}
		for (int i = 0; i < N / 2; ++i) {
			k = (-PI) / dx + (double)(i)* range2;
			inC[N / 2 + i + 1] = exp(-I * dt * k * k) * toComplex(outC[N / 2 + i + 1]);
		}

		fftw_execute(pInverse); //backward fourier transfrormation

		processed = true;
		//lk.unlock();
		//cv.notify_all();

		//lk.lock();
		//cv.wait(lk, [] {return !mainReady; });

		while (mainReady) {}

		for (int i = 0; i < N; ++i) {
			function[i] = toComplex(out[i]) / (double)N;
		}

		processed = false;
		//lk.unlock();
		//cv.notify_all();
	}
}



/////////////////////////////////////////////////

int main()
{
	g.metafl("CONS"); //wyświetlanie wykresów w konsoli
	//g.metafl("PNG");


	//values for corelation function
	for (int i = 0; i <= N / 2; ++i) {
		J1[i] = J0 / sqrt(PI) / w * exp(-pow((i * dx) + PI / 2, 2) / w / w);
		J2[i] = J0 / sqrt(PI) / w * exp(-pow((i * dx) - PI / 2, 2) / w / w);
		//J[i] = J0;
	}
	for (int i = N / 2 + 1; i < N; ++i) {
		J1[i] = J0 / sqrt(PI) / w * exp(-pow(((i - N) * dx) + PI / 2, 2) / w / w);
		J2[i] = J0 / sqrt(PI) / w * exp(-pow(((i - N) * dx) - PI / 2, 2) / w / w);
		//J1[i] = J0 / sqrt(PI) / w * exp(-((i - N) * dx) * ((i - N) * dx) / w / w);

		//J[i] = J0;
	}

	double x0 = 0.4;
	//starting data for t=0;
	for (int i = 0; i <= N / 2; ++i) {
		function1[i] = sqrt(u / G) * exp(i * dx * I) + sin(3 * i * dx) / 100;
		function2[i] = sqrt(u / G) * exp(i * dx * I) - sin(3 * i * dx) / 100;
		function3[i] = sqrt(u / G) * exp(i * dx * I) + sin(3 * i * dx) / 100;
	}
	for (int i = N / 2 + 1; i < N; ++i) {
		function1[i] = sqrt(u / G) * exp((i - N) * dx * I) + sin(3 * (i - N) * dx) / 100;
		function2[i] = sqrt(u / G) * exp((i - N) * dx * I) - sin(3 * (i - N) * dx) / 100;
		function3[i] = sqrt(u / G) * exp((i - N) * dx * I) + sin(3 * (i - N) * dx) / 100;
	}

	//copuling matrix
	complex<double> c1[N];
	complex<double> c2[N];
	complex<double> c3[N];
	complex<double> c4[N];
	complex<double> c5[N];
	complex<double> c6[N];
	complex<double> c7[N];
	complex<double> c8[N];
	complex<double> c9[N];

	for (int i = 0; i < N; ++i) {
		complex<double> sqr = sqrt(J1[i] * J1[i] + J2[i] * J2[i]);
		if (sqr != complex<double>(0, 0)) {
			c1[i] = (cos(sqr * dt) * pow(J1[i], 2) + pow(J2[i], 2)) / pow(sqr, 2);
			c2[i] = -I * sin(sqr * dt) * J1[i] / sqr;
			c3[i] = (cos(sqr * dt) * J1[i] * J2[i] - J1[i] * J2[i]) / pow(sqr, 2);

			c4[i] = -I * sin(sqr * dt) * J1[i] / sqr;
			c5[i] = cos(sqr * dt);
			c6[i] = -I * sin(sqr * dt) * J2[i] / sqr;

			c7[i] = (cos(sqr * dt) * J1[i] * J2[i] - J1[i] * J2[i]) / pow(sqr, 2);
			c8[i] = -I * sin(sqr * dt) * J2[i] / sqr;
			c9[i] = (cos(sqr * dt) * pow(J2[i], 2) + pow(J1[i], 2)) / pow(sqr, 2);
		}
		else {
			c1[i] = 0;
			c2[i] = 0;
			c3[i] = 0;

			c4[i] = 0;
			c5[i] = 0;
			c6[i] = 0;

			c7[i] = 0;
			c8[i] = 0;
			c9[i] = 0;
		}
	}

	complex<double> n1[N]; //n1=|function1|^2
	complex<double> n2[N]; //n2=|function2|^2

	for (int i = -N / 2; i <= N / 2; ++i) {
		x[i + N / 2] = dx * i;
	}

	g.disini();
	g.titlin("funkcja J(x) korelacji okregow", 3);
	showR(J1);
	showR(J2);

	complex<double> in1[N];
	complex<double> in2[N];
	complex<double> in3[N];
	complex<double> temp1[N];
	complex<double> temp2[N];
	complex<double> temp3[N];

	complex<double> nextStep1[N]; //for temporary result from fft
	complex<double> nextStep2[N];
	complex<double> nextStep3[N];

	double k; //wave number


	complex<double> inC1[N];
	complex<double> inC2[N];
	complex<double> inC3[N];
	fftw_complex outC1[N];
	fftw_complex outC2[N];
	fftw_complex outC3[N];
	fftw_complex out1[N];
	fftw_complex out2[N];
	fftw_complex out3[N];

	fftw_plan p1 = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(temp1), outC1, FFTW_FORWARD, FFTW_MEASURE);
	fftw_plan p2 = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(temp2), outC2, FFTW_FORWARD, FFTW_MEASURE);
	fftw_plan p3 = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(temp3), outC3, FFTW_FORWARD, FFTW_MEASURE);
	fftw_plan pInverse1 = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(inC1), out1, FFTW_BACKWARD, FFTW_MEASURE);
	fftw_plan pInverse2 = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(inC2), out2, FFTW_BACKWARD, FFTW_MEASURE);
	fftw_plan pInverse3 = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(inC3), out3, FFTW_BACKWARD, FFTW_MEASURE);

	//ploting starting data
//	showR(function);
	g.disini();
	g.titlin("modul funkcji poczatkowej na pierwszym okregu", 3);
	showM(function1);
	g.disini();
	g.titlin("modul funkcji poczatkowej na drugim okregu", 3);
	showM(function2);

	//topological charge
	float topCharge1[maxTime / resolution + 1];
	float topCharge2[maxTime / resolution + 1];
	float topCharge3[maxTime / resolution + 1];
	float xAxis[maxTime / resolution + 1];
	for (int i = 0; i <= maxTime / resolution; ++i) {
		xAxis[i] = i * (resolution * dt);
	}

	int range2 = 2 * PI / range;

	//thread testy(test, processed1, topCharge1);

	thread ring1(computeRing, function1, temp1, c1, c2, c3, k, inC1, outC1, p1, pInverse1, nextStep1, out1, range2,
		ref(ring1Mutex), ref(cv1), ref(processed1), topCharge1);
	thread ring2(computeRing, function2, temp2, c4, c5, c6, k, inC2, outC2, p2, pInverse2, nextStep2, out2, range2,
		ref(ring1Mutex), ref(cv2), ref(processed2), topCharge2);
	thread ring3(computeRing, function3, temp3, c7, c8, c9, k, inC3, outC3, p3, pInverse3, nextStep3, out3, range2,
		ref(ring1Mutex), ref(cv3), ref(processed3), topCharge3);

	//unique_lock<mutex> lk1(ring1Mutex);
	//unique_lock<mutex> lk2(ring2Mutex);
	//unique_lock<mutex> lk3(ring3Mutex);

	//lk1.unlock();
	//lk2.unlock();
	//lk3.unlock();

	//////////////////////main loop//////////////
	while (t <= maxTime) {

		//lk1.lock();
		//lk2.lock();
		//lk3.lock();

		//cv1.wait(lk1, [] {return processed1; });//&& processed2 && processed3; });
		//cv2.wait(lk1, [] {return processed2; });
		//cv3.wait(lk1, [] {return processed3; });

		while (!(processed1 && processed2 && processed3)) {}

		if (t % resolution == 0)
			++index;
		mainReady = false;
		//lk1.unlock();
		//lk2.unlock();
		//lk3.unlock();
		//cv1.notify_all();
		//cv2.notify_all();
		//cv3.notify_all();

		//lk1.lock();
		//lk2.lock();
		//lk3.lock();
		//cv1.wait(lk1, [] {return !processed1; });
		//cv2.wait(lk1, [] {return !processed2; });
		//cv3.wait(lk1, [] {return !processed3; });

		while (processed1 || processed2 || processed3) {}

		++t;

		mainReady = true;
		//lk1.unlock();
		//lk2.unlock();
		//lk3.unlock();
		//cv1.notify_all();
		//cv2.notify_all();
		//cv3.notify_all();

		////ploting results during code execution
		//if (t % 2 == 0 && t > 0) {
		//	g.disini();
		//	std::string tmpStr = "modul funkcji na pierwszym okregu po " + std::to_string(t) + " krokach przy dt= " + std::to_string(dt);
		//	g.titlin(tmpStr.c_str(), 3);
		//	showM(function1);
		//	g.disini();
		//	tmpStr = "modul funkcji na drugim okregu po " + std::to_string(t) + " krokach przy dt= " + std::to_string(dt);
		//	g.titlin(tmpStr.c_str(), 3);
		//	showM(function2);
		//	g.disini();
		//	tmpStr = "modul funkcji na trzecim okregu po " + std::to_string(t) + " krokach przy dt= " + std::to_string(dt);
		//	g.titlin(tmpStr.c_str(), 3);
		//	showM(function3);
		//}
	}

	ring1.join();
	ring2.join();
	ring3.join();

	fftw_destroy_plan(p1);
	fftw_destroy_plan(p2);
	fftw_destroy_plan(p3);
	fftw_destroy_plan(pInverse1);
	fftw_destroy_plan(pInverse2);
	fftw_destroy_plan(pInverse3);

	//plotting resoults
		//showR(function);

	g.disini();
	std::string tmpStr = "modul funkcji na pierwszym okregu po przepropagowaniu o " + std::to_string(t) + " krokow przy dt= " + std::to_string(dt) + ", T=" + std::to_string(dt * t);
	g.titlin(tmpStr.c_str(), 3);
	showM(function1);
	g.disini();
	tmpStr = "modul funkcji na drugim okregu po przepropagowaniu o " + std::to_string(t) + " krokow przy dt= " + std::to_string(dt) + ", T=" + std::to_string(dt * t);
	g.titlin(tmpStr.c_str(), 3);
	showM(function2);
	g.disini();
	tmpStr = "modul funkcji na trzecim okregu po przepropagowaniu o " + std::to_string(t) + " krokow przy dt= " + std::to_string(dt) + ", T=" + std::to_string(dt*t);
	g.titlin(tmpStr.c_str(), 3);
	showM(function3);


	topCharge1[maxTime / resolution - 1] = -4;
	topCharge2[maxTime / resolution - 1] = -4;
	topCharge3[maxTime / resolution - 1] = -4;

	topCharge1[maxTime / resolution] = 4;
	topCharge2[maxTime / resolution] = 4;
	topCharge3[maxTime / resolution] = 4;

	//ploting topological charge
	g.disini();
	tmpStr = "topological charge ring 1";
	g.titlin(tmpStr.c_str(), 3);
	g.qplot(xAxis, topCharge1, maxTime / resolution + 1);

	g.disini();
	tmpStr = "topological charge ring 2";
	g.titlin(tmpStr.c_str(), 3);
	g.qplot(xAxis, topCharge2, maxTime / resolution + 1);

	g.disini();
	tmpStr = "topological charge ring 3";
	g.titlin(tmpStr.c_str(), 3);
	g.qplot(xAxis, topCharge3, maxTime / resolution + 1);

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

