#include "discpp.h"
#include "fftw3.h"
#include "progress_bar.hpp"

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <conio.h>
#include <complex>
#include <math.h>
#include <thread>
#include <fstream>
#include <vector>


#include <unsupported/Eigen/MatrixFunctions>
#include <Eigen/Dense>
#include <Eigen/Core>

#include <assert.h>
#include "progress_bar.hpp"


using namespace std;

//define macros for fftw_complex
#define REAL 0
#define IMAG 1

const double PI = 3.14159265358979323846;
const complex<double> I(0, 1); //sqruare root of -1

double T; //total propagtion time
int N; //number of given points (should be even)
double dx; //range between two points
double range; //total domain range
double dt; //time step
int maxTime; //number of steps in simulation 
double G; //nonlinearity coeficient
double u;
double w;
double vortex;
unsigned numberOfRings; //nuber of rings
double d; //pt-symmetry paremeter

vector<float> x, y; //points to plot

Dislin g; //plotting object
int t = 0; //actual time

//corelation between rings cooficients
//complex<double> **J;
complex<double> J0 = 1.1;
vector<complex<double>> J1;
vector<complex<double>> J2;

//topological charge parameters
int resolutionPoints;
//const int resolution = 500;
int resolution; //for how many steps in simulation one point of topological charge is calculated

vector<Eigen::Matrix<complex<double>, Eigen::Dynamic, 1>>  functionVec;//wave function
vector<Eigen::Matrix<complex<double>, Eigen::Dynamic, Eigen::Dynamic>>  C; //matrix of cooficients

//converting fftw_complex to  comlex<double>
complex<double> toComplex(fftw_complex a) {
	return complex<double>(a[REAL], a[IMAG]);
}

//changing the order of points to normal, in fftw calculation first and second part of function are reversed
complex<double>* fixOrder(fftw_complex* a) {
	complex<double>* b;
	b = new complex<double>[N];
	for (int i = 0; i < N / 2; ++i) {
		b[i] = toComplex(a[N / 2 + i]);
	}
	for (int i = 0; i < N / 2; ++i) {
		b[N / 2 + i] = toComplex(a[i]);
	}
	return b;
}

complex<double>* fixOrder(complex<double>* a) {
	complex<double>* b;
	b = new complex<double>[N];
	for (int i = 0; i < N / 2; ++i) {
		b[i] = a[N / 2 + i];
	}
	for (int i = 0; i < N / 2; ++i) {
		b[N / 2 + i] = a[i];
	}
	return b;
}
complex<double>* fixOrder(Eigen::Matrix<complex<double>, Eigen::Dynamic, 1> * a, int ring) {
	complex<double>	*b = new complex<double>[N];
	for (int i = 0; i < N / 2; ++i) {
		b[i] = a[N / 2 + i][ring];
	}
	for (int i = 0; i < N / 2; ++i) {
		b[N / 2 + i] = a[i][ring];
	}
	return b;
}

double* fixOrder(double* a) {
	double* b;
	b = new double[N];
	for (int i = 0; i < N / 2; ++i) {
		b[i] = a[N / 2 + i];
	}
	for (int i = 0; i < N / 2; ++i) {
		b[N / 2 + i] = a[i];
	}
	return b;
}
float* fixOrder(float* a) {
	float* b;
	b = new float[N];
	for (int i = 0; i < N / 2; ++i) {
		b[i] = a[N / 2 + i];
	}
	for (int i = 0; i < N / 2; ++i) {
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
	g.qplot(x.data(), y.data(), N);
}

//ploting  function with points
void showP(double* a) {
	double* temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = temp[i];
	}
	g.qplsca(x.data(), y.data(), N);
}

