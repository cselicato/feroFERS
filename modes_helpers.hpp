#define NBOARDS 4
#define NCHANNELS 64
#define MAXHITS 100

#include <stdio.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <argp.h>
#include "TFile.h"
#include "TTree.h"
#include <TStopwatch.h>
#include "TTimeStamp.h"

using namespace std;

enum class modes {
    Spectroscopy,
    Spect_Timing,
    Timing,
    Counting
};

class stored_vars
{
    public:
    UInt_t n_boards = NBOARDS;
    UInt_t n_channels = NCHANNELS;
    UInt_t max_hits = MAXHITS;
    // metadata
    uint16_t board_mod;
    TString file_format;
    TString janus_rel;
    TString acq_mode;
    uint16_t run;
    uint16_t e_Nbins;
    uint64_t time_epoch;
    Double_t time_conv;
    TTimeStamp * time_UTC;
    TString time_unit;
    // data
    Double_t TStamp;    
    uint64_t Trg_Id;   
    uint64_t ch_mask;     
    Int_t hits;
    TString * p_file_format = &this->file_format;
    TString * p_janus_rel = &this->janus_rel;
    TString * p_acq_mode = &this->acq_mode;
    
    // only the 2D and 3D variables need to have a different type than the one read in the binary file
    int16_t data_type[NBOARDS][NCHANNELS];
    int16_t data_type_timing[NBOARDS][NCHANNELS][MAXHITS];
    int32_t LG[NBOARDS][NCHANNELS];
    int32_t HG[NBOARDS][NCHANNELS];
    int64_t counts[NBOARDS][NCHANNELS]; // warning: (in principle) there could be loss of data...
    float ToA[NBOARDS][NCHANNELS];
    float ToT[NBOARDS][NCHANNELS];
    float ToA_timing[NBOARDS][NCHANNELS][MAXHITS];
    float ToT_timing[NBOARDS][NCHANNELS][MAXHITS];

};



template<typename T>
void reset(T c[NBOARDS][NCHANNELS]){
    for (int i = 0; i < NBOARDS; i++) {
        for (int j = 0; j < NCHANNELS; j++) {
            c[i][j] = static_cast<T>(-1);
        }
    }
}

template<typename T>
void reset(T c[NBOARDS][NCHANNELS][MAXHITS]){
    for (int i = 0; i < NBOARDS; i++) {
        for (int j = 0; j < NCHANNELS; j++) {
            for (int k=0; k<MAXHITS; k++){
                c[i][j][k] = static_cast<T>(-1);
            }
        }
    }
}

void is_valid_ind(int board,int ch);
modes find_mode(const TString& str);
modes find_mode(uint8_t acq_mode);
void reset_stored_vars(stored_vars &v, modes &mode);
TTree * make_branches_info(TTree * t, const modes& mode, stored_vars &v);
TTree * make_branches_data(TTree * t, const modes& mode, stored_vars &v);
TTree * open_branches_info(TTree * t, const modes& mode, stored_vars &v);
