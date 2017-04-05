#include "cwav.h"
#include <iostream>
#include <fstream>
#include <cmath>
#define max(a,b) (a > b ? a : b)

unsigned char *c_to_uc(char *c) {
	unsigned char *a = (unsigned char*)malloc(sizeof(c));
	int n = sizeof(c) / sizeof(c[0]);
	for (int i = 0; i < n; i++)
		a[i] = (unsigned char)c[i];
	return a;
}

void store(char *a, uint32_t &target) {
	unsigned char *c = c_to_uc(a);
	target = (c[0] << 24) | (c[1] << 16) | (c[2] << 8) | c[3];
}

void storeLE(char *a, uint32_t &target) {
	unsigned char *c = c_to_uc(a);
	target = c[0] | (c[1] << 8) | (c[2] << 16) | (c[3] << 24);
}

void store(char *a, uint16_t &target) {
	unsigned char *c = c_to_uc(a);
	target = (c[0] << 8) | c[1];
}

void storeLE(char *a, uint16_t &target) {
	unsigned char *c = c_to_uc(a);
	target = c[0] | (c[1] << 8);
}

char *uint32_to_c(uint32_t t) {
	char *c = new char[4];
	c[0] = ((0xFF000000 & t) >> 24);
	c[1] = ((0x00FF0000 & t) >> 16);
	c[2] = ((0x0000FF00 & t) >> 8);
	c[3] = (0x000000FF & t);
	return c;
}

char *uint32_to_c_LE(uint32_t t) {
	char *c = new char[4];
	c[3] = ((0xFF000000 & t) >> 24);
	c[2] = ((0x00FF0000 & t) >> 16);
	c[1] = ((0x0000FF00 & t) >> 8);
	c[0] = (0x000000FF & t);
	return c;
}

char *uint16_to_c_(uint16_t t) {
	char *c = new char[2];
	c[0] = ((0xFF00 & t) >> 8);
	c[1] = (0x00FF & t);
	return c;
}

char *uint16_to_c_LE(uint16_t t) {
	char *c = new char[2];
	c[1] = ((0xFF00 & t) >> 8);
	c[0] = (0x00FF & t);
	return c;
}

WAV::WAV(std::string filename) {
	using namespace std;
	
	ifstream in(filename, ios::binary | ios::ate);
	//ifstream::pos_type pos = in.tellg();
	in.seekg(0, ios::beg);
	
	char *c = new char[4];
	in.read(c, 4);
	store(c, chunkID);
	if (chunkID != 0x52494646) {
		cerr << "No RIFF header" << endl;
		return;
	}	
	in.read(c, 4);
	storeLE(c, chunkSize);
	in.read(c, 4);
	store(c, format);
	if (format != 0x57415645) {
		cerr << "Not WAVE format" << endl;
		return;
	}
	in.read(c, 4);
	store(c, sc1ID);	
	if (sc1ID != 0x666d7420) {
		cerr << "fmt header misssing" << endl;
		return;
	}
	in.read(c, 4);
	storeLE(c, sc1Size);
	in.read(c, 2);
	storeLE(c, audioFormat);
	if (audioFormat != 1) {
		cerr << "Compressed WAV files not supported" << endl;
		return;
	}
	in.read(c, 2);
	storeLE(c, numChannels);
	sample = (uint16_t**)malloc(numChannels * sizeof(uint16_t*));
	in.read(c, 4);
	storeLE(c, sampleRate);		
	in.read(c, 4);
	storeLE(c, byteRate);
	in.read(c, 2);
	storeLE(c, blockAlign);
	in.read(c, 2);
	storeLE(c, bitsPerSample);
	in.read(c, 4);
	store(c, sc2ID);
	if (sc2ID != 0x64617461) {
		uint16_t extra_params = 0;
		storeLE(c, extra_params);
		if (extra_params != 0) {
			cerr << "Extra parameters not supported, they will be ignored" << endl;
		}
		in.read(c, 2);
		swap(c[0], c[2]);
		swap(c[1], c[3]);
		store(c, sc2ID);
		while (sc2ID != 0x64617461) {
			char *c2 = new char[4];
			in.read(c2, 1);
			swap(c[0], c[1]);
			swap(c[1], c[2]);
			swap(c[2], c[3]);
			c[3] = c2[0];
			store(c, sc2ID);
		}
	}
	in.read(c, 4);
	storeLE(c, sc2Size);
	for (int i = 0; i < numChannels; i++) {
		sample[i] = (uint16_t*)malloc((sc2Size / (2 * numChannels)) * (sizeof(uint16_t)));	
	}
	for (int i = 0; i < (int)sc2Size / (2 * numChannels); i++) {
		for (int j = 0; j < numChannels; j++) {
			in.read(c, 2);
			if (in.gcount() == 0) {
				break;
			}
			storeLE(c, sample[j][i]);
		}
	}
}

