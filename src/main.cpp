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
const double dx = 0.005*PI; //range between two points
const double range = dx * (N-1); //total domain range
const int maxTime = 1000; //number of steps in simulation 
const double h = 1;  //h-bar
const double m = 1; //particle mass
const double G = 10; //nonlinearity coeficient
const double dt = 0.01; //time step
const int c = 1;//speed of light
const complex<double> I(0, 1); //sqruare root of -1
const double w = 2; //harmonic oscillator potencial frequency

float x[N], y[N]; //points to plot

Dislin g; //plotting object

complex<double> function[N];
double value0[N]; //value of [function] at starting time and position 

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

	double x0 = 1;
	//starting data for t=0;
	for (int i = 0; i <= N/2; ++i) {
		function[i] = 1 / cosh(-i * dx/ x0) *exp(I*(complex<double>)(i*dx / x0));
	}
	for (int i = N/2+1; i < N; ++i) {
		function[i] = 1 / cosh(-(i - N)*dx / x0) *exp(I*(complex<double>)((i - N)*dx / x0));
	}



	complex<double> n[N]; //N=|f|^2

	for (int i = -N/2; i <= N/2; ++i) {
		x[i+N/2] = dx*i;
	}

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

//	showR(function);
	showM(function);

	//////////////////////main loop//////////////
	while(time <= maxTime) {
		//for (int i = 0; i < N; ++i) {
		//	n[i] = G*norm(function[i]);	
		//}

		 ///additional potential of harmonic oscillator/////
		for (int i = 0; i <= N / 2; ++i) {
			n[i] = G * norm(function[i]) + (i*dx)*(i*dx)*w*w*m / 2.0;
		}
		for (int i = N / 2 + 1; i < N; ++i) {
			n[i] = G * norm(function[i]) + ((i - N)*dx)*((i - N)*dx)*w*w*m / 2.0;
		}
		//////////////////

		for (int i = 0; i < N; ++i) {
			temp[i] = (exp((-I)*(-I)*dt*n[i])*function[i]);
		}

		fftw_execute(p);  //fourier transformation

		for (int i = 0; i <= N / 2; ++i) {
			k = (double)i / range*2*PI;
			inC[i] = exp(-I * (-I)*dt*k*k/2.0)*toComplex(outC[i]);
		}
		for (int i = 0; i < N / 2; ++i) {
			//k = 2*PI*i/(c*N*dt);
			k = (-1.0 / (2 *dx) + (double)(i) / range) * 2 * PI;
			inC[N / 2 + i+1] = exp(-I * (-I)*dt*k*k/2.0)*toComplex(outC[N/2+i+1]);
		}

		fftw_execute(pInverse); //backward fourier transfrormation

		++time;

		double old = 0;
		double now = 0;

		for (int i = 0; i < N; ++i) {
			old += norm(function[i])*dx;
			function[i] = toComplex(out[i])/ (double)N ; //normalizing and saving output
			now += norm(function[i])*dx;
		}

		for (int i = 0; i < N; ++i) {
			function[i] *= sqrt(old / now);
		}
		
		//ploting results during compilation
		if (time % 200 == 0 && time > 1500 && 0) {
			showM(function);
		}
	
	}

	fftw_destroy_plan(p);
	fftw_destroy_plan(pInverse);

	//plotting resoults
		//showR(function);
		showM(function);
	
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
	for (int i = 0; i <= N / 2; ++i) {
		test[i] = m*w*w/2.0/G*(pow(3*N0*G/(m*w*w),2.0/3.0)- pow((i * dx / x0),2.0));
		if (real(test[i]) < 0) 
			test[i] = 0;
		else
			test[i] = sqrt(test[i]);
	}
	for (int i = N / 2 + 1; i < N; ++i) {
		test[i] = m * w * w / 2.0 /G* (pow(3 * N0*G / (m * w * w), 2.0 / 3.0) - pow(((i - N) * dx / x0), 2.0));
		if (real(test[i]) < 0)
			test[i] = 0;
		else
			test[i] = sqrt(test[i]);
	}

	//showR(test);
	showM(test);

	complex<double> roznica[N];
	for (int i = 0; i < N; ++i) {
		roznica[i] = function[i] - test[i];
	}
	//showR(roznica);
	showM(roznica);

	_getch();
    return 0;
}
