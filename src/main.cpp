#include "discpp.h"
#include "fftw3.h"

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <conio.h>
#include <complex>
#include <math.h>


using namespace std;

//define macros for fftw_complex
#define REAL 0
#define IMAG 1

const double PI = 3.14159265358979323846;
const int N = 1001; //number of given points (must be odd)
const double dx = 0.002*PI; //range between two points
const double range = dx * (N-1); //total domain range
const int maxTime = 10000; //number of steps in simulation 
const double h = 1;  //h-bar
const double m = 1; //particle mass
const double G = 3; //nonlinearity coeficient
const double dt = 0.01; //time step
const int c = 1;//speed of light
const complex<double> I(0, 1); //sqruare root of -1

float x[N], y[N]; //points to plot

Dislin g; //plotting object

//corelation between rings cooficients
const complex<double> J0 = 2;
complex<double> J[N];
const double u = 1;

complex<double> function1[N]; //wave function on first ring
complex<double> function2[N]; //wave function on second ring

//converting fftw_complex to  comlex<double>
complex<double> toComplex(fftw_complex a) {
	return complex<double>(a[REAL], a[IMAG]);
}

complex<double>* fixOrder(fftw_complex *a) {
	complex<double> *b;
	b = new complex<double>[N];
	for (int i = 0; i < N / 2; ++i) {
		b[i] = toComplex(a[N/2+i+1]);
	}
	for (int i = 0; i <= N / 2; ++i) {
		b[N/2+i] = toComplex(a[i]);
	}
	return b;
}

complex<double>* fixOrder(complex<double> *a) {
	complex<double> *b;
	b = new complex<double>[N];
	for (int i = 0; i < N / 2; ++i) {
		b[i] = a[N / 2 + i + 1];
	}
	for (int i = 0; i <= N / 2; ++i) {
		b[N / 2 + i] = a[i];
	}
	return b;
}

double* fixOrder(double *a) {
	double *b;
	b = new double[N];
	for (int i = 0; i < N / 2; ++i) {
		b[i] =  a[N / 2 + i + 1];
	}
	for (int i = 0; i <= N / 2; ++i) {
		b[N / 2 + i] = a[i];
	}
	return b;
}

//ploting function with lines
void show(double *a) {
	double *temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = temp[i];
	}
	g.qplot(x,y,N);
}

//ploting  function with points
void showP(double *a) { 
	double *temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = temp[i];
	}
	g.qplsca(x, y, N);
}

void showM(fftw_complex *a) {
	complex<double> *temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = norm(temp[i]);
	}
	g.qplot(x, y, N);
}

void showR(fftw_complex *a) {
	complex<double> *temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] =temp[i].real();
	}
	g.qplot(x, y, N);
}

void showR(complex <double> *a) {
	complex<double> *temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = temp[i].real();
	}
	g.qplot(x, y, N);
}

void showIm(fftw_complex *a) {
	complex<double> *temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = temp[i].imag();
	}
	g.qplot(x, y, N);
}

void showM(complex<double> *a) {
	complex<double> *temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = norm(temp[i]);
	}
	g.qplot(x, y, N);
}

void showIm(complex<double> *a) {
	complex<double> *temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = temp[i].imag();
	}
	g.qplot(x, y, N);
}