void WAV::buildDFT(int _resolution, double windowTime) {
	if (_resolution > windowTime * sampleRate) {
		std::cerr << "Error: window length too small for resolution" << std::endl;
		return;
	}
	resolution = _resolution;
	windowLen = windowTime * sampleRate;
	/*std::cerr << sc2Size / (2 * numChannels) << std::endl;
	D.clear();
	for (int i = 0; i < numChannels; i++) {
		std::vector< std::complex<double> > cSample(sc2Size / (2 * numChannels));
		for (int j = 0; j < sc2Size / (2 * numChannels); j++) {
			cSample[j] = std::complex<double>((double)sample[i][j], 0.0);
		}
		D.push_back(DFT(cSample, resolution, (int)(windowLen * sampleRate)));
	}
	std::cerr << "Done" << " " << windowLen * sampleRate<<std::endl;*/
	dft.resize(numChannels);
	for (int i = 0; i < numChannels; i++)
		dft[i].resize(resolution);
	sample_index = 0;
	has_frame = true;
	nextFrame();
}

int WAV::distr(int ind) {
	if (ind == windowLen - 1) {
		return resolution - 1;
	}
	//distr(0) = 0, distr(windowLen-1) = resolution-1, distr(x) is logarithmically scaled
	//distr(x) = log(1+x)
	//(R-1) log(1 + ind) / log(L) at L = 
	return (int)floor((double)resolution * log(ind+1) / log(windowLen+1));
}

void WAV::nextFrame() {
	if (!has_frame) {
		return;
	}
	if (sample_index + windowLen >= (int)sc2Size / (2 * numChannels)) {
		std::cerr << "No frames left" << std::endl;
		has_frame = false;
		return;
	}
	for (int i = 0; i < numChannels; i++) {
		std::vector< double > inReal(windowLen, 0.0);
		std::vector< double > inImag(windowLen, 0.0);
		for (int j = 0; j < windowLen; j++) {
			inReal[j] = (double)sample[i][sample_index + j];
		}
		Fft::transform(inReal, inImag);
		for (int j = 0; j < resolution; j++) {
			dft[i][j] = 0;
		}
		for (int j = 1; j < windowLen; j++) {
			dft[i][distr(j)] = max(dft[i][distr(j)], sqrt(pow(inReal[j], 2.0) + pow(inImag[j], 2.0)));
		}
	}
	sample_index += windowLen;
}

bool WAV::hasFrame() {
	return has_frame;
}

std::vector< std::vector< double > > WAV::getFrame() {
	std::vector< std::vector< double > > ret;
	for (int i = 0; i < numChannels; i++) {
		std::vector< double > c;
		for (int j = 0; j < resolution; j++) {
			c.push_back(dft[i][j]);
		}
		ret.push_back(c);
	}
	return ret;
}

uint16_t WAV::get(int i, int chan) {
	return sample[chan][i];
}

void WAV::set(int i, uint16_t val, int chan) {
	sample[chan][i]= val;	
}

void WAV::save(std::string target) {
	using namespace std;
	
	ofstream out(target, ios::binary);
	
	out.write(uint32_to_c(chunkID), 4);
	out.write(uint32_to_c_LE(chunkSize), 4);
	out.write(uint32_to_c(format), 4);
	out.write(uint32_to_c(sc1ID), 4);
	out.write(uint32_to_c_LE(sc1Size), 4);
	out.write(uint16_to_c_LE(audioFormat), 2);
	out.write(uint16_to_c_LE(numChannels), 2);
	out.write(uint32_to_c_LE(sampleRate), 4);
	out.write(uint32_to_c_LE(byteRate), 4);
	out.write(uint16_to_c_LE(blockAlign), 2);
	out.write(uint16_to_c_LE(bitsPerSample), 2);
	out.write(uint32_to_c(sc2ID), 4);
	out.write(uint32_to_c_LE(sc2Size), 4);
	for (int i = 0; i < (int)sc2Size / (2 * numChannels); i++) {
		for (int j = 0; j < numChannels; j++) {
			out.write(uint16_to_c_LE(sample[j][i]), 2);		
		}
	}
}

void WAV::setNumChannels(int chan) { 
	sc2Size = (numChannels==1?(2*sc2Size):sc2Size/2); 
	numChannels = chan;
	if (chan == 2) {
		sample = (uint16_t**)realloc(sample, 2*sizeof(uint16_t*));	
		sample[STEREO_R] = (uint16_t*)malloc((sc2Size / (2 * numChannels)) * (sizeof(uint16_t)));
	} else {
		sample = (uint16_t**)realloc(sample, sizeof(uint16_t*));
	}
	for (int i = 0; i < (int)sc2Size / (2 * numChannels); i++) {
		if (chan == 1) {
			sample[MONO][i] = (sample[STEREO_L][i] >> 1) + (sample[STEREO_R][i] >> 1);	
		} else {
			sample[STEREO_L][i] = sample[STEREO_R][i] = sample[MONO][i];
		}
	}
}


