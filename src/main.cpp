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

#include<thread>

using namespace std;

//define macros for fftw_complex
#define REAL 0
#define IMAG 1

const double PI = 3.14159265358979323846;
const int N = 255; //number of given points (must be odd)
const double dx = 2 * PI / (N - 1); //range between two points
const double range = dx * (N - 1); //total domain range
const int maxTime = 100000; //number of steps in simulation 
const double h = 1;  //h-bar
const double m = 1; //particle mass
const double dt = 2 * 0.5 * dx * dx / 2 / PI; //time step
const int c = 1;//speed of light
const complex<double> I(0, 1); //sqruare root of -1
const double G = 1; //nonlinearity coeficient
const double u = 3;

float x[N], y[N]; //points to plot

Dislin g; //plotting object

//corelation between rings cooficients
const complex<double> J0 = 1;
complex<double> J[N];
double w = 1;


complex<double> function1[N]; //wave function on first ring
complex<double> function2[N]; //wave function on second ring
complex<double> function3[N]; //wave function on second ring

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

int topologicalCharge(complex<double>* tab) {
	tab = fixOrder(tab);
	double argument = arg(tab[0]);
	int result = 0;
	double temp;
	for (int i = 0; i < N; ++i) {
		temp = argument;
		argument = arg(tab[i]);
		if (argument > temp + 2)
			++result;
		if (temp > argument + 2)
			--result;
	}
	return result;
}

//void firstRing()


int main()
{

	g.metafl("CONS"); //wyświetlanie wykresów w konsoli
	//g.metafl("PNG");


	//values for corelation function
	for (int i = 0; i <= N / 2; ++i) {
		J[i] = J0 / sqrt(PI) / w * exp(-(i * dx) * (i * dx) / w / w);
		//J[i] = J0;
	}
	for (int i = N / 2 + 1; i < N; ++i) {
		J[i] = J0 / sqrt(PI) / w * exp(-((i - N) * dx) * ((i - N) * dx) / w / w);
		//J[i] = J0;
	}

	double x0 = 0.4;
	//starting data for t=0;
	for (int i = 0; i <= N / 2; ++i) {
		//function1[i] = 1 / cosh(-i * dx / x0);//*exp(I*(complex<double>)(i*dx / x0));
		//function2[i] =  1/(i * dx / x0)* sin(i * dx / x0);
		//if (i == 0) function2[i] = 1;
		//function1[i] = 1 / (i * dx / x0) * sin(i * dx / x0);
		//if (i == 0) function1[i] = 1;

		function1[i] = sqrt(u / G) + sin(i * dx) / 300;
		function2[i] = sqrt(u / G) + sin(i * dx) / 300;
		function3[i] = sqrt(u / G) + sin(i * dx) / 300;
	}
	for (int i = N / 2 + 1; i < N; ++i) {
		//function1[i] = 1 / cosh(-(i - N) * dx / x0); //*exp(I*(complex<double>)((i - N)*dx / x0));
		//function2[i] = 1 / ((i - N) * dx / x0) * sin((i - N) * dx / x0);
		//function1[i] = 1 / ((i - N) * dx / x0) * sin((i - N) * dx / x0);

		//function1[i] = J[i] * function2[i];
		function1[i] = sqrt(u / G) + sin((i - N) * dx) / 300;
		function2[i] = sqrt(u / G) + sin((i - N) * dx) / 300;
		function3[i] = sqrt(u / G) + sin((i - N) * dx) / 300;
	}



	complex<double> n1[N]; //n1=|function1|^2
	complex<double> n2[N]; //n2=|function2|^2

	for (int i = -N / 2; i <= N / 2; ++i) {
		x[i + N / 2] = dx * i;
	}

	g.disini();
	g.titlin("funkcja J(x) korelacji okregow", 3);
	showR(J);

	complex<double> in[N];
	complex<double> temp[N];

	int time = 0; //actual time
	double k; //wave number


	double inR[N], outR[N];
	complex<double> inC[N];
	fftw_complex outC[N];
	fftw_complex out[N];

	fftw_plan p = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(temp), outC, FFTW_FORWARD, FFTW_MEASURE);
	fftw_plan pInverse = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(inC), out, FFTW_BACKWARD, FFTW_MEASURE);

	//ploting starting data
