#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <argp.h>
#include "TFile.h"
#include "TTree.h"
#include <TStopwatch.h>

using namespace std;

vector<string> split_line(string line, char delimiter);
vector<vector<string>> get_csv_metadata(fstream &file);
vector<vector<string>> get_csv_data(fstream &file);