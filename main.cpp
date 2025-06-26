#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "TFile.h"
#include "TTree.h"

using namespace std;

void read_csv(const string& filepath){
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

    cout << "Numero di righe lette: " << line_count << endl;
    cout << data.size() << endl;    

    // create tree to store the variables and the info about the acquisition
    TTree *tr_info = new TTree("info","info");
    TTree *tr_data = new TTree("data","data");
    unsigned long long Trg_Id;
    double TStamp;
    int hits;
    vector<vector<int>> LG;
    vector<vector<int>> HG;

    tr_data->Branch("Trg_Id",&Trg_Id, "Trg_Id/l");
    tr_data->Branch("TStamp",&TStamp, "TStamp/D");
    tr_data->Branch("Num_Hits",&hits, "Num_Hits/I");
    tr_data->Branch("PHA_LG",&LG, "PHA_LG");
    tr_data->Branch("PHA_HG",&HG, "PHA_HG");

    // vector containing the column names:
    // TStamp_us,Trg_Id,Board_Id,Num_Hits,ChannelMask,CH_Id,DataType,PHA_LG,PHA_HG
    vector<string> col_names = data[0];

    unsigned long long r;
    int new_events = 1;
    int ev_count = 0;
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
            ev_count+=event_block.size();
            Trg_Id = pr_tr_ID;
            TStamp = stof(data[r-1][0]);
            hits = stoi(data[r-1][3]);

            for (int i=0; i<event_block.size(); i++){
                ch_ID = stoi(event_block[i][5]);
                pha_lg= stoi(event_block[i][7]); 
                pha_hg= stoi(event_block[i][8]); 
                board= stoi(event_block[i][2]);
                LG.push_back({board, ch_ID, pha_lg});
                HG.push_back({board, ch_ID, pha_hg});
            }

            tr_data->Fill();
            // assign new value to the start of the next event: its first element
            // is the first one with a different trigger ID
            ev_start = r;

        }
    }

    // save the data of the last event :( 
    auto start = data.begin() + ev_start;  // element coresponding to the start iterator is included
    auto end = data.begin() + r;           // element coresponding to the stop iterator is excluded

    vector<vector<string>> event_block(start, end);
    ev_count+=event_block.size();
    Trg_Id = pr_tr_ID;
    TStamp = stoi(data[r-1][0]);
    hits = stoi(data[r-1][3]);

    for (int i=0; i<event_block.size(); i++){
        ch_ID = stoi(event_block[i][5]);
        pha_lg= stoi(event_block[i][7]); 
        pha_hg= stoi(event_block[i][8]); 
        board= stoi(event_block[i][2]);
        LG.push_back({board, ch_ID, pha_lg});
        HG.push_back({board, ch_ID, pha_hg});
    }

    tr_data->Fill();

    ev_count+=event_block.size();

    cout << r << endl;
    cout << new_events*64 << endl;
    cout << ev_count << endl;

    // write output file
    TFile fout("test_files/test_c.root", "recreate");
    tr_data->Write();
    fout.Close();
    cout << Trg_Id*64 << endl;
    cout << Trg_Id << endl;



}

int main(){
    read_csv("test_files/test.csv");

    return 1;
}