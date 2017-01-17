#include "cwav.h"
#include <iostream>

int main(int argc, char** argv) {
	using namespace std;
	WAV W("song_5.wav");
	cout << "Done opening" << endl;
	cout << W.getNumSamples() << endl;
	W.setNumChannels(1);
	cout << "Done modifying" << endl;
	W.save("test4.wav");
	return 0;
}
