
#include <cstdio>
#include <iostream>
#include <system_error>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>
#include <SDL2/SDL.h>

using namespace std::string_literals;

class Program {
	std::vector<std::string> args;
	// SDL_Window* window;
	bool running = true;

public:
	Program(const std::vector<std::string>& args) : args(args) {
		// init sdl
		if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
			throw std::runtime_error(SDL_GetError());
		}
		std::cout << "SDL_Init()" << std::endl;

		//// create a window
		//window = SDL_CreateWindow(
		//	"DriveAssist",
		//	SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		//	1280, 720,
		//	0);
		//if (window == nullptr) {
		//	throw std::runtime_error(SDL_GetError());
		//}
		//std::cout << "SDL_CreateWindow()" << std::endl;
	}

	int run() {
		for (int i = 1; i <= 16; i++) {
			std::cout << "Processing image " << i << "... ";

			// load the image
			cv::Mat testImage = cv::imread(
				"C:/Users/razz/Desktop/DriveAssist/Test Images/snap ("s + std::to_string(i) + ").png"s);
			if (testImage.empty()) throw std::runtime_error("y image no work?");

			cv::Mat imageChannels[3];
			cv::split(testImage, imageChannels);

			// subtract the blue/green channels from red
			imageChannels[2] -= imageChannels[0];
			imageChannels[2] -= imageChannels[1];
			imageChannels[2] *= 4;				// amplify the dim red bits

			// make it gray
			cv::Mat gray = imageChannels[2];	// pick out the red channel
			cv::medianBlur(gray, gray, 5);		// blur to lower the amound of false positives

			std::vector<cv::Vec3f> circles;
			cv::HoughCircles(
				gray, circles, cv::HOUGH_GRADIENT, 1,
				gray.rows / 16,
				100, 30, 1, 40);

			std::vector<cv::Mat> circleImages;

			// mark the circles on the map
			for (const auto& c : circles) {
				auto center = cv::Point(c[0], c[1]);
				auto radius = c[2];
				
				// cut out an image
				int cutRadius = radius * 2;
				cv::Mat cut = testImage(cv::Rect(
					center.x - cutRadius / 2,
					center.y - cutRadius / 2,
					cutRadius, cutRadius)).clone();
				circleImages.push_back(cut);

				// circle center
				cv::circle(testImage, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);

				// circle outline
				cv::circle(testImage, center, radius, cv::Scalar(255, 0, 255), 3, cv::LINE_AA);
			}

			std::cout << circleImages.size() << " images found." << std::endl;

			cv::namedWindow("");
			if (circleImages.size() > 0) {
				cv::namedWindow("Random circle in image");
				cv::imshow("Random circle in image", circleImages[rand() % circleImages.size()]);
			}
			cv::imshow("", testImage);
			cv::waitKey(0);
			if (circleImages.size() > 0) cv::destroyWindow("Random circle in image");
			cv::destroyWindow("");
		}

		//SDL_Event ev;
		//std::cout << "Starting event loop" << std::endl;
		//while (running) {
		//	while (SDL_PollEvent(&ev)) {
		//		if (ev.type == SDL_QUIT) {
		//			running = false;
		//		}
		//	}
		//}

		return 0;
	}

	~Program() {
		//// destory window, exit program
		//SDL_DestroyWindow(window);
		//std::cout << "SDL_DestroyWindow()" << std::endl;
		SDL_Quit();
		std::cout << "SDL_Quit()" << std::endl;
	}
};

#undef main
int main(int argc, char* argv[])
{
#if !_DEBUG
	try {
#endif
	Program program(std::vector<std::string>(argv, argv + argc));
	return program.run();
#if !_DEBUG
	}
	catch (const std::exception& ex) {
		std::cerr << "A C++ exception has occurred." << std::endl;
		std::cerr << "Error message: " << ex.what() << std::endl;
		// TODO: Add some sort of stack trace ?
		return 1;
	}
#endif

}
