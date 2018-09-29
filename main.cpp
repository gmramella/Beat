#pragma once
#include "libUnicornio.h"
#include "fftw3.h"
#include "AudioManager.h"
#include "Utils.h"
#include "Render.h"
#include "Wav.h"

int main(int argc, char* argv[])
{
	uniInicializar(800, 600, false);

	gRecursos.carregarFonte("font1", "./fonte_padrao.ttf", 32);
	gRecursos.carregarFonte("font2", "./fonte_padrao.ttf", 8);
	Texto time;
	time.setFonte("font1");
	time.setCor(blue);
	time.setAlinhamento(TEXTO_CENTRALIZADO);

	unsigned short scaleStep = 32;
	Texto* scale;
	scale = new Texto[gJanela.getLargura() / scaleStep];
	for (unsigned int i = 0; i < gJanela.getLargura() / scaleStep; i++)
	{
		scale[i].setFonte("font2");
		scale[i].setCor(blue);
		scale[i].setAlinhamento(TEXTO_CENTRALIZADO);
	}

	const char* path = "./Musics/stereo8.wav";
	std::string title(path);
	title = title.substr(title.find_last_of("/\\") + 1);
	gRecursos.carregarMusica("audio1", path);
	Wav* wav = AudioManager::loadWAV(path);

	if (wav == NULL)
	{
		gRecursos.descarregarTudo();
		uniFinalizar();
		return EXIT_FAILURE;
	}
	wav->printHeader();
	/*wav->writeHeader();
	wav->writeSamples();*/

	sample* samplesMono = NULL;
	sample* samplesLeft = NULL;
	sample* samplesRight = NULL;
	double* amplitudesMono = NULL;
	double* amplitudesLeft = NULL;
	double* amplitudesRight = NULL;
	double* phasesMono = NULL;
	double* phasesLeft = NULL;
	double* phasesRight = NULL;
	double highestAmplitudeMono = 0;
	double highestAmplitudeLeft = 0;
	double highestAmplitudeRight = 0;
	unsigned int chunkMono = (int)pow(2, 10 + 6 * !RENDER_ON);//fps increases if it does not need to render
	unsigned int chunkStereo = chunkMono / 2;
	sample* windowMono = new sample[chunkMono];
	sample* windowLeft = new sample[chunkStereo];
	sample* windowRight = new sample[chunkStereo];
	unsigned int numBinsMono = chunkMono / 2;
	float binWidthMono = 1.0 * wav->SampleRate / 2 / numBinsMono;
	unsigned int numBinsStereo = chunkStereo / 2;
	float binWidthStereo = 1.0 * wav->SampleRate / 2 / numBinsStereo;
	unsigned int frameSize;

	if (wav->NumChannels == 1)
	{
		cout << "NumBins " << numBinsMono << endl;
		cout << "BinWidth " << binWidthMono << endl;
		samplesMono = new sample[wav->NumSamples];
		amplitudesMono = new double[chunkMono / 2];
		phasesMono = new double[chunkMono / 2];

		for (int i = 0; i < wav->NumSamples; i++)
		{
			samplesMono[i] = wav->IntData[i];
		}
	}
	else
	{
		cout << "NumBins " << numBinsStereo << endl;
		cout << "BinWidth " << binWidthStereo << endl;
		samplesLeft = new sample[wav->NumSamples];
		samplesRight = new sample[wav->NumSamples];
		amplitudesLeft = new double[chunkStereo / 2];
		amplitudesRight = new double[chunkStereo / 2];
		phasesLeft = new double[chunkStereo / 2];
		phasesRight = new double[chunkStereo / 2];

		for (int i = 0; i < 2 * wav->NumSamples; i++)
		{
			if (i % 2 == 0)
			{
				samplesLeft[i / 2] = wav->IntData[i];
			}
			else
			{
				samplesRight[i / 2] = wav->IntData[i];
			}
		}
	}

	//beat-------------------------------------------------------------------------------------------------
	double variance = 0.0;
	float C = 1.3;
	unsigned int numSubBands;
	unsigned int widthSubBands;
	unsigned int sizeOfHistory;
	unsigned int samplesPerSecond;
	unsigned int numHigher = 0.0;
	bool wasBeat = false;
	double highestBeat = 0.0;

	if (wav->NumChannels == 1)
	{
		numSubBands = chunkMono / 2;
		widthSubBands = chunkMono / 2 / numSubBands;
		sizeOfHistory = (int)floor(wav->SampleRate / chunkMono);
		samplesPerSecond = sizeOfHistory * chunkMono;
	}
	else
	{
		numSubBands = chunkStereo / 2;
		widthSubBands = chunkStereo / 2 / numSubBands;
		sizeOfHistory = (int)floor(wav->SampleRate / chunkStereo);
		samplesPerSecond = sizeOfHistory * chunkStereo;
	}

	double** history = new double*[numSubBands];
	for (unsigned int i = 0; i < numSubBands; i++)
		history[i] = new double[sizeOfHistory];
	double* ratios = new double[numSubBands];
	double* average = new double[numSubBands];

	ofstream preFile("#pre.txt");
	ofstream posFile("#pos.txt");
	if (wav->NumChannels == 1)
	{
		for (unsigned int j = 0; j < sizeOfHistory; j++)
		{
			preFile << "History " << j << "\n";
			Utils::hamming(windowMono, samplesMono, j * chunkMono, chunkMono);
			Utils::fft(amplitudesMono, phasesMono, windowMono, chunkMono);
			for (unsigned int k = 0; k < chunkMono / 2; k++)
				preFile << (amplitudesMono[k] * amplitudesMono[k]) << " + ";
			preFile << "\n";
			for (unsigned int i = 0; i < numSubBands; i++)
			{
				history[i][j] = Utils::energy(amplitudesMono, i * widthSubBands, widthSubBands);
				preFile << "Energy " << history[i][j] << "\n";
			}
		}

		for (unsigned int i = 0; i < numSubBands; i++)
		{
			average[i] = 0;
			for (unsigned int j = 0; j < sizeOfHistory; j++)
			{
				posFile << i << " " << j << " " << history[i][j] << "\n";
				average[i] += history[i][j];
			}
			average[i] *= 1 / (float)sizeOfHistory;
		}
	}
	else
	{
		for (unsigned int j = 0; j < sizeOfHistory; j++)
		{
			preFile << "History " << j << "\n";
			Utils::hamming(windowLeft, samplesLeft, j * chunkStereo, chunkStereo);
			Utils::hamming(windowRight, samplesRight, j * chunkStereo, chunkStereo);
			Utils::fft(amplitudesLeft, phasesLeft, windowLeft, chunkStereo);
			Utils::fft(amplitudesRight, phasesRight, windowRight, chunkStereo);
			for (unsigned int k = 0; k < chunkStereo / 2; k++)
				preFile << (amplitudesLeft[k] * amplitudesLeft[k]) << " + " << (amplitudesRight[k] * amplitudesRight[k]) << " + ";
			preFile << "\n";
			for (unsigned int i = 0; i < numSubBands; i++)
			{
				history[i][j] = Utils::energy(amplitudesLeft, amplitudesRight, i * widthSubBands, widthSubBands);
				preFile << "Energy " << history[i][j] << "\n";
			}
		}

		for (unsigned int i = 0; i < numSubBands; i++)
		{
			average[i] = 0;
			for (unsigned int j = 0; j < sizeOfHistory; j++)
			{
				posFile << i << " " << j << " " << history[i][j] << "\n";
				average[i] += history[i][j];
			}
			average[i] *= 1 / (float)sizeOfHistory;
		}
	}
	preFile.close();
	posFile.close();
	//-----------------------------------------------------------------------------------------------------
	float max = (float)pow(2, wav->BitsPerSample - 1);
	Timer timer;
	timer.start();
	gMusica.tocar("audio1", false);
	unsigned int frames = 0;

	ofstream averageFile("#average.txt");
	ofstream ratiosFile("#ratios.txt");
	while (!gTeclado.soltou[TECLA_ESC] && !gEventos.sair)
	{
		uniIniciarFrame();
		long double seconds = timer.update() / 1e9;

		unsigned int frameFirst = (int)round(wav->SampleRate * seconds);
		if (frameFirst <= wav->NumSamples)
		{
			if (wav->NumChannels == 1)
			{
				if (frameFirst <= wav->NumSamples - chunkMono)
				{
					frameSize = chunkMono;
				}
				else
				{
					frameSize = wav->NumSamples - frameFirst;
				}
				Utils::hamming(windowMono, samplesMono, frameFirst, frameSize);
				Utils::fft(amplitudesMono, phasesMono, windowMono, frameSize);

				if (RENDER_ON || DEBUG_ON)
				{
					highestAmplitudeMono = amplitudesMono[2];
					for (unsigned int i = 3; i < frameSize / 2; i++) {
						if (highestAmplitudeMono < amplitudesMono[i]) {
							highestAmplitudeMono = amplitudesMono[i];
						}
					}
					if (highestAmplitudeMono == 0) highestAmplitudeMono = 1;
				}

				//beat-------------------------------------------------------------------------------------------------
				for (unsigned int i = 0; i < numSubBands; i++)
				{
					Utils::shiftAdd(history[i], sizeOfHistory, Utils::energy(amplitudesMono, i * widthSubBands, widthSubBands));

					average[i] = 0.0;
					for (unsigned int j = 0; j < sizeOfHistory; j++)
					{
						average[i] += history[i][j];
					}
					average[i] *= 1 / (float)sizeOfHistory;
				}

				variance = 0.0;
				for (unsigned int i = 0; i < numSubBands; i++)
				{
					for (unsigned int j = 0; j < sizeOfHistory; j++)
						variance += pow(history[i][j] - average[i], 2);
					variance *= 1 / (float)sizeOfHistory;
				}
				C = 1.4142857 - 0.0000015 * variance;

				for (unsigned int i = 0; i < numSubBands; i++)
				{
					if (history[i][0] > C * average[i])
						ratios[i] = history[i][0] / average[i];
					else
						ratios[i] = 0;

					averageFile << i << " " << average[i] << "\n";
					ratiosFile << i << " " << ratios[i] << "\n";
				}

				numHigher = 0;
				for (unsigned int i = 0; i < numSubBands; i++)
				{
					numHigher += (int)(ratios[i] != 0);
				}

				wasBeat = ((float)numHigher >= 0.75 * numSubBands);

				if (wasBeat)
				{
					cout << to_string(numHigher / (float)numSubBands) << endl;
				}

				if (RENDER_ON)
				{
					highestBeat = ratios[0];
					for (unsigned int i = 1; i < numSubBands; i++)
					{
						if (highestBeat < ratios[i]) {
							highestBeat = ratios[i];
						}
					}
					if (highestBeat == 0) highestBeat = 1;
					Render::RenderBeatsMono(ratios, numSubBands, highestBeat);
				}
				//-----------------------------------------------------------------------------------------------------

				if (RENDER_ON)
				{
					Render::RenderMono(frameSize, wav->NumSamples, frameFirst, wav->NumChannels, samplesMono, max, amplitudesMono, highestAmplitudeMono, scaleStep, scale, binWidthMono);
				}
			}
			else
			{
				if (frameFirst <= wav->NumSamples - chunkStereo)
				{
					frameSize = chunkStereo;
				}
				else
				{
					frameSize = wav->NumSamples - frameFirst;
				}
				Utils::hamming(windowLeft, samplesLeft, frameFirst, frameSize);
				Utils::hamming(windowRight, samplesRight, frameFirst, frameSize);
				Utils::fft(amplitudesLeft, phasesLeft, windowLeft, frameSize);
				Utils::fft(amplitudesRight, phasesRight, windowRight, frameSize);

				if (RENDER_ON || DEBUG_ON)
				{
					highestAmplitudeLeft = amplitudesLeft[2];
					highestAmplitudeRight = amplitudesRight[2];
					for (unsigned int i = 3; i < frameSize / 2; i++) {
						if (highestAmplitudeLeft < amplitudesLeft[i]) {
							highestAmplitudeLeft = amplitudesLeft[i];
						}
						if (highestAmplitudeRight < amplitudesRight[i]) {
							highestAmplitudeRight = amplitudesRight[i];
						}
					}
					if (highestAmplitudeLeft == 0) highestAmplitudeLeft = 1;
					if (highestAmplitudeRight == 0) highestAmplitudeRight = 1;
				}

				//beat-------------------------------------------------------------------------------------------------
				for (unsigned int i = 0; i < numSubBands; i++)
				{
					Utils::shiftAdd(history[i], sizeOfHistory, Utils::energy(amplitudesLeft, amplitudesRight, i * widthSubBands, widthSubBands));

					average[i] = 0.0;
					for (unsigned int j = 0; j < sizeOfHistory; j++)
					{
						average[i] += history[i][j];
					}
					average[i] *= 1 / (float)sizeOfHistory;
				}

				variance = 0.0;
				for (unsigned int i = 0; i < numSubBands; i++)
				{
					for (unsigned int j = 0; j < sizeOfHistory; j++)
						variance += pow(history[i][j] - average[i], 2);
					variance *= 1 / (float)sizeOfHistory;
				}
				C = 1.4142857 - 0.0000015 * variance;

				for (unsigned int i = 0; i < numSubBands; i++)
				{
					if (history[i][0] > C * average[i])
						ratios[i] = history[i][0] / average[i];
					else
						ratios[i] = 0;

					averageFile << i << " " << average[i] << "\n";
					ratiosFile << i << " " << ratios[i] << "\n";
				}

				numHigher = 0;
				for (unsigned int i = 0; i < numSubBands; i++)
				{
					numHigher += (int)(ratios[i] != 0);
				}

				wasBeat = ((float)numHigher >= 0.75 * numSubBands);

				if (wasBeat)
				{
					cout << to_string(numHigher / (float)numSubBands) << endl;
				}
				
				if (RENDER_ON)
				{
					highestBeat = ratios[0];
					for (unsigned int i = 1; i < numSubBands; i++)
					{
						if (highestBeat < ratios[i]) {
							highestBeat = ratios[i];
						}
					}
					if (highestBeat == 0) highestBeat = 1;
					Render::RenderBeatsStereo(ratios, numSubBands, highestBeat);
				}
				//-----------------------------------------------------------------------------------------------------

				if (RENDER_ON)
				{
					Render::RenderStereo(frameSize, wav->NumSamples, frameFirst, wav->NumChannels, samplesLeft, samplesRight, max, amplitudesLeft, amplitudesRight, highestAmplitudeLeft, highestAmplitudeRight, scaleStep, scale, binWidthStereo);
				}
			}
		}

		time.setString(to_string(seconds));
		time.desenhar(gJanela.getLargura() / 2, gJanela.getAltura() / 2);

		if (LOOP && !gMusica.estaTocando())
		{
			//restart audio and timer
			gMusica.tocar("audio1", false);
			timer.reset();
		}

		if (DEBUG_ON)
		{
			Render::RenderDebug(title, wav, highestAmplitudeMono, highestAmplitudeLeft, highestAmplitudeRight, chunkMono, chunkStereo, numBinsMono, binWidthMono, numBinsStereo, binWidthStereo, frameSize, variance, C, numSubBands, widthSubBands, sizeOfHistory, samplesPerSecond, numHigher, wasBeat, highestBeat, ++frames, timer.since() / 1e9);
		}

		uniTerminarFrame();
	}
	averageFile.close();
	ratiosFile.close();

	if (samplesMono != NULL) delete[] samplesMono;
	if (samplesLeft != NULL) delete[] samplesLeft;
	if (samplesRight != NULL) delete[] samplesRight;
	if (amplitudesMono != NULL) delete[] amplitudesMono;
	if (amplitudesLeft != NULL) delete[] amplitudesLeft;
	if (amplitudesRight != NULL) delete[] amplitudesRight;
	if (phasesMono != NULL) delete[] phasesMono;
	if (phasesLeft != NULL) delete[] phasesLeft;
	if (phasesRight != NULL) delete[] phasesRight;
	highestAmplitudeMono = 0;
	highestAmplitudeLeft = 0;
	highestAmplitudeRight = 0;
	chunkMono = 0;
	chunkStereo = 0;
	if (windowMono != NULL) delete[] windowMono;
	if (windowLeft != NULL) delete[] windowLeft;
	if (windowRight != NULL) delete[] windowRight;
	numBinsMono = 0;
	binWidthMono = 0;
	numBinsStereo = 0;
	binWidthStereo = 0;
	frameSize = 0;

	//beat-------------------------------------------------------------------------------------------------
	variance = 0.0;
	C = 0;
	numSubBands = 0;
	widthSubBands = 0;
	sizeOfHistory = 0;
	samplesPerSecond = 0;
	numHigher = 0.0;
	wasBeat = false;
	highestBeat = 0.0;

	if (history != NULL)
	{
		for (int i = 0; i < numSubBands; ++i)
			delete[] history[i];
		if (history != NULL) delete[] history;
	}
	if (ratios != NULL) delete[] ratios;
	if (average != NULL) delete[] average;
	//-----------------------------------------------------------------------------------------------------

	delete[] scale;
	gRecursos.descarregarTudo();
	uniFinalizar();

	return 0;
}