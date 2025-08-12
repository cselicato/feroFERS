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

struct stored_vars
{
    public:
    int N_boards;
    int max_hits;
    // metadata
    int board_mod;
    TString file_format;
    TString janus_rel;
    TString acq_mode;
    Int_t run;
    Int_t e_Nbins;
    UInt_t time_epoch;
    Double_t time_LSB;
    TString time_UTC;
    TString time_unit;
    // data
    Int_t hits;
    unsigned long long Trg_Id;
    TString data_type, ch_mask;
    Double_t TStamp;

    Int_t** LG;
    Int_t** HG;
    Int_t** counts;
    Double_t** ToA;
    Double_t** ToT;

    Double_t*** ToT_timing;
    Double_t*** ToA_timing;
    
    stored_vars(int N_boards_, int max_hits_): N_boards(N_boards_), max_hits(max_hits_)
    {
        LG = new Int_t*[N_boards];
        HG = new Int_t*[N_boards];
        counts = new Int_t*[N_boards];
        ToA = new Double_t*[N_boards];
        ToT = new Double_t*[N_boards];
        ToT_timing = new Double_t**[N_boards];
        ToA_timing = new Double_t**[N_boards];

        for (int i = 0; i < N_boards; i++) {
            LG[i] = new Int_t[64];
            HG[i] = new Int_t[64];
            counts[i] = new Int_t[64];
            ToA[i] = new Double_t[64];
            ToT[i] = new Double_t[64];

            ToT_timing[i] = new Double_t*[64];
            ToA_timing[i] = new Double_t*[64];

            for (int j = 0; j < 64; j++) {
                ToT_timing[i][j] = new Double_t[max_hits];
                ToA_timing[i][j] = new Double_t[max_hits];
            }
        }

    }

    int get_N_boards() const {return N_boards;}
    int get_max_hits() const {return max_hits;}

};


void is_valid_ind(int board,int ch,int N_boards);
vector<vector<string>> get_event(vector<vector<string>>& data, unsigned long long& r, unsigned long long& ev_start);
modes find_mode(const TString& str);
modes find_mode(uint8_t acq_mode);
TTree * make_branches_info(TTree * t, const TString& mode, stored_vars &v);
TTree * make_branches_data(TTree * t, const TString& mode, stored_vars &v);
TTree * make_info_tree(vector<vector<string>>& metadata, const TString& mode, stored_vars &v);
TTree * make_data_tree(vector<vector<string>>& data, const TString& mode, stored_vars &v);

