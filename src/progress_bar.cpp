#include "progress_bar.hpp"

ProgressBar::ProgressBar() {
	completed = 0;
	stages = 20;
	std::cout << std::setw(50) << "[";
	for (int i = 0; i < stages; ++i) {
		std::cout << '-';
	}
	std::cout << "]";
	//returning coursor to the beginning of progress bar
	for (int i = 0; i <= stages; ++i) {
		std::cout << '\b';
	}
	std::cout << std::flush; //making sure progress bar is displayed on the screen
}
ProgressBar::ProgressBar(int n) {
	completed = 0;
	stages = n;
	std::cout << std::setw(30) << "[";
	for (int i = 0; i < stages; ++i) {
		std::cout << '-';
	}
	std::cout << "]";
	//returning coursor to the beginning of progress bar
	for (int i = 0; i <= stages; ++i) {
		std::cout << '\b';
	}
	std::cout << std::flush; //making sure progress bar is displayed on the screen
}
void ProgressBar::nextStage(){
	if (completed < stages) {
		++completed;
		std::cout << '#' << std::flush;
	}
}