//plotting modulo of functions on all rings
void showM(Eigen::Matrix<complex<double>, Eigen::Dynamic, 1> * a) {
	for (int i = 0; i < numberOfRings; ++i){
		std::string fileName = "results/" + std::to_string(numberOfRings) + "rings/pt-f-g10_w-" + std::to_string(w) +
			"_J0-" + std::to_string(J0.real()) + "_G-" + std::to_string(G) + "_u-" + std::to_string(u) +
			"_vortex-" + std::to_string(vortex) + "_d-" + std::to_string(d) + "_N-" + std::to_string(N) + "_resolutionPoints-" + std::to_string(resolutionPoints) +
			"_T-" + std::to_string(dt * t) + "_ring_" + std::to_string(i + 1) + ".png";
		g.setfil(fileName.c_str());
		g.disini();
		g.errmod("ALL", "OFF");//turning off all output information in console
		std::string tmpStr = "modul funkcji na okregu " + std::to_string(i+1) + " po przepropagowaniu o " + std::to_string(t) + " krokow przy dt= " + std::to_string(dt) + ", T=" + std::to_string(dt * t);
		g.titlin(tmpStr.c_str(), 3);
		complex<double> * temp = fixOrder(a, i);
		for (int i = 0; i < N; ++i) {
			y[i] = abs(temp[i]);
		}
		g.qplot(x.data(), y.data(), N);
		delete[] temp;
		}
}
//plotting modulo of function on ring [ring]
void showM(Eigen::Matrix<complex<double>, Eigen::Dynamic, 1> *a, int ring) {
		g.disini();
		std::string tmpStr = "modul funkcji na okregu " + std::to_string(ring + 1) + " po przepropagowaniu o " + std::to_string(t) + " krokow przy dt= " + std::to_string(dt) + ", T=" + std::to_string(dt * t);
		g.titlin(tmpStr.c_str(), 3);
		complex<double>* temp = fixOrder(a, ring);
		for (int i = 0; i < N; ++i) {
			y[i] = abs(temp[i]);
		}
		g.qplot(x.data(), y.data(), N);
		delete[] temp;
}

void showM(fftw_complex* a) {
	complex<double>* temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = abs(temp[i]);
	}
	g.qplot(x.data(), y.data(), N);
	delete[] temp;
}

void showR(fftw_complex* a) {
	complex<double>* temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = temp[i].real();
	}
	g.qplot(x.data(), y.data(), N);
}

void showR(complex <double>* a) {
	complex<double>* temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = temp[i].real();
	}
	g.qplot(x.data(), y.data(), N);
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
		g.qplot(x.data(), y.data(), N);
		delete[] temp;
	}
}

void showIm(fftw_complex* a) {
	complex<double>* temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = temp[i].imag();
	}
	g.qplot(x.data(), y.data(), N);
}

void showM(complex<double>* a) {
	complex<double>* temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		//y[i] = sqrt(norm(temp[i]));
		y[i] = abs(temp[i]);
	}
	g.qplot(x.data(), y.data(), N);
}

