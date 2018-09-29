#pragma once
#include <string>
#include <chrono>
#include <iostream>
#include "fftw3.h"

#define REAL 0
#define IMAG 1

typedef short int sample;

#define LOOP true				//audio plays nonstop
#define DEBUG_ON true			//shows debug window
#define RENDER_ON true			//draws samples and amplitudes
#define POINT_OR_LINE true		//draws samples and amplitudes as points or lines (false = point, true = line)

#define white Cor(255, 255, 255, 255)
#define gray Cor(127, 127, 127, 255)
#define black Cor(0, 0, 0, 255)
#define red Cor(255, 0, 0, 255)
#define green Cor(0, 255, 0, 255)
#define blue Cor(0, 0, 255, 255)
#define yellow Cor(255, 255, 0, 255)
#define magenta Cor(255, 0, 255, 255)
#define cyan Cor(0, 255, 255, 255)

class Utils
{
private:
	static bool isBigEndian();
public:
	static int Utils::convertToInt(char* buffer, int len);
	static std::string convertToString(char* buffer, int len);
	static void Utils::fft(double* amplitudes, double* phases, sample* window, unsigned int numSamples);
	static double Utils::db(double amplitude);
	static void Utils::hamming(sample* window, sample* samples, unsigned int first, unsigned int size);
	static double Utils::energy(double* amplitudesMono, unsigned int first, unsigned int size);
	static double Utils::energy(double* amplitudesLeft, double* amplitudesRight, unsigned int first, unsigned int size);
	template <typename T>
	static void shiftAdd(T* arr, unsigned int size, T newValue);
};

//https://stackoverflow.com/questions/31487876/getting-a-time-difference-in-milliseconds
class Timer
{
public:
	void start()
	{
		beginning = std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now());
		initial = beginning;
	}

	void reset()
	{
		initial = std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now());
	}

	long long update()
	{
		return (std::chrono::system_clock::now() - initial).count();
	}

	long long since()
	{
		return (std::chrono::system_clock::now() - beginning).count();
	}
private:
	//initial can be changed on reset (used to know for how long a music is playing), beginning never changes (used for debugging average fps)
	//update is relative to initial, since is relative to beginning
	std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> beginning, initial;
};

template<typename T>
inline void Utils::shiftAdd(T * arr, unsigned int size, T newValue)
{
	for (unsigned int i = size - 1; i > 0; i--)
	{
		arr[i] = arr[i - 1];
	}
	arr[0] = newValue;
}