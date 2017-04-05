#include "cwav.h"
#include <iostream>
#include <bits/stdc++.h>
#include <Windows.h>

using namespace std;
int main() {
	ios::sync_with_stdio(false);
	cin.tie(0);
	WAV W("aaaa.wav");
	system("aaaa.wav");
	double windowTime = 0.15;
	const int height = 65, width= 300;
	W.buildDFT(width, windowTime);
	double m = 4e7;
	clock_t last_time = clock();
	while (W.hasFrame()) {
		last_time = clock();
		auto x = W.getFrame();
		if (x.empty()) {
			cerr << "wtf" << endl;
		}
		vector< double > v = x[0];
		char out[height][width];
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				out[i][j] = ' ';
			}
		}
		for (int i = 0; i < width; i++) {
			//m = max(m, v[i]);
			int barHeight = min(height,(int)(height * v[i] / m));
			for (int j = 0, l = height-1; j < barHeight; j++, l--) {
				out[l][i] = '#';
			}
		}
		system("cls");
		for (int i = 0; i < height; i++) {
			for (int j = 0; j< width; j++) {
				cout << out[i][j];
			}
			cout << endl;
		}
		W.nextFrame();
		while ((double)(clock() - last_time)/CLOCKS_PER_SEC < windowTime - 0.0045) {
		}
	}
	return 0;
}
