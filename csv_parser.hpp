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
#include "modes_helpers.hpp"

using namespace std;

vector<string> split_line(string line, char delimiter);
void is_consistent(int size, int exp_size);
void fill_data_var(vector<string> &row, modes &mode, stored_vars &v, int hit);
void fill_info_var(vector<vector<string>> &metadata, modes &mode, stored_vars &v);
int parse_csv(string inFile, TTree * tr_info,TTree * tr_data, stored_vars &v);