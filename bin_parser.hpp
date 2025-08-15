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
    uint8_t board_Id;       // int 8 bit
    double TStamp;          // double 64 
    uint64_t Trg_Id;        // int 64 bit
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

int get_bin_data(string inFile, string outFile, stored_vars &v);