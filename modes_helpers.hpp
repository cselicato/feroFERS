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

// class to contain all the variables stored in the TTree
class stored_vars
{
    public:
    int N_boards;
    int max_hits;
    // metadata
    uint16_t board_mod;
    TString file_format;
    TString janus_rel;
    TString acq_mode;
    uint8_t bin_acq_mode;
    uint16_t run;
    uint16_t e_Nbins;
    uint64_t time_epoch;
    Double_t time_LSB;
    TString time_UTC;
    TString time_unit;
    // data
    Int_t hits;
    uint64_t Trg_Id;
    uint8_t data_type;
    uint64_t ch_mask;
    Double_t TStamp;

    // only the 2D and 3D variables need to have a different type than the one read in the binary file
    int32_t** LG;
    int32_t** HG;
    int64_t** counts; // warning: (in principle) there could be loss of data...
    float** ToA;         // does float need to change too?
    float** ToT;
    float*** ToA_timing;
    float*** ToT_timing;
    
    stored_vars(int N_boards_, int max_hits_): N_boards(N_boards_), max_hits(max_hits_)
    {
        LG = new int32_t*[N_boards];
        HG = new int32_t*[N_boards];
        counts = new int64_t*[N_boards];
        ToA = new float*[N_boards];
        ToT = new float*[N_boards];
        ToA_timing = new float**[N_boards];
        ToT_timing = new float**[N_boards];
        
        for (int i = 0; i < N_boards; i++) {
            LG[i] = new int32_t[64];
            HG[i] = new int32_t[64];
            counts[i] = new int64_t[64];
            ToA[i] = new float[64];
            ToT[i] = new float[64];

            ToA_timing[i] = new float*[64];
            ToT_timing[i] = new float*[64];

            for (int j = 0; j < 64; j++) {
                ToA_timing[i][j] = new float[max_hits];
                ToT_timing[i][j] = new float[max_hits];

            }
        }

    }

    int get_N_boards() const {return N_boards;}
    int get_max_hits() const {return max_hits;}

};



// class for the variables used in the parsing of the .dat files
// (that are not already in classes)
class read_vars
{
    public:
    uint16_t LG;
    uint16_t HG;
    float ToA_ns;
    float ToT_ns;
    uint32_t ToA_LSB;
    uint16_t ToT_LSB;
    uint8_t ch_ID;
    uint64_t counts;
};


void is_valid_ind(int board,int ch,int N_boards);
vector<vector<string>> get_event(vector<vector<string>>& data, unsigned long long& r, unsigned long long& ev_start);
modes find_mode(const TString& str);
modes find_mode(uint8_t acq_mode);
template<typename T> T** reset(T** c, stored_vars& v);
template<typename T> T*** reset(T*** c, stored_vars& v);
TTree * make_branches_info(TTree * t, const TString& mode, stored_vars &v);
TTree * make_branches_data(TTree * t, const TString& mode, stored_vars &v);
TTree * make_info_tree(vector<vector<string>>& metadata, const TString& mode, stored_vars &v);
TTree * make_data_tree(vector<vector<string>>& data, const TString& mode, stored_vars &v);

