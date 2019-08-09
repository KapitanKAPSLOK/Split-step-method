#pragma once
#include "Matrix.h"

//constructors
template<typename T>
Matrix<T>::Matrix(int h, int w, T** t) {
	if (h < 0 || t < 0) {
		width = 0;
		height = 0;
		tab = nullptr;
		return;
	}
	width = static_cast<unsigned>(w);
	height = static_cast<unsigned>(h);
	//allocating memory
	tab = new T * [height];
	for (int i = 0; i < height; ++i) {
		tab[i] = new T[width];
	}
	//assinging values
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			tab[i][j] = t[i][j];
		}
	}
}

template<typename T>
Matrix<T>::Matrix(int h, int w) {
	if (h < 0 || w < 0) {
		width = 0;
		height = 0;
		tab = nullptr;
		return;
	}
	width = static_cast<unsigned>(w);
	height = static_cast<unsigned>(h);
	//allocating memory
	tab = new T * [height]();
	for (int i = 0; i < height; ++i) {
		tab[i] = new T[width];
	}
}

//destructor
template<typename T>
Matrix<T>::~Matrix() {
	for (int i = 0; i < height; ++i) {
		delete[] tab[i];
	}
	delete[] tab;
}

////////operators////////

template<typename T>
Matrix<T> Matrix<T>::operator+(const Matrix& m){
	if (this->width != m.width) throw "Matrices are not in the same size";
	if (this->height != m.height) throw "Matrices are not in the same size";

	Matrix<T> *result= new Matrix<T>(this->height,this->width);
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			(*result)[i][j] = (*this)[i][j] + m[i][j];
		}
	}
	return (*result);
}

template<typename T>
Matrix<T>& Matrix<T>::operator+=(const Matrix& m){
	if (this->width != m.width) throw "Matrices are not in the same size";
	if (this->height != m.height) throw "Matrices are not in the same size";

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			(*this)[i][j] += m[i][j];
		}
	}
	return (*this);
}

template<typename T>
Matrix<T> Matrix<T>::operator-(const Matrix& m) {
	if (this->width != m.width) throw "Matrices are not in the same size";
	if (this->height != m.height) throw "Matrices are not in the same size";

	Matrix<T>* result = new Matrix<T>(this->height, this->width);
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			(*result)[i][j] = (*this)[i][j] - m[i][j];
		}
	}
	return (*result);
}

template<typename T>
Matrix<T>& Matrix<T>::operator-=(const Matrix& m) {
	if (this->width != m.width) throw "Matrices are not in the same size";
	if (this->height != m.height) throw "Matrices are not in the same size";

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			(*this)[i][j] -= m[i][j];
		}
	}
	return (*this);
}

template<typename T>
Matrix<T>& Matrix<T>::operator=(const Matrix& m){
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			tab[i][j] = m[i][j];
		}
	}
	return *this;
}

template<typename T>
Matrix<T> Matrix<T>::operator*(const Matrix& m)
{
	if (this->width != m.height) throw "Invalid matrix size.";//nie da siê pomno¿yæ macierzy

	Matrix<T>* result = new Matrix<T>(this->height, m.width);
	for (int i = 0; i < m.width; ++i) {
		for (int j = 0; j < width; ++j) {
			for (int k = 0; k < width; ++k) {
				(*result)[j][i] += (*this)[j][k] * m[k][i];
			}
		}
	}
	return (*result);
}
template<typename T>

Matrix<T>& Matrix<T>::operator*=(const Matrix& m){
	*this = (*this)*m;
	return (*this);
}

template<typename T>
Matrix<T> Matrix<T>::operator*(const T& c){
	Matrix<T>* result = new Matrix<T>(this->height, this->width);
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			(*result)[i][j] = (*this)[i][j]*c;
		}
	}
	return (*result);
}

template<typename T>
Matrix<T>& Matrix<T>::operator*=(const T& c) {
	Matrix<T>* result = new Matrix<T>(this->height, this->width);
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			(*this)[i][j] = (*this)[i][j] * c;
		}
	}
	return (*this);
}

template<typename T>
Matrix<T> operator*(const T c, const Matrix<T>& m){
	return m * c;
}

template<typename T>
T* Matrix<T>::operator[](const int row){
	return tab[row];
}

template<typename T>
const T* Matrix<T>::operator[](const int row) const {
	return tab[row];
}

//////methods//////

template<typename T>
T* Matrix<T>::at(const int row)
{
	if (row >= height || row < 0) throw "Index out of bound.";
	return tab[row];
}

