#pragma once

#include <iostream>

template <typename T> class Matrix
{
private:
	unsigned width;
	unsigned height;
	T** tab;
public:
	Matrix(int h, int w, T** t);
	Matrix(int h, int w);
	~Matrix();

	T* operator[](const int row);
	const T* operator[](const int row) const;
	T* at(const int row);
	Matrix operator+(const Matrix &m);
	Matrix& operator+=(const Matrix &m);
	Matrix operator-(const Matrix& m);
	Matrix& operator-=(const Matrix& m);
	Matrix& operator=(const Matrix &m);
	Matrix operator*(const Matrix& m);
	Matrix& operator*=(const Matrix& m);
	Matrix operator*(const T& m);
	Matrix& operator*=(const T& c);
	//friend Matrix operator*(const T& m1, const Matrix& m2);

	unsigned getWidth() const { return width; }
	unsigned getHeight() const { return height; }
};

template <typename T>
Matrix<T> operator*(const T m1, const Matrix<T>& m2);

//for tests
template <typename T>
std::ostream& operator<<(std::ostream& os, const Matrix<T>& m)
{
	for (int i = 0; i < m.getHeight(); ++i) {
		os << std::endl;
		for (int j = 0; j < m.getWidth(); ++j) {
			os << m[i][j] <<" ";
		}
	}
	return os;
}