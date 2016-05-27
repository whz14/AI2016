#include "dirent.h"
#include <iostream>
#include <stdio.h>
#include <ctime>
#include <cassert>
#include <fstream>
#include <algorithm>
#include <string>
#include <set>
#include <map>
#include <vector>
#define ATHEISM 0
#define CRYPT 1
#define GUNS 2
#define HARDWARE 3
using namespace std;
const double eps = 1e-7;

int length[4];
map<string, double[4]> p;			// p["sth"][ATHEISM] <=> p("sth"|atheism);
double p_k[4];						// p(atheism), p(crypt), p(guns), p(hardware);

map<string, unsigned int> text[4];	// text[ATHEISM]["sth"] <=> nj; n = sum(text[ATHEISM]["i"])
set<string> vocabulary;

vector<string> getFiles(char* dirName) {
	vector<string> res;
	DIR *dir;
	dirent *ent;
	if((dir = opendir(dirName)) != NULL) {
		while((ent = readdir(dir)) != NULL) {
			if(ent->d_type == DT_REG) {	// regular file
				res.push_back(string(dirName) + string(ent->d_name));
			}
		}
		closedir(dir);
	}
	else {
		/* could not open directory */
		perror("could not open such directory");
		exit(EXIT_FAILURE);
	}
	return res;
}

inline bool formlize(string& str) {
/*
* formlize the string, remove the ',', '"', '.', 'a', 'the' that kind of thing.
*/	
	if(str == "!" || str == "." || str == "," || str == "\"" || str == "\\") {	//	meaningless punctuation
		//cout << str << endl;
		return 0;
	}
	if(str[str.size() - 1] == ',' || str[str.size() - 1] == '.' || str[str.size() - 1] == '!' || str[str.size() - 1] == '\"') {
		str = str.substr(0, str.size() - 1);
	}
	if(str[0] == '\'' || str[0] == '\"' || str[0] == '\\') {
		str = str.substr(1, str.size() - 1);
	}
	return 1;
}

void buildText(int type, const vector<string> files) {
/*
 * build text[type] together with vocabulary
 */
	assert(0 <= type&& type <= 3);
	for(auto it = files.begin(); it != files.end(); ++it) {
		ifstream fin(*it);
		string tmp;
		while(fin >> tmp) {
			//cout << tmp << endl;
			if(formlize(tmp)) {
				++text[type][tmp];
				++length[type];
				vocabulary.insert(tmp);
			}
		}
	}
}

void learn(const vector<string>& athFile, const vector<string>& cryFile, const vector<string>& gunFile, const vector<string>& hardwareFile) {
	/*auto athFile = getFiles("./data/c1_atheism/");
	auto cryFile = getFiles("./data/c2_sci.crypt/");
	auto gunFile = getFiles("./data/c3_talk.politics.guns/");
	auto hardwareFile = getFiles("./data/c4_comp.sys.mac.hardware/");
*/
	int ttl = athFile.size() + cryFile.size() + gunFile.size() + hardwareFile.size();
	p_k[ATHEISM] = 1.0*athFile.size() / ttl;
	p_k[CRYPT] = 1.0*cryFile.size() / ttl;
	p_k[GUNS] = 1.0*gunFile.size() / ttl;
	p_k[HARDWARE] = 1.0*hardwareFile.size() / ttl;

	buildText(ATHEISM, athFile);
	buildText(CRYPT, cryFile);
	buildText(GUNS, gunFile);
	buildText(HARDWARE, hardwareFile);

	for(auto it = vocabulary.begin(); it != vocabulary.end(); ++it) {
		for(int i = 0; i < 4; ++i) {
			if(text[i].find(*it) != text[i].end()) {
				p[*it][i] = log((1 + text[i][*it]) * 1.0) - log(length[i] + vocabulary.size());
				//if(p[*it][i] == 0) {
				//	cout << "holy shit\n";
				//}
			}
			else {
				p[*it][i] = -log(length[i] + vocabulary.size());
			}
		}
	}
}

