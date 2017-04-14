#ifndef __WAV_H_
#define __WAV_H_

#include <cstdint>
#include <string>
#include <vector>
#include "fft.hpp"

enum CHANNEL { MON = 0, STEREO_L = 0, STEREO_R };

class WAV {
private:
	uint32_t chunkID;
	uint32_t chunkSize;
	uint32_t format;
	uint32_t sc1ID;
	uint32_t sc1Size;
	uint16_t audioFormat;
	uint16_t numChannels;
	uint32_t sampleRate;
	uint32_t byteRate;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
	uint32_t sc2ID;
	uint32_t sc2Size;
	uint16_t **sample;
	bool has_frame;
	int resolution;
	int sample_index;
	int windowLen;
	std::string name;
	std::vector< std::vector< double > > dft;
	int distr(int);

public:
	WAV(std::string filename);
	uint16_t getNumSamples() { return sc2Size / (2 * numChannels); }
	uint16_t getNumChannels() { return numChannels; }
	uint16_t get(int, int=0);
	void set(int, uint16_t, int=0);
	void setNumChannels(int);
	void save(std::string, std::string);
	void buildDFT(int, double);
	void nextFrame();
	bool hasFrame();
	std::vector< std::vector< double > > getFrame();
};

#endif
