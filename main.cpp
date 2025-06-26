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

    int line_count = 0;
    string line;
    vector<vector<string>> data;

    while (getline(file, line)){
        if (line[0] == *"/"){
            continue;
        }
        vector<string> row;
        stringstream ss(line);
        string value;

        while (getline(ss, value, ',')) {
            row.push_back(value);
        }

        data.push_back(row);
        line_count+=1;
    } 

    // create tree to store the variables and the info about the acquisition
    TTree *tr_info = new TTree("info","info");
    TTree *tr_data = new TTree("data","data");
    unsigned long long Trg_Id;
    double TStamp;
    int hits;
    int LG[N_boards][64];
    int HG[N_boards][64];

    tr_data->Branch("Trg_Id",&Trg_Id, "Trg_Id/l");
    tr_data->Branch("TStamp",&TStamp, "TStamp/D");
    tr_data->Branch("Num_Hits",&hits, "Num_Hits/I");
    tr_data->Branch("PHA_LG",&LG,Form("LG[%i][64]/I",N_boards));
    tr_data->Branch("PHA_HG",&HG,Form("HG[%i][64]/I",N_boards));

    // vector containing the column names:
    // TStamp_us,Trg_Id,Board_Id,Num_Hits,ChannelMask,CH_Id,DataType,PHA_LG,PHA_HG
    vector<string> col_names = data[0];

    unsigned long long r;
    int new_events = 1;
    unsigned long long cur_tr_ID, pr_tr_ID;
    int ch_ID, pha_lg, pha_hg, board;

    // remove first row (it contains the names of the columns)
    data.erase(data.begin());
    // so to compare each row to the previous one the first index is 1
    unsigned long long ev_start = 0;
    for (r=1; r<data.size(); r++){
        cur_tr_ID = stoi(data[r][1]);
        pr_tr_ID = stoi(data[r-1][1]);
        if (cur_tr_ID!=pr_tr_ID){
            new_events+=1;

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

    cout << r << endl;
    cout << new_events*64 << endl;

    // write output file
    TFile fout("test_files/test.root", "recreate");
    tr_data->Write();
    fout.Close();

}

int main(){
    read_csv("test_files/test.csv");

    return 1;
}