#ifndef __WAV_H_
#define __WAV_H_

#include <cstdint>
#include <string>

enum CHANNEL { MONO = 0, STEREO_L = 0, STEREO_R };

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
	uint32_t numSamples;
public:
	WAV(std::string filename);
	uint16_t getNumSamples() { return numSamples; }
	uint16_t getNumChannels() { return numChannels; }
	uint16_t get(int, int=0);
	void set(int, uint16_t, int=0);
	void setNumChannels(int n) { numChannels = n; }
	void save(std::string target);
};

#endif