//	showR(function);
	g.disini();
	g.titlin("modul funkcji poczatkowej na pierwszym okregu", 3);
	showM(function1);
	g.disini();
	g.titlin("modul funkcji poczatkowej na drugim okregu", 3);
	showM(function2);

	//topological charge
	int index = 0;
	const int resolution = 10000;
	float topCharge1[maxTime / resolution + 1];
	float topCharge2[maxTime / resolution + 1];
	float xAxis[maxTime / resolution + 1];
	for (int i = 0; i <= maxTime / resolution; ++i) {
		xAxis[i] = i * (resolution * dt);
	}

	//////////////////////main loop//////////////
	while (time <= maxTime) {

		if (time % resolution == 0) {
			topCharge1[index] = topologicalCharge(function1);
			topCharge2[index++] = topologicalCharge(function2);
		}


		//for (int i = 0; i < N; ++i) {
		//	n1[i] = (-I - G) * norm(function1[i]); //n * (-I)
		//	n2[i] = (-I - G) * norm(function2[i]); //n * (-I)
		//}

		//////////////////

		///first ring///

		for (int i = 0; i < N; ++i) {
			temp[i] = exp(dt * (((-I - G) * norm(function1[i])) + u)) * (cos(J[i] * dt) * function1[i] - I * sin(J[i] * dt) * function2[i]);// *function1[i];
			//temp[i] = exp((-I) * dt * (n1[i] + I * u)) * (cosh(-I*J[i] * dt) * function1[i] + sinh(-I*J[i] * dt) * function2[i]);
		}
		fftw_execute(p);  //fourier transformation

		int range2 = 2 * PI / range;
		for (int i = 0; i <= N / 2; ++i) {
			k = (double)i * range2;
			inC[i] = exp(-I * dt * k * k) * toComplex(outC[i]);
		}
		for (int i = 0; i < N / 2; ++i) {
			k = (-PI) / dx + (double)(i)* range2;
			inC[N / 2 + i + 1] = exp(-I * dt * k * k) * toComplex(outC[N / 2 + i + 1]);
		}

		fftw_execute(pInverse); //backward fourier transfrormation

		///second ring////

		for (int i = 0; i < N; ++i) {
			temp[i] = exp(dt * (((-I - G) * norm(function2[i])) + u)) * (cos(J[i] * dt) * function2[i] - I * sin(J[i] * dt) * function1[i]);
			//temp[i] = exp((-I) * dt * (n1[i] + I * u)) * (cosh(-I * J[i] * dt) * function2[i] + sinh(-I * J[i] * dt) * function1[i]);
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

		//complex<double> nextStep1[N];
		//complex<double> nextStep2[N];

		for (int i = 0; i < N; ++i) {
			//nextStep1[i] = toComplex(out[i]) / (double)N; //normalizing and saving output for first ring without corelation with second one
			function1[i] = toComplex(out[i]) / (double)N;
		}

		fftw_execute(pInverse); //backward fourier transfrormation

		//for (int i = 0; i < N; ++i) {
		//	//nextStep1[i] += function2[i] * J[i]*dt; //second ring impact
		//	nextStep1[i] *= (cos(J[i]*dt)*function1[i]-I*sin(J[i]*dt)*function2[i]); //second ring impact
		//}

		for (int i = 0; i < N; ++i) {
			//nextstep2[i] = toComplex(out[i]) / (double)N;
			function2[i] = toComplex(out[i]) / (double)N;
		}

		//for (int i = 0; i < N; ++i) {
		//	//function2[i] += function1[i] * J[i]*dt; //normalizing and saving output
		//	nextStep2[i] *= (cos(J[i]*dt) * function2[i] - I * sin(J[i]*dt) * function1[i]);
		//}

		//for (int i = 0; i < N; ++i) {
		//	function1[i] = nextStep1[i];
		//	function2[i] = nextStep2[i];
		//}

		++time;


		////ploting results during code execution
		//if (time % 3000000 == 0 && time > 0) {
		//	g.disini();
		//	std::string tmpStr = "modul funkcji na pierwszym okregu po " + std::to_string(time) + " krokach przy dt= " + std::to_string(dt);
		//	g.titlin(tmpStr.c_str(), 3);
		//	showM(function1);
		//	g.disini();
		//	tmpStr = "modul funkcji na drugim okregu po " + std::to_string(time) + " krokach przy dt= " + std::to_string(dt);
		//	g.titlin(tmpStr.c_str(), 3);
		//	showM(function2);
		//}

	}

	fftw_destroy_plan(p);
	fftw_destroy_plan(pInverse);

	//plotting resoults
		//showR(function);

	g.disini();
	std::string tmpStr = "modul funkcji na pierwszym okregu po przepropagowaniu o " + std::to_string(time) + " krokow przy dt= " + std::to_string(dt);
	g.titlin(tmpStr.c_str(), 3);
	showM(function1);
	g.disini();
	tmpStr = "modul funkcji na drugim okregu po przepropagowaniu o " + std::to_string(time) + " krokow przy dt= " + std::to_string(dt);
	g.titlin(tmpStr.c_str(), 3);
	showM(function2);

	//ploting topological charge
	g.disini();
	tmpStr = "topological charge ring 1";
	g.titlin(tmpStr.c_str(), 3);
	g.qplot(xAxis, topCharge1, maxTime / resolution + 1);

	g.disini();
	tmpStr = "topological charge ring 2";
	g.titlin(tmpStr.c_str(), 3);
	g.qplot(xAxis, topCharge2, maxTime / resolution + 1);


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

