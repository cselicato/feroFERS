#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "TFile.h"
#include "TTree.h"

using namespace std;

void read_csv(const string& filepath, int N_boards=1){
    
    fstream file;
    file.open(filepath, ios::in);

    if (!file.is_open()) {
        cout << "Error while opening file " << filepath << endl;
        return;
    }
    cout << "Opened file: " << filepath << endl;

    string line;
    vector<vector<string>> data;
    vector<vector<string>> metadata;

    // read file
    while (getline(file, line)){
        if (line[2] == *"*"){
            continue;        }

        else if (line[0] == *"/"){
            vector<string> row;
            stringstream ss(line);
            string value;
            while (getline(ss, value, ':')) {
                row.push_back(value);    }
            metadata.push_back(row);
        }
        else {       
            vector<string> row;
            stringstream ss(line);
            string value;
            while (getline(ss, value, ',')) {
                row.push_back(value);    }
            data.push_back(row);
        }
    } 

    // get metadata
    // up to the acquisition mode all the files have the same format
    int board_mod = stoi(metadata[0][1]);
    TString file_format = metadata[1][1];
    TString janus_rel = metadata[2][1];
    TString acq_mode = metadata[3][1];
    acq_mode.Resize(acq_mode.Sizeof()-2);

    // create TTrees
    TTree *tr_info = new TTree("info","info");
    TTree *tr_data = new TTree("datas","datas");

    tr_info->Branch("board_mod", &board_mod);
    tr_info->Branch("file_format", &file_format);
    tr_info->Branch("janus_rel", &janus_rel);
    tr_info->Branch("acq_mode", &acq_mode);


    if (acq_mode.CompareTo("Spectroscopy")==0){
        cout << "The acquisition mode is Spectroscopy." << endl;
        // get the remaining metadata
        int e_Nbins = stoi(metadata[4][1]);
        int run = stoi(metadata[5][1]);
        char * pEnd;
        UInt_t time_epoch = strtol(metadata[6][1].c_str(),&pEnd,10); // ?
        TString time_UTC = metadata[7][1];        

        // create branches for the info tree and fill it with the metadata
        tr_info->Branch("e_Nbins", &e_Nbins);
        tr_info->Branch("run", &run);
        tr_info->Branch("time_epoch", &time_epoch, "time_epoch/i");
        tr_info->Branch("time_UTC", &time_UTC);

        tr_info->Fill(); // this tree needs to be filled only once

        // create branches to store the recorded data
        unsigned long long Trg_Id;
        double TStamp;
        int hits;
        int LG[N_boards][64];
        int HG[N_boards][64];
        // initialize all values to -1
        for (int i=0; i<N_boards; i++){
            for (int j=0; j<64; j++){
                LG[i][j] = -1;
                HG[i][j] = -1;  }
        }
        tr_data->Branch("Trg_Id",&Trg_Id, "Trg_Id/l");
        tr_data->Branch("TStamp",&TStamp, "TStamp/F");
        tr_data->Branch("Num_Hits",&hits, "Num_Hits/I");
        tr_data->Branch("PHA_LG",&LG,Form("LG[%i][64]/I",N_boards));
        tr_data->Branch("PHA_HG",&HG,Form("HG[%i][64]/I",N_boards));

        // vector containing the column names:
        // TStamp_us,Trg_Id,Board_Id,Num_Hits,ChannelMask,CH_Id,DataType,PHA_LG,PHA_HG
        vector<string> col_names = data[0];

        unsigned long long r;
        unsigned long long cur_tr_ID, pr_tr_ID;
        int ch_ID, pha_lg, pha_hg, board;

        // remove first row (it contains the names of the columns)
        data.erase(data.begin());
        
        unsigned long long ev_start = 0;
        for (r=1; r<data.size(); r++){  // to compare each row to the previous one the first index must be 1
            cur_tr_ID = stoi(data[r][1]);
            pr_tr_ID = stoi(data[r-1][1]);
            if (cur_tr_ID!=pr_tr_ID){
                // create the event: collection of recorded data that have the same trigger, so 
                // same trigger, so it's from index ev_start to (r-1)
                auto start = data.begin() + ev_start;  // element coresponding to the start iterator is included
                auto end = data.begin() + r;           // element coresponding to the stop iterator is excluded

                vector<vector<string>> event_block(start, end);
                Trg_Id = pr_tr_ID;
                TStamp = stof(data[r-1][0]);
                hits = stoi(data[r-1][3]);

                for (int i=0; i<event_block.size(); i++){
                    ch_ID = stoi(event_block[i][5]);
                    pha_lg= stoi(event_block[i][7]); 
                    pha_hg= stoi(event_block[i][8]); 
                    board= stoi(event_block[i][2]);
                    LG[board][ch_ID] = pha_lg;
                    HG[board][ch_ID] = pha_hg;
                }

                tr_data->Fill();
                // assign new value to the start of the next event: its first element
                // is the first one with a different trigger ID
                ev_start = r;
            }
            // save last event
            else if (r==(data.size()-1)){
                // create the event: collection of recorded data that have the same trigger, so 
                // same trigger, so it's from index ev_start to (r-1)
                auto start = data.begin() + ev_start;  // element coresponding to the start iterator is included
                auto end = data.begin() + r;           // element coresponding to the stop iterator is excluded

                vector<vector<string>> event_block(start, end);
                Trg_Id = pr_tr_ID;
                TStamp = stof(data[r-1][0]);
                hits = stoi(data[r-1][3]);

                for (int i=0; i<event_block.size(); i++){
                    ch_ID = stoi(event_block[i][5]);
                    pha_lg= stoi(event_block[i][7]); 
                    pha_hg= stoi(event_block[i][8]); 
                    board= stoi(event_block[i][2]);
                    LG[board][ch_ID] = pha_lg;
                    HG[board][ch_ID] = pha_hg;
                }

                tr_data->Fill();
            }
        }
    
    }
    
    
    else if (acq_mode.CompareTo("Spect_Timing")==0){
        cout << "The acquisition mode is Spect_Timing." << endl;

        // get the remaining metadata
        Int_t e_Nbins = stoi(metadata[4][1]);
        Float_t time_LSB = stof(metadata[5][1]);
        TString time_unit = metadata[6][1];
        Int_t run = stoi(metadata[7][1]);
        char * pEnd;
        UInt_t time_epoch = strtol(metadata[8][1].c_str(),&pEnd,10); // ?
        TString time_UTC = metadata[9][1];  

        // create branches for the info tree and fill it with the metadata
        tr_info->Branch("e_Nbins", &e_Nbins, "e_Nbins/I");
        tr_info->Branch("time_LSB", &time_LSB, "time_LSB/F");
        tr_info->Branch("time_unit", &time_unit);
        tr_info->Branch("run", &run, "run/I");
        tr_info->Branch("time_epoch", &time_epoch, "time_epoch/i");
        tr_info->Branch("time_UTC", &time_UTC);

        tr_info->Fill(); // this tree needs to be filled only once

        // create branches to store the recorded data
        unsigned long long Trg_Id;
        Double_t TStamp;
        Int_t hits;
        Int_t LG[N_boards][64];
        Int_t HG[N_boards][64];
        Double_t ToA[N_boards][64];
        Double_t ToT[N_boards][64];
        // initialize all values to -1
        for (int i=0; i<N_boards; i++){
            for (int j=0; j<64; j++){
                LG[i][j] = -1;
                HG[i][j] = -1;  
                ToA[i][j] =(Double_t) -1.;
                ToT[i][j] =(Double_t) -1.;  }
        }
        tr_data->Branch("Trg_Id",&Trg_Id, "Trg_Id/l");
        tr_data->Branch("TStamp",&TStamp, "TStamp/D");
        tr_data->Branch("Num_Hits",&hits, "Num_Hits/I");
        tr_data->Branch("PHA_LG",&LG,Form("LG[%i][64]/I",N_boards));
        tr_data->Branch("PHA_HG",&HG,Form("HG[%i][64]/I",N_boards));
        tr_data->Branch("ToA",&ToA, Form("ToA[%i][64]/D",N_boards));
        tr_data->Branch("ToT",&ToT, Form("ToT[%i][64]/D",N_boards));

        // vector containing the column names:
        vector<string> col_names = data[0];

        unsigned long long r;
        unsigned long long cur_tr_ID, pr_tr_ID;
        int ch_ID, board;

        // remove first row (it contains the names of the columns)
        data.erase(data.begin());

        unsigned long long ev_start = 0;
        for (r=1; r<data.size(); r++){  // to compare each row to the previous one the first index must be 1
            cur_tr_ID = stoi(data[r][1]);
            pr_tr_ID = stoi(data[r-1][1]);
            if (cur_tr_ID!=pr_tr_ID){
                // create the event: collection of recorded data that have the same trigger, so 
                // same trigger, so it's from index ev_start to (r-1)
                auto start = data.begin() + ev_start;  // element coresponding to the start iterator is included
                auto end = data.begin() + r;           // element coresponding to the stop iterator is excluded

                vector<vector<string>> event_block(start, end);
                Trg_Id = pr_tr_ID;
                TStamp = stod(data[r-1][0]);
                hits = stoi(data[r-1][3]);
                
                for (int i=0; i<event_block.size(); i++){
                    board= stoi(event_block[i][2]);
                    ch_ID = stoi(event_block[i][5]);                    
                    // store the spectroscopy data
                    LG[board][ch_ID] = stoi(event_block[i][7]);
                    HG[board][ch_ID] = stoi(event_block[i][8]);
                    // store the timing data
                    ToA[board][ch_ID] = (Double_t)stod(event_block[i][9]);
                    ToT[board][ch_ID] = (Double_t)stod(event_block[i][10]);
                }

                tr_data->Fill();
                // assign new value to the start of the next event: its first element
                // is the first one with a different trigger ID
                ev_start = r;
            }
            // save last event
            else if (r==(data.size()-1)){
                // create the event: collection of recorded data that have the same trigger, so 
                // same trigger, so it's from index ev_start to (r-1)
                auto start = data.begin() + ev_start;  // element coresponding to the start iterator is included
                auto end = data.begin() + r;           // element coresponding to the stop iterator is excluded

                vector<vector<string>> event_block(start, end);
                Trg_Id = pr_tr_ID;
                TStamp = stod(data[r-1][0]);
                hits = stoi(data[r-1][3]);
                
                for (int i=0; i<event_block.size(); i++){
                    board= stoi(event_block[i][2]);
                    ch_ID = stoi(event_block[i][5]);                    
                    // store the spectroscopy data
                    LG[board][ch_ID] = stoi(event_block[i][7]);
                    HG[board][ch_ID] = stoi(event_block[i][8]);
                    // store the timing data
                    ToA[board][ch_ID] = (Double_t)stod(event_block[i][9]);
                    ToT[board][ch_ID] = (Double_t)stod(event_block[i][10]);
                }

                tr_data->Fill();\
            }
        
        }
    


            
    }
    else if (acq_mode.CompareTo("Timing_CStart")==0){
        cout << "The acquisition mode is Timing_CStart." << endl;

    }
    else {cout << "Unable to recognize data acquisition mode " << acq_mode.Data() << endl;  }


    // write trees in the output file
    string outpath = filepath.substr(0,filepath.size()-3)+"root";
    TFile fout(outpath.c_str(), "recreate");
    tr_data->Write();
    tr_info->Write();
    // close file
    fout.Close();

}

int main(){
    string filepath;
    cin >> filepath;
    read_csv(filepath);

    return 1;
}