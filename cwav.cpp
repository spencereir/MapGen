#include "cwav.h"
#include <iostream>
#include <fstream>

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
	ifstream::pos_type pos = in.tellg();
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
		sample[i] = (uint16_t*)malloc(sc2Size * (sizeof(uint16_t)));	
	}
	numSamples = 0;
	for (int i = 0; i < sc2Size; i++) {
		for (int j = 0; j < numChannels; j++) {
			in.read(c, 2);
			if (in.gcount() == 0) {
				break;
			}
			storeLE(c, sample[j][i]);
		}
		numSamples++;
	}
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
	int bytesWritten = 0;
	for (int i = 0; i < numSamples; i++) {
		for (int j = 0; j < numChannels; j++) {
			out.write(uint16_to_c_LE(sample[j][i]), 2);		
			bytesWritten += 2;
		}
		if (bytesWritten >= sc2Size)
			break;
	}
}