int classify(const string& doc) {
	double p4[4] = {0, 0, 0, 0};
	ifstream fin(doc);
	if(!fin) {
		cout << "cannot open file " << doc << " when classfying\n";
		exit(-1);
	}
	string tmp;
	while(fin >> tmp) {
		if(!formlize(tmp)) {	// meaningless punctuations
			continue;
		}
		if(vocabulary.find(tmp) == vocabulary.end()) {	// not exist in the vocabulary
			continue;
		}
		for(int i = 0; i < 4; ++i) {
			p4[i] += p[tmp][i];
		}
	}
	//if(p4[0] == && p4[1] < eps && p4[2] < eps && p4[3] < eps) {	// p all equals to 0;
	//	cout << "the p4[] are " << p4[0] << " " << p4[1] << " " << p4[2] << " " << p4[3] << endl;
	//	return rand() % 4;
	//}
	double max = p4[0]; unsigned int res = 0;
	for(int i = 1; i < 4; ++i) {
		if(p4[i] > max) {
			max = p4[i]; res = i;
		}
	}
	return res;
}

inline void classifyAll(const vector<string>& testFiles, ostream& out) {
	for(auto it = testFiles.begin(); it != testFiles.end(); ++it) {
		out << "the file is,\t" << *it << ",\t" << classify(*it) << endl;
	}
}

void crossVali(int zhe, const vector<string>& athFile, const vector<string>& cryFile, const vector<string>& gunFile, const vector<string>& hardwareFile ) {
	for(int i = 0; i < zhe; ++i) {	// zhe 
	vector<string> athTra, cryTra, gunTra, hardTra;
	vector<string> athTes, cryTes, gunTes, hardTes;
	for(int j = 0; j < zhe; j++) {
		cout << (athFile.begin() + athFile.size() / zhe * (j + 1) == athFile.end()) << endl;
		if(i == j) {	// this 1/zhe data used as testing data
			athTes.insert(athTes.begin(), athFile.begin() + athFile.size() / zhe * j, athFile.begin() + athFile.size() / zhe * (j + 1));// &athFile[athFile.size() / zhe * (j + 1)]);
			cryTes.insert(cryTes.begin(), cryFile.begin() + cryFile.size() / zhe * j, cryFile.begin() + cryFile.size() / zhe * (j + 1));
			gunTes.insert(gunTes.begin(), gunFile.begin() + gunFile.size() / zhe * j, gunFile.begin() + gunFile.size() / zhe * (j + 1));
			hardTes.insert(hardTes.begin(), hardwareFile.begin() + hardwareFile.size() / zhe * j, hardwareFile.begin() + hardwareFile.size() / zhe * (j + 1));
		}
		else {			// used as training data
			athTra.insert(athTra.begin(), athFile.begin() + athFile.size() / zhe * j, athFile.begin() + athFile.size() / zhe * (j + 1));
			cryTra.insert(cryTra.begin(), cryFile.begin() + cryFile.size() / zhe * j, cryFile.begin() + cryFile.size() / zhe * (j + 1));
			gunTra.insert(gunTra.begin(), gunFile.begin() + gunFile.size() / zhe * j, gunFile.begin() + gunFile.size() / zhe * (j + 1));
			hardTra.insert(hardTra.begin(), hardwareFile.begin() + hardwareFile.size() / zhe * j, hardwareFile.begin() + hardwareFile.size() / zhe * (j + 1));
		}
	}
	learn(athTra, cryTra, gunTra, hardTra);
	
	ofstream athout("ath_res.csv", ios::app);
	athout << "the type is atheism with " << zhe << "parts\n";
	classifyAll(athTes, athout);
		ofstream cryout("cry_res.csv", ios::app);
	cryout << "the type is crypt with " << zhe << "parts\n";
	classifyAll(cryTes, cryout);
	
	ofstream gunout("gun_res.csv", ios::app);
	gunout << "the type is guns with " << zhe << "parts\n";
	classifyAll(gunTes, gunout);
	
	ofstream hardout("hardware_res.csv", ios::app);
	hardout << "the type is hardware with " << zhe << "parts\n";
	classifyAll(hardTes, hardout);
	}

}

int main() {
	srand(time(NULL));
	auto athFile = getFiles("./data/c1_atheism/");
	auto cryFile = getFiles("./data/c2_sci.crypt/");
	auto gunFile = getFiles("./data/c3_talk.politics.guns/");
	auto hardwareFile = getFiles("./data/c4_comp.sys.mac.hardware/");

	random_shuffle(athFile.begin(), athFile.end());
	random_shuffle(cryFile.begin(), cryFile.end());
	random_shuffle(gunFile.begin(), gunFile.end());
	random_shuffle(hardwareFile.begin(), hardwareFile.end());

	crossVali(5, athFile, cryFile, gunFile, hardwareFile);
	crossVali(10, athFile, cryFile, gunFile, hardwareFile);
	// system("pause");
	return 0;
}