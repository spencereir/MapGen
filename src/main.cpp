#include "cwav.h"
#include <iostream>
#include <bits/stdc++.h>
#include <Windows.h>

using namespace std;

int sign(int x) {
	return (x < 0 ? -1 : 1);
}

int clip(int x, int l, int r) {
	if (x < l)
		return l;
	if (x > r)
		return r;
	return x;
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		std::cout << "Usage: MapGen <filename.wav> <background image>" << std::endl;
		return 1;
	}

	std::string filename = argv[1];
	std::string background = argv[2];

	std::cout << "Reading WAV file...\t";
	WAV W(filename);
	std::cout << "Done" << std::endl;

	std::cout << "Creating MP3...\t";
	W.save("tmp.mp3", "MP3");
	std::cout << "Done" << std::endl;

	double windowTime = 0.38;

	std::cout << "Analyzing WAV file...\t";
	int dftSize = 1024;
	W.buildDFT(dftSize, windowTime);
	std::cout << "Done" << std::endl;
	double thresh = 1.5e7;
	int t_ms = 0;

	ofstream out(filename.substr(0, filename.length()-4) + ".osu");

	out << "osu file format v14\n\n[General]\nAudioFilename: tmp.mp3\nAudioLeadIn: 0\nPreviewTime: 164471\nCountdown: 0\nSampleSet: Soft\nStackLeniency: 0.7\nMode: 3\nLetterboxInBreaks: 0\nSpecialStyle: 0\nWidescreenStoryboard: 0\n\n[Editor]\nBookmarks: 966,227777\nDistanceSpacing: 0.8\nBeatDivisor: 2\nGridSize: 32\nTimelineZoom: 0.6999998\n\n[Metadata]\nTitle:Unknown\nArtist:Unknown\nCreator:MapGen\nVersion:Normal\nSource:Unknown\n";
	out << "BeatmapID:" << rand() << "\nBeatmapSetID:" << rand() << "\n[Difficulty]\nHPDrainRate:7\nCircleSize:7\nOverallDifficulty:7\nApproachRate:5\nSliderMultiplier:1.4\nSliderTickRate:1\n\n[Events]\n//Background and Video events\n0,0,\"";
	out << background << "\",0,0\n//Break Periods\n//Storyboard Layer 0 (Background)\n//Storyboard Layer 1 (Fail)\n//Storyboard Layer 2 (Pass)\n//Storyboard Layer 3 (Foreground)\n//Storyboard Sound Samples\n[TimingPoints]\n956,329.67032967033,4,2,0,40,1,0\n119637,659.340659340659,4,2,0,40,1,0\n119637,-57.1428571428571,4,2,0,40,0,0\n161834,329.67032967033,4,2,0,40,1,0\n164471,-100,4,2,0,40,0,1\n206669,659.340659340659,4,2,0,40,1,0\n206669,-57.1428571428571,4,2,0,40,0,0\n[HitObjects]\n\n";

	std::cout << "Creating beatmap...\t";

	vector< bool > lastBar(7, false);
	vector< int > lastBin(7, 0);

	while (W.hasFrame()) {
		auto x = W.getFrame();
		if (x.empty()) {
			break;
		}
		vector< int > hi;

		for (int i = 0; i < x[0].size(); i++) {
			double k = 0;
			for (int j = 0; j < x.size(); j++) {
				k += x[j][i] / x.size();
			}
			if (k > thresh) {
				hi.push_back(i);
			}
		}

		vector< bool > bar(7), cbin(7);

		for (auto b : hi) {
			int bin = -1;
			for (int j = 0; j < 7; j++) {
				if (lastBar[j] && abs(b - lastBin[j]) <= 1) {
					bar[j] = 1;
					cbin[j] = b;
					bin = 1;
				} else if (lastBar[j] && abs(b - lastBin[j]) <= 20) {
					bar[clip(j + sign(b-lastBin[j]), 0, 6)] = 1;
					cbin[clip(j + sign(b-lastBin[j]), 0, 6)] = b;
					bin = 1;
				} else if (lastBar[j] && abs(b - lastBin[j]) <= 50) {
					bar[clip(j + 2*sign(b-lastBin[j]), 0, 6)] = 1;
					cbin[clip(j + 2*sign(b-lastBin[j]), 0, 6)] = b;
					bin = 1;
				}
			}
			if (bin == -1) {
				bin = (double)b / dftSize * 7;
				bar[bin] = 1;
				cbin[bin] = b;
			}
		}

		for (int i = 1; i < 8; i++) {
			if (bar[i]) {
				out << i*128 << ",192," << t_ms << ",1,0,0:0:0:0:\n";
			}
		}
		
		for (int i = 0; i < 8; i++) {
			lastBar[i] = bar[i];
			lastBin[i] = cbin[i];
		}

		W.nextFrame();
		t_ms += 1000 * windowTime;
	}
	std::cout << "Done" << std::endl;
	out.close();

	if (system(("zip " + filename.substr(0, filename.length() - 4) + ".osz tmp.mp3 " + filename.substr(0, filename.length()-4) + ".osu " + background).c_str()) == -1) {
		std::cout << "Error zipping file; manually create the " << filename.substr(0, filename.length() - 4) << ".osz file" << std::endl;
	}
	return 0;
}
