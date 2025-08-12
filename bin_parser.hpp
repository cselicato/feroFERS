#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <vector>
#include <set>
#include <variant>
#include <argp.h>
#include "TFile.h"
#include "TTree.h"
#include "TStopwatch.h"

using namespace std;


class stored_vars_b
{
    public:
    int N_boards;
    int max_hits;
    // metadata
    uint16_t board_mod;
    TString file_format;
    TString janus_rel;
    uint8_t acq_mode;
    uint16_t run;
    uint16_t e_Nbins;
    uint64_t time_epoch;
    Double_t time_LSB;
    TString time_UTC;
    uint8_t time_unit;
    // data
    Int_t hits;
    unsigned long long Trg_Id;
    TString data_type, ch_mask;
    Double_t TStamp;

    int16_t** LG;
    int16_t** HG;
    Int_t** counts;
    variant<float, uint32_t>** ToA;
    variant<float, uint16_t>** ToT;
    variant<float, uint32_t>*** ToA_timing;
    variant<float, uint16_t>*** ToT_timing;
    
    stored_vars_b(int N_boards_, int max_hits_): N_boards(N_boards_), max_hits(max_hits_)
    {
        LG = new int16_t*[N_boards];
        HG = new int16_t*[N_boards];
        counts = new Int_t*[N_boards];
        ToA = new variant<float, uint32_t>*[N_boards];
        ToT = new variant<float, uint16_t>*[N_boards];
        ToA_timing = new variant<float, uint32_t>**[N_boards];
        ToT_timing = new variant<float, uint16_t>**[N_boards];
        
        for (int i = 0; i < N_boards; i++) {
            LG[i] = new int16_t[64];
            HG[i] = new int16_t[64];
            counts[i] = new Int_t[64];
            ToA[i] = new variant<float, uint32_t>[64];
            ToT[i] = new variant<float, uint16_t>[64];

            ToA_timing[i] = new variant<float, uint32_t>*[64];
            ToT_timing[i] = new variant<float, uint16_t>*[64];

            for (int j = 0; j < 64; j++) {
                ToA_timing[i][j] = new variant<float, uint32_t>[max_hits];
                ToT_timing[i][j] = new variant<float, uint16_t>[max_hits];

            }
        }

    }


    int get_N_boards() const {return N_boards;}
    int get_max_hits() const {return max_hits;}

};


/**
 * Class for the file header in binary format
 * (it has the same format for every acquisition mode)
 */
#pragma pack(push, 1)
class FHEADER {
public:
    uint8_t file_format[2];     // 16 bit
    uint8_t janus_rel[3];       // 24 bit
    uint16_t board_mod;         // 16 bit
    uint16_t run;               // 16 bit
    uint8_t acq_mode;           // 8 bit
    uint16_t e_Nbins;           // 16 bit
    uint8_t time_unit;          // 8 bit
    float time_conv;            // 32 bit float
    uint64_t time_epoch;        // 64 bit
};
#pragma pack(pop)

/**
 * Class for the event header for the spectroscopy, spec+timing and counting modes
 */
#pragma pack(push, 1)
class EHEADER {
    public:
    uint16_t ev_size;        // int 16 bit
    uint8_t board_ID;       // int 8 bit
    double TStamp;          // double 64 
    uint64_t trig_ID;        // int 64 bit
    uint64_t ch_mask;        // int 64 bit

    int get_mask_hits() const {
        return __builtin_popcountll(ch_mask);}
};
#pragma pack(pop)

/**
 * Class for the event header for the timing mode
 */
#pragma pack(push, 1)
class T_EHEADER {
    public:
    uint16_t ev_size;        // int 16 bit
    uint8_t board_ID;        // int 8 bit
    double TStamp;           // double 64 (in this case the time stamp is also the reference time)
    uint16_t hits;           // int 16 bit
};
#pragma pack(pop)

// useful for all modes except counting
#pragma pack(push, 1)
class EDATA {
    public:
    uint8_t ch_ID;             // 8 bit
    uint8_t data_type;         // 8 bit
};
#pragma pack(pop)

int get_bin_data(string inFile);