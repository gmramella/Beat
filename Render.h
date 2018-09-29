#pragma once
#include "libUnicornio.h"
#include "Utils.h"
#include "Wav.h"

static const Cor samplesColor = gray;
static const Cor amplitudesColor = cyan;
static const Cor centerColor = red;
static const Cor beatsColor = yellow;

class Render
{
public:
	static void RenderMono(unsigned int frameSize, unsigned int NumSamples, unsigned int frameFirst, unsigned int NumChannels, sample* samplesMono, float max, double* amplitudesMono, double highestAmplitudeMono, unsigned short scaleStep, Texto* scale, float binWidthMono);
	static void RenderStereo(unsigned int frameSize, unsigned int NumSamples, unsigned int frameFirst, unsigned int NumChannels, sample* samplesLeft, sample* samplesRight, float max, double* amplitudesLeft, double* amplitudesRight, double highestAmplitudeLeft, double highestAmplitudeRight, unsigned short scaleStep, Texto* scale, float binWidthStereo);
	static void RenderBeatsMono(double* beats, unsigned int numSubBands, double highestBeat);
	static void RenderBeatsStereo(double* beats, unsigned int numSubBands, double highestBeat);
	static void RenderDebug(std::string title, Wav* wav, double highestAmplitudeMono, double highestAmplitudeLeft, double highestAmplitudeRight, unsigned int chunkMono, unsigned int chunkStereo, unsigned int numBinsMono, float binWidthMono, unsigned int numBinsStereo, float binWidthStereo, unsigned int frameSize, double variance, float C, unsigned int numSubBands, unsigned int widthSubBands, unsigned int sizeOfHistory, unsigned int samplesPerSecond, unsigned int numHigher, bool wasBeat, double highestBeat, unsigned int frames, double seconds);
};