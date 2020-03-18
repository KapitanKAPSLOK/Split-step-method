/*
Class implementing progress bar in console.
*/

#pragma once

#include <iomanip>
#include <iostream>

class ProgressBar {
private:
	int stages;
	int completed;
public:
	ProgressBar();
	ProgressBar(int);
	~ProgressBar() = default;
	void nextStage();
};