void showIm(complex<double>* a) {
	complex<double>* temp = fixOrder(a);
	for (int i = 0; i < N; ++i) {
		y[i] = temp[i].imag();
	}
	g.qplot(x.data(), y.data(), N);
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

void normSums(Eigen::Matrix<complex<double>, Eigen::Dynamic, 1> * tab, float* val) {
	*val = 0;
	for (int i = 0; i < N; ++i) {
		*val += dx * (norm(tab[0][0]) + norm(tab[1][0]));
	}
}

////////////////////multithreading///////////////////

//signals to communicate between processes
bool mainReady = true;
unique_ptr<bool[]> processed;

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
	fftw_plan p, fftw_plan pInverse, fftw_complex *out, double range2, bool *processed, float *topCharge) {

	while (t < maxTime) {

		waitForMain(); //synchronization of processes in every single dt time step

		////topological charge calculation
		//if (t % resolution == 0) {
		//	topologicalCharge(functionVec.data(), &topCharge[index], ringNumber);
		//}

		//split-step calculation
		for (int i = 0; i < N; ++i) {
			////gain and lose model
			//temp[i] = exp(dt * (((-I - G) * norm(functionVec[i][ringNumber])) + u)) * C[i].row(ringNumber) * functionVec[i];

			////saturation
			//temp[i] = exp(dt * (u * u / (G * norm(functionVec[i][ringNumber]) + u) - I * norm(functionVec[i][ringNumber]))) * C[i].row(ringNumber) * functionVec[i];
			
			//pt-symmetry
			//different signs before small gamma in odd and even rings
			temp[i] = exp(dt * u * (ringNumber%2 ? 1.0 : -1.0)) * C[i].row(ringNumber) * functionVec[i];
		}

		fftw_execute(p);  //fourier transformation
		for (int i = 0; i < N / 2; ++i) {
			k = (double)i * range2;

			//gain&lose / saturation
			//inC[i] = exp(-I * dt * k * k) * toComplex(outC[i]);

			//pt-symmetry
			inC[i] = exp(-d * I * dt * k * k) * toComplex(outC[i]);
		}
		for (int i = 0; i < N / 2; ++i) {
			k = (-PI) / dx + (double)(i)* range2;

			////gain&lose / saturation
			//inC[N / 2 + i + 1] = exp(-I * dt * k * k) * toComplex(outC[N / 2 + i + 1]);

			//pt-symmetry
			inC[N / 2 + i] = exp(-d * I * dt * k * k) * toComplex(outC[N / 2 + i]);
		}
		fftw_execute(pInverse); //backward fourier transfrormation
		*processed = true; //thread signalize end of calculations

		waitForNotMain();
		//saving output
		for (int i = 0; i < N; ++i) {
			functionVec[i][ringNumber] = toComplex(out[i]) / (double)N;
		}

		*processed = false;
	}
}

/////////////////////////////////////////////////