int main()
{
	g.metafl("CONS"); //wyświetlanie wykresów w konsoli
	//g.metafl("PNG");


	double w = 0.1;
	//values for corelation function
	for (int i = 0; i <= N / 2; ++i) {
		J[i] = 1 / sqrt(PI) / w * exp(-(i * dx) * (i * dx) / w / w);
		//J[i] = 2;
	}
	for (int i = N / 2 + 1; i < N; ++i) {
		J[i] = 1 / sqrt(PI) / w * exp(-((i - N) * dx) * ((i - N) * dx) / w / w);
		//J[i] = 2;
	}

	double x0 = 0.4;
	//starting data for t=0;
	for (int i = 0; i <= N/2; ++i) {
		//function1[i] = 1 / cosh(-i * dx/ x0) *exp(I*(complex<double>)(i*dx / x0));
		function2[i] =  1/(i * dx / x0)* sin(i * dx / x0);
		if (i == 0) function2[i] = 1;
		function1[i] = 1 / (i * dx / x0) * sin(i * dx / x0);
		if (i == 0) function1[i] = 1;
		//function1[i] = J[i]*function2[i];
	}
	for (int i = N/2+1; i < N; ++i) {
		//function1[i] = 1 / cosh(-(i - N)*dx / x0) *exp(I*(complex<double>)((i - N)*dx / x0));
		function2[i] =  1/((i - N) * dx / x0)* sin((i - N) * dx / x0);
		function1[i] = 1 / ((i - N) * dx / x0) * sin((i - N) * dx / x0);
		//function1[i] = J[i] * function2[i];
	}

	
	
	complex<double> n1[N]; //n1=|function1|^2
	complex<double> n2[N]; //n2=|function2|^2

	for (int i = -N/2; i <= N/2; ++i) {
		x[i+N/2] = dx*i;
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

	//////////////////////main loop//////////////
	while(time <= maxTime) {
		for (int i = 0; i < N; ++i) {
			n1[i] = G * norm(function1[i]);	
			n2[i] = G * norm(function2[i]);
		}

		//////////////////

		///first ring///

		for (int i = 0; i < N; ++i) {
			temp[i] = (exp((-I)/h*dt*(n1[i]+J[i]*function2[i]/function1[i]+I*u))*function1[i]);
		}

		fftw_execute(p);  //fourier transformation

		for (int i = 0; i <= N / 2; ++i) {
			k = (double)i / range*2*PI;
			inC[i] = exp(-I *dt*k*k/2.0)*toComplex(outC[i]);
		}
		for (int i = 0; i < N / 2; ++i) {
			//k = 2*PI*i/(c*N*dt);
			k = (-1.0 / (2 *dx) + (double)(i) / range) * 2 * PI;
			inC[N / 2 + i+1] = exp(-I *dt*k*k/2.0)*toComplex(outC[N/2+i+1]);
		}

		fftw_execute(pInverse); //backward fourier transfrormation

		///second ring////

		for (int i = 0; i < N; ++i) {
			temp[i] = (exp((-I) / h * dt * (n2[i] + J[i] * function1[i] / function2[i]+I*u)) * function2[i]);
		}

		fftw_execute(p);  //fourier transformation

		for (int i = 0; i <= N / 2; ++i) {
			k = (double)i / range * 2 * PI;
			inC[i] = exp(-I * dt * k * k / 2.0) * toComplex(outC[i]);
		}
		for (int i = 0; i < N / 2; ++i) {
			k = (-1.0 / (2 * dx) + (double)(i) / range) * 2 * PI;
			inC[N / 2 + i + 1] = exp(-I * dt * k * k / 2.0) * toComplex(outC[N / 2 + i + 1]);
		}

		for (int i = 0; i < N; ++i) {
			function1[i] = toComplex(out[i]) / (double)N; //normalizing and saving output
		}

		fftw_execute(pInverse); //backward fourier transfrormation

		for (int i = 0; i < N; ++i) {
			function2[i] = toComplex(out[i]) / (double)N; //normalizing and saving output
		}


		++time;

		
		//ploting results during code execution
		if (time % 10 == 0 && time > 000) {
			g.disini();
			std::string tmpStr = "modul funkcji na pierwszym okregu po " + std::to_string(time) + " krokach przy dt= " + std::to_string(dt);
			g.titlin(tmpStr.c_str(), 3);
			showM(function1);
			g.disini();
			tmpStr = "modul funkcji na drugim okregu po " + std::to_string(time) + " krokach przy dt= " + std::to_string(dt);
			g.titlin(tmpStr.c_str(), 3);
			showM(function2);
		}
	
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
		test[i] = m/2.0/G*(pow(3*N0*G/(m),2.0/3.0)- pow((i * dx / x0),2.0));
	}
	for (int i = N / 2 + 1; i < N; ++i) {
		test[i] = m  / 2.0 /G* (pow(3 * N0*G / (m ), 2.0 / 3.0) - pow(((i - N) * dx / x0), 2.0));
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
