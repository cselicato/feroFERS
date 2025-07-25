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

enum class modes {
    Spectroscopy,
    Spect_Timing,
    Timing,
    Counting
};


void is_valid_ind(int board,int ch,int N_boards);
vector<vector<string>> get_event(vector<vector<string>>& data, unsigned long long& r, unsigned long long& ev_start);
TTree * make_info_tree(vector<vector<string>>& metadata, const TString& mode);
TTree * make_data_tree(vector<vector<string>>& data, const TString& mode, int N_boards);