int main(int argc, char* argv[])
{
	//g.metafl("CONS"); //plotting graphs in terminal
	g.metafl("PNG"); //saving plots as png file

	//complex<double>** temp; //for temporary data during split step computation
	vector<unique_ptr<float[]>> topCharge; //table of calculated value of topological fnction for different times
	unique_ptr<float[]> normSum; 
	
	vector<float> xAxis; //points of x axis for plot of topological charge

	//temporary variables for split step calculation
	vector<vector< complex<double> > > temp;
	vector<vector<complex<double>>> inC;
	//can't change unigue_ptr to another nested vector in below definitions because 
	//fftw_complex is typedef for two-dimensional array
	vector<unique_ptr<fftw_complex[]>> outC;
	vector<unique_ptr<fftw_complex[]>> out;

	//plans for fftw
	vector<fftw_plan> p;
	vector<fftw_plan> pInverse;

	//table of threads for every ring
	vector<unique_ptr<thread>> ring;


	int lineNr = 0; //line number for error handling
	int succCommands = 0; //number of succesfully readed commands 
	
	string line;
	//opening file with input data
	ifstream inputFile;
	inputFile.open("splitStepInput.in");
	if (!inputFile.is_open()) {
		std::cout << "Cannot open the file";
		return 1;
	}
	//legend of printed data during compilation
	std::cout << "| numberOfRings | w | J0 | G | u | vortex | d | N | resolutionPoints | T |" << endl;

	while (!inputFile.eof()) {
		index = 0;
		t = 0;
		++lineNr;
		//loading number of rings
		try {
			getline(inputFile, line);
			istringstream buf(line);
			buf >> numberOfRings;
			if (buf.fail())
				throw "numberOfRings (number of rings)";
			if (buf.get() != ' ')
				throw runtime_error("Syntax error.");
			//loading gaussian coupling width
			buf >> w;
			if (buf.fail())
				throw "w (coupling width)";
			if (buf.get() != ' ')
				throw runtime_error("Syntax error.");
			//loading coupling strenght
			double tmp;
			buf >> tmp;
			if (buf.fail())
				throw "J0 (coupling strenght)";
			J0 = complex<double>(tmp, 0);
			if (buf.get() != ' ')
				throw runtime_error("Syntax error.");
			//loading big gamma parameter
			buf >> G;
			if (buf.fail())
				throw "G (big gamma parameter)";
			if (buf.get() != ' ')
				throw runtime_error("Syntax error.");
			//loading small gamma parameter
			buf >> u;
			if (buf.fail())
				throw "u (small gamma parameter)";
			if (buf.get() != ' ')
				throw runtime_error("Syntax error.");
			//loading starting vortex
			buf >> vortex;
			if (buf.fail())
				throw "vortex (starting vortex)";
			if (buf.get() != ' ')
				throw runtime_error("Syntax error.");
			//loading cooficient before second derivative in schoringer equation
			buf >> d;
			if (buf.fail())
				throw "d (factor before second derivative)";
			if (buf.get() != ' ')
				throw runtime_error("Syntax error.");
			//loading number of points in wave function
			buf >> N;
			if (buf.fail())
				throw "N (number of points)";
			if (buf.get() != ' ')
				throw runtime_error("Syntax error.");
			//loading number of points in topological charge plot
			buf >> resolutionPoints;
			if (buf.fail())
				throw "resolutionPoints (number of points in topological charge plot)";
			if (buf.get() != ' ')
				throw runtime_error("Syntax error.");
			//loading total time of simulation
			buf >> T;
			if (buf.fail())
				throw "T (total time of simulation)";
			//checking if there is more data
			char c = buf.get();
			if (c != '\n' && c!=EOF)
				throw runtime_error("To many arguments at line "+lineNr);

			//calculating dipendent cooficients
			dx = 2 * PI / (N ); //range between two points
			range = dx * (N ); //total domain range
			dt = dx * dx / PI; //time ste
			maxTime = T / dt; //number of steps in simulation 
			resolution = maxTime / resolutionPoints + 1; //for how many steps in simulation one point of topological charge is calculated
			double range2 = 2 * PI / range;

			//allocating dynamic structures
			try {
				//vectors are resizing and than shrink_to_fit to free potential unused space

				//variables used in plots
				x.resize(N);
				x.shrink_to_fit();
				y.resize(N);
				y.shrink_to_fit();

				//copuling constants
				//J = new complex<double> * [N];
				//for (int i = 0; i < N; ++i) {
				//	J[i] = new complex<double>[numberOfRings];
				//}
				J1.resize(N);
				J1.shrink_to_fit();
				J2.resize(N);
				J2.shrink_to_fit();

				//wave functions
				functionVec.resize(N);
				functionVec.shrink_to_fit();
				for (int i = 0; i < N; ++i) {
					functionVec[i].resize(numberOfRings);
				}

				//matrix of copuling cooficients
				C.resize(N); 
				C.shrink_to_fit();
				for (int i = 0; i < N; ++i) {
					C[i].resize(numberOfRings, numberOfRings);
				}

				//variable for threads synchronization
				processed.reset(new bool[numberOfRings]);

				//variables for temporary data during split-step method calculation
				temp.resize(numberOfRings);
				temp.shrink_to_fit();
				for (int i = 0; i < numberOfRings; ++i) {
					temp[i].resize(N);
					temp[i].shrink_to_fit();
				}
				inC.resize(numberOfRings);
				inC.shrink_to_fit();
				for (int i = 0; i < numberOfRings; ++i) {
					inC[i].resize(N);
					inC[i].shrink_to_fit();
				}
				outC.resize(numberOfRings);
				outC.shrink_to_fit();
				for (int i = 0; i < numberOfRings; ++i) {
					outC[i].reset(new fftw_complex[N]);
				}
				out.resize(numberOfRings);
				out.shrink_to_fit();
				for (int i = 0; i < numberOfRings; ++i) {
					out[i].reset(new fftw_complex[N]);
				}

				//creating plans for fftw
				p.resize(numberOfRings);
				p.shrink_to_fit();
				pInverse.resize(numberOfRings);
				pInverse.shrink_to_fit();
				for (int i = 0; i < numberOfRings; ++i) {
					p[i] = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(temp[i].data()), outC[i].get(), FFTW_FORWARD, FFTW_MEASURE);
				}
				for (int i = 0; i < numberOfRings; ++i) {
					pInverse[i] = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(inC[i].data()), out[i].get(), FFTW_BACKWARD, FFTW_MEASURE);
				}

				//table of threads for every ring
				ring.resize(numberOfRings);

				//variable for calculated topological charge
				topCharge.resize(numberOfRings);
				topCharge.shrink_to_fit(); 

				normSum.reset(new float[maxTime / resolution + 1]);
			}
			catch (bad_alloc) {
				throw runtime_error("Cannot allocate enough memory. Line ommited.");
			}
			try {
				for (int i = 0; i < numberOfRings; ++i) {
					//topCharge[i]= unique_ptr<float[]>(new float[maxTime / resolution + 1]);
					topCharge[i].reset(new float[maxTime / resolution + 1]);
				}
				xAxis.resize(maxTime / resolution + 1);
				xAxis.shrink_to_fit();
				for (int i = 0; i <= maxTime / resolution; ++i) {
					xAxis[i] = i * (resolution * dt);
				}
			}
			catch (bad_alloc) {
				//trying to lower number of point on topological charge plot to fit in limited memory
				while (resolutionPoints >= 16) {
					resolutionPoints /= 2;
					resolution = maxTime / resolutionPoints + 1;
					int i = 0;
					for (i; i < numberOfRings; ++i) {
						topCharge[i].reset(new(nothrow) float[maxTime / resolution + 1]);
						if (!topCharge[i]) //if topCharge[i] is nulll allocation failure occured
							break;
						//topCharge[i].resize(maxTime / resolution + 1); //TODO:
					}
					if (i == numberOfRings) //memory for every ring was successfully allocated
						break;
				}
				if(resolutionPoints<16)
					throw runtime_error("Cannot allocate enough memory. Line ommited.");
				cout << "WARNING: Decreased resolutionPoints value due to memory shortage." << endl;
			}


			std::cout << numberOfRings << " " << w << " " << J0 << " " << G << " " << u << +" " << vortex << " " << d << " " << N <<
				" " << resolutionPoints << " " << T;


			//values for corelation function
			const int superGauss = 10;
			for (int i = 0; i < N / 2; ++i) {
				//J1[i] = J0 / sqrt(PI) / w * exp(-pow((i * dx) + PI / 2, 2) / w / w);
				//J2[i] = J0 / sqrt(PI) / w * exp(-pow((i * dx) - PI / 2, 2) / w / w);

				//super gaussian corelation
				J1[i] = exp(-pow(((i * dx)) / w, superGauss));
				J2[i] = exp(-pow(((i * dx) ) / w, superGauss));
			}
			for (int i = N / 2; i < N; ++i) {
				//J1[i] = J0 / sqrt(PI) / w * exp(-pow(((i - N) * dx) + PI / 2, 2) / w / w);
				//J2[i] = J0 / sqrt(PI) / w * exp(-pow(((i - N) * dx) - PI / 2, 2) / w / w);

				//super gaussian corelation
				J1[i] = exp(-pow((((i - N) * dx) ) / w, superGauss));
				J2[i] = exp(-pow((((i - N) * dx) ) / w, superGauss));
			}

			//starting data for t=0;
			for (int i = 0; i < N / 2; ++i) {
				for (int j = 0; j < numberOfRings; ++j) {

					////gain&lose / saturation
					//if (j == 0)
					//	functionVec[i][j] = sqrt(u / G) * exp(i * dx * I * vortex) + sin(i * dx) / 100;
					//else
					//	functionVec[i][j] = 0;

					//pt-symmetry
					functionVec[i][j] = 1;
				}

			}
			for (int i = N / 2; i < N; ++i) {
				for (int j = 0; j < numberOfRings; ++j) {
					////gain&lose / saturation
					//if (j == 0)
					//	functionVec[i][j] = sqrt(u / G) * exp((i - N) * dx * I * vortex) + sin((i - N) * dx) / 100;
					//else
					//	functionVec[i][j] = 0;

					//pt-symmetry
					functionVec[i][j] = 1;
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
			for (int i = -N / 2; i < N / 2; ++i) {
				x[i + N / 2] = dx * i;
			}

			//g.disini();
			//g.titlin("funkcja J(x) korelacji okregow", 3);
			//showR(J1);
			//showR(J2);


			double k; //wave number


			//	assert(fftw_init_threads());

			Eigen::initParallel();

			for (int i = 0; i < numberOfRings; ++i) {
				ring[i].reset(new thread(computeRing, i, temp[i].data(), k, inC[i].data(), outC[i].get(),
					p[i], pInverse[i], out[i].get(), range2, &(processed[i]), topCharge[i].get()));
			}

			int stages = 20;
			ProgressBar progress(stages); //adding progress bar
			//////////////////////main loop//////////////
			while (t <= maxTime) {
				waitForProcessed();

				if (t % resolution == 0) {
					normSums(functionVec.data(), &normSum.get()[index]);
					++index;
				}
					
				mainReady = false;
				waitForNotProcessed();

				++t;

				mainReady = true;

				//if (t % 10000 == 0) {
				//	showM(functionVec.data(), 3);
				//}

				//chceking actual progress
				if (t % (maxTime / stages) == 0)
					progress.nextStage();
			}

			cout << endl;

			//freeing resources
			for (int i = 0; i < numberOfRings; ++i) {
				ring[i]->join();
				fftw_destroy_plan(p[i]);
				fftw_destroy_plan(pInverse[i]);
			}

			//showM(functionVec);

			////ploting topological charge
			//for (int i = 0; i < numberOfRings; ++i) {
			//	topCharge[i][maxTime / resolution - 1] = -6;
			//	topCharge[i][maxTime / resolution] = 6;

			//	//setting file name
			//	std::string fileName = "results/" + std::to_string(numberOfRings) + "rings/pt-t_w-" + std::to_string(w) +
			//		"_J0-" + std::to_string(J0.real()) + "_G-" + std::to_string(G) + "_u-" + std::to_string(u) +
			//		"_vortex-" + std::to_string(vortex) + "_d-" + std::to_string(d) + "_N-" + std::to_string(N) + "_resolutionPoints-" + std::to_string(resolutionPoints) +
			//		"_T-" + std::to_string(dt * t) + "_ring_" + std::to_string(i + 1) + ".png";
			//	g.setfil(fileName.c_str());

			//	g.disini();
			//	//plot title
			//	std::string tmpStr = "topological charge ring " + std::to_string(i + 1);
			//	g.titlin(tmpStr.c_str(), 3);

			//	g.errmod("ALL", "OFF");//turning off all output information in console
			//	g.qplot(xAxis.data(), topCharge[i].get(), maxTime / resolution + 1);
			//}
			

			std::string fileName = "results/" + std::to_string(numberOfRings) + "rings/pt-n_w-" + std::to_string(w) +
				"_J0-" + std::to_string(J0.real()) + "_G-" + std::to_string(G) + "_u-" + std::to_string(u) +
				"_vortex-" + std::to_string(vortex) + "_d-" + std::to_string(d) + "_N-" + std::to_string(N) + "_resolutionPoints-" + std::to_string(resolutionPoints) +
				"_T-" + std::to_string(dt * t) + ".png";
			g.setfil(fileName.c_str());
			g.disini();
			g.errmod("ALL", "OFF");//turning off all output information in console
			g.qplot(xAxis.data(), normSum.get(), maxTime / resolution + 1);

			showM(functionVec.data());
		}
		catch (string s) {
			cout << "Cannot load " << s << " in line " << lineNr << ". Rest of the line ommited." << endl;
		}
		catch (runtime_error r) {
			cout << "Error occured in line " << lineNr << ": " << r.what() << endl;
		}
		catch (...) {
			cout << "Unrecognized error occured while calculating line " << lineNr<<"." << endl;
		}
	}

	inputFile.close();
	return 0;
}

