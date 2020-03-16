#include "discpp.h"
#include "fftw3.h"

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <conio.h>
#include <complex>
#include <math.h>
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
const complex<double> I(0, 1); //sqruare root of -1

double T; //total propagtion time
int N; //number of given points (must be odd)
double dx; //range between two points
double range; //total domain range
double dt; //time step
int maxTime; //number of steps in simulation 
double G; //nonlinearity coeficient
double u;
double w;
double vortex;
unsigned numberOfRings; //nuber of rings

float *x, *y; //points to plot

Dislin g; //plotting object
int t = 0; //actual time

//corelation between rings cooficients
complex<double> **J;
complex<double> J0 = 1.1;
complex<double> *J1;
complex<double> *J2;

//topological charge parameters
int resolutionPoints;
//const int resolution = 500;
int resolution; //for how many steps in simulation one point of topological charge is calculated

Eigen::Matrix<complex<double>, Eigen::Dynamic, 1>  *functionVec;//wave function
Eigen::Matrix<complex<double>, Eigen::Dynamic, Eigen::Dynamic>  *C; //matrix of cooficients

//converting fftw_complex to  comlex<double>
complex<double> toComplex(fftw_complex a) {
	return complex<double>(a[REAL], a[IMAG]);
}

//changing the order of points to normal, in fftw calculation first and second part of function are reversed
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
complex<double>* fixOrder(Eigen::Matrix<complex<double>, Eigen::Dynamic, 1> * a, int ring) {
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

//plotting modulo of function
void showM(Eigen::Matrix<complex<double>, Eigen::Dynamic, 1> * a) {
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

void showIm(Eigen::Matrix<complex<double>, Eigen::Dynamic, 1> * a) {
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

//calculates topological charge of function in [tab] and return the value via [val]
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

void topologicalCharge(Eigen::Matrix<complex<double>,Eigen::Dynamic,1>  *tab, float* val, int ring) {
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

//signals to communicate between processes
bool mainReady = true;
bool *processed;
bool processed1 = false, processed2 = false, processed3 = false;

//excluded from optimalization to avoid removing it by compilator
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

int index = 0; //current point number in topological charge plot

void computeRing(int ringNumber, complex<double> *temp, double k, complex<double>* inC, fftw_complex* outC, 
	fftw_plan p, fftw_plan pInverse, fftw_complex *out, double range2, bool &processed, float *topCharge) {

	while (t < maxTime) {

		waitForMain(); //synchronization of processes in every single dt time step

		//topological charge calculation
		if (t % resolution == 0) {
			topologicalCharge(functionVec, &topCharge[index], ringNumber);
		}

		//split-step calculation
		for (int i = 0; i < N; ++i) {
			//temp[i] = exp(dt * (((-I - G) * norm(functionVec[i][ringNumber])) + u)) * C[i].row(ringNumber) * functionVec[i];
			//temp[i] = exp(dt * (((-I - G) * norma) + u)) * C[i].row(ringNumber) * functionVec[i];
			temp[i] = exp(dt * u*u/(G* norm(functionVec[i][ringNumber]) + u)) * C[i].row(ringNumber) * functionVec[i];
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
		processed = true; //thread signalize end of calculations

		waitForNotMain();
		//saving output
		for (int i = 0; i < N; ++i) {
			functionVec[i][ringNumber] = toComplex(out[i]) / (double)N;
		}

		processed = false;
	}
}

/////////////////////////////////////////////////

#include <fstream>

int main(int argc, char* argv[])
{
	//g.metafl("CONS"); //plotting graphs in terminal
	g.metafl("PNG"); //saving plots as png file

	int lineNr = 0; //line number for error handling
	int succCommands = 0; //number of succesfully readed commands 
	
	//opening file with input data
	ifstream inputFile;
	inputFile.open("splitStepInput.in");
	if (!inputFile.is_open()) {
		cout << "Cannot open the file";
		return 1;
	}
	cout << "| w |" << " J0 |"  << " G |"  << " u |"  << +" vortex |"  << " N |"  <<" resolutionPoints |"  << " T |" << endl;
	while (!inputFile.eof()) {
		index = 0;
		t = 0;
		++lineNr;
		//loading number of rings
		inputFile >> numberOfRings;
		if (inputFile.fail())
			throw "numberOfRings (number of rings)";
		//loading gaussian coupling width
		inputFile >> w;
		if (inputFile.fail())
			throw "w (coupling width)";
		//loading coupling strenght
		double tmp;
		inputFile >> tmp;
		if (inputFile.fail())
			throw "J0 (coupling strenght)";
		J0 = complex<double>(tmp, 0);
		//loading big gamma parameter
		inputFile >> G;
		if (inputFile.fail())
			throw "G (big gamma parameter)";
		//loading small gamma parameter
		inputFile >> u;
		if (inputFile.fail())
			throw "u (small gamma parameter)";
		//loading starting vortex
		inputFile >> vortex;
		if (inputFile.fail())
			throw "vortex (starting vortex)";
		//loading number of points in wave function
		inputFile >> N;
		if (inputFile.fail())
			throw "N (number of points)";
		//loading number of points in topological charge plot
		inputFile >> resolutionPoints;
		if (inputFile.fail())
			throw "resolutionPoints (number of points in topological charge plot)";
		//loading total time of simulation
		inputFile >> T;
		if (inputFile.fail())
			throw "T (total time of simulation)";

		//calculating dipendent cooficients
		dx = 2 * PI / (N - 1); //range between two points
		range = dx * (N - 1); //total domain range
		dt = dx * dx / PI; //time step
		maxTime = T / dt; //number of steps in simulation 
		resolution = maxTime / resolutionPoints + 1; //for how many steps in simulation one point of topological charge is calculated
		double range2 = 2 * PI / range;

		//cout << "w-" << w << " J0-" << J0 << " G-" << G << " u-" << u << +" vortex-" << vortex << " N-" << N <<
		//	" resolutionPoints-" << resolutionPoints << " T-" << T << endl;
		cout << w << " " << J0 << " " << G << " " << u << +" " << vortex << " " << N <<
			" " << resolutionPoints << " " << T << endl;

		//allocating dynamic structures
		x = new float[N];
		y = new float[N];
		J = new complex<double> * [N];
		for (int i = 0; i < N; ++i) {
			J[i] = new complex<double>[numberOfRings];
		}
		J1 = new complex<double>[N];
		J2 = new complex<double>[N];
		functionVec = new Eigen::VectorXcd[N];//wave functions
		for (int i = 0; i < N; ++i) {
			functionVec[i] = Eigen::VectorXcd(numberOfRings);
		}
		C = new Eigen::MatrixXcd[N]; //matrix of cooficients
		for (int i = 0; i < N; ++i) {
			C[i] = Eigen::MatrixXcd(numberOfRings, numberOfRings);
		}
		processed = new bool[numberOfRings] {false};

		complex<double>** temp = new complex<double> * [numberOfRings];
		for (int i = 0; i < numberOfRings; ++i) {
			temp[i] = new complex<double>[N];
		}

		//topological charge
		float** topCharge = new float* [numberOfRings];
		for (int i = 0; i < numberOfRings; ++i) {
			topCharge[i] = new float[maxTime / resolution + 1];
		}
		float* xAxis = new float[maxTime / resolution + 1];
		for (int i = 0; i <= maxTime / resolution; ++i) {
			xAxis[i] = i * (resolution * dt);
		}

		


		//values for corelation function
		for (int i = 0; i <= N / 2; ++i) {
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
			//the matrix of influence rings to each other
			for (int j = 0; j < numberOfRings; ++j) {
				if (j - 1 >= 0) {
					if (j % 2 == 0)
						C[i](j, j - 1) = J2[i];
					else
						C[i](j, j - 1) = J1[i];
				}
				if (j + 1 < numberOfRings) {
					if (j % 2 == 0)
						C[i](j, j + 1) = J1[i];
					else
						C[i](j, j + 1) = J2[i];
				}
			}
			//calculating exponent of above matrix
			C[i] *= -dt * I;
			C[i] = C[i].exp();
			C[i] = C[i].array().isNaN().select(0, C[i]).eval(); //removing NaN values from exp result
			if (C[i] == Eigen::MatrixXcd::Zero(numberOfRings, numberOfRings)) {
				for (int j = 0; j < numberOfRings; ++j) {
					C[i](j, j) = complex<double>(1, 0);
				}
			}
		}

		//x axis data for plots
		for (int i = -N / 2; i <= N / 2; ++i) {
			x[i + N / 2] = dx * i;
		}

		//g.disini();
		//g.titlin("funkcja J(x) korelacji okregow", 3);
		//showR(J1);
		//showR(J2);


		double k; //wave number

		complex<double> **inC=new complex<double>* [numberOfRings];
		for (int i = 0; i < numberOfRings; ++i) {
			inC[i] = new complex<double>[N];
		}
		fftw_complex **outC= new fftw_complex*[numberOfRings];
		for (int i = 0; i < numberOfRings; ++i) {
			outC[i] = new fftw_complex[N];
		}
		fftw_complex **out=new fftw_complex * [numberOfRings];
		for (int i = 0; i < numberOfRings; ++i) {
			out[i] = new fftw_complex[N];
		}

		fftw_plan* p = new fftw_plan[numberOfRings];
		fftw_plan * pInverse = new fftw_plan[numberOfRings];
		for (int i = 0; i < numberOfRings; ++i) {
			p[i] = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(temp[i]), outC[i], FFTW_FORWARD, FFTW_MEASURE);
		}
		for (int i = 0; i < numberOfRings; ++i) {
			pInverse[i] = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(inC[i]), out[i], FFTW_BACKWARD, FFTW_MEASURE);
		}

		////ploting starting data
		//showM(functionVec);


		//	assert(fftw_init_threads());

		Eigen::initParallel();

		thread** ring = new thread*[numberOfRings];
		for (int i = 0; i < numberOfRings; ++i) {
			ring[i] = new thread(computeRing, i, temp[i], k, inC[i], outC[i], p[i], pInverse[i], out[i], range2, ref(processed[i]), topCharge[i]);
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
		//freeing resources
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
			std::string fileName = "results/" + std::to_string(numberOfRings) + "rings/w-" + std::to_string(w) + 
				"_J0-" + std::to_string(J0.real()) + "_G-" + std::to_string(G) + "_u-" +std::to_string(u) + 
				"_vortex-" + std::to_string(vortex) + "_N-" + std::to_string(N) + "_resolutionPoints-"+std::to_string(resolutionPoints) +
				"_T-" + std::to_string(dt * t) + "_ring_" + std::to_string(i + 1) + ".png";
			g.setfil(fileName.c_str());

			g.disini();
			//plot title
			std::string tmpStr = "topological charge ring " + std::to_string(i + 1);
			g.titlin(tmpStr.c_str(), 3);
			
			g.errmod("ALL", "OFF");//turning off all output information in console
			g.qplot(xAxis, topCharge[i], maxTime / resolution + 1);
		}

		//memory deallocations
		delete[] x;
		delete[] y;
		for (int i = 0; i < N; ++i) {
			delete[] J[i];
		}
		delete[] J;
		delete[] J1;
		delete[] J2;
		delete[] functionVec;
		delete[] C;
		delete[] processed;
		for (int i = 0; i < numberOfRings; ++i) {
			delete[] topCharge[i];
		}
		delete[] topCharge;
		delete[] xAxis;
		for (int i = 0; i < numberOfRings; ++i) {
			delete[] temp[i];
		}
		delete[] temp;
		for (int i = 0; i < numberOfRings; ++i) {
			delete[] inC[i];
		}
		delete[] inC;
		for (int i = 0; i < numberOfRings; ++i) {
			delete[] outC[i];
		}
		delete[] outC;
		for (int i = 0; i < numberOfRings; ++i) {
			delete[] out[i];
		}
		delete[] out;
		delete[] p;
		delete[] pInverse;
		for (int i = 0; i < numberOfRings; ++i) {
			delete ring[i];
		}
		delete[] ring;
	}
	inputFile.close();
	return 0;
}

