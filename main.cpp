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

const char *argp_program_version = "converter 1.0";
const char *argp_program_bug_address = "<carmen.selicato@cern.ch>";
static char doc[] = "Code to convert a csv file written by the Janus software to a root file.";
static char args_doc[] = "inputFile N_boards";

static struct argp_option options[] = {
  {"output", 'o', "FILE", 0, "Optional output file name"},
  { 0 }
};

struct arguments {
  string inFile;
  int N_boards;  
  string outFile;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = static_cast<struct arguments*>(state->input);
    switch (key) {
    case 'o':
        arguments->outFile = arg;
        break;
    case ARGP_KEY_ARG:
      switch (state->arg_num) {
        case 0: arguments->inFile = arg; break;
        case 1: arguments->N_boards = stoi(arg); break;
        default: return ARGP_ERR_UNKNOWN;
      }
      break;
    default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

vector<string> split_line(string line, char delimiter){
    vector<string> line_content={};
    stringstream ss(line);
    string value;
    while (getline(ss, value, delimiter)) {
        if (!value.empty()){
        line_content.push_back(value);}    }

    return line_content;
}

bool consistent_ind(int board,int ch,int N_boards){
    if((board<N_boards)&&(ch<64)){
        return true;    }
    else {
        cout << "Board ID " << board << " and ch. ID " << ch << " are invalid."<< endl;
        cout << "File conversion failed." << endl;
        return false;
    }
}


void read_csv(const string& filepath, const string& outfile, int N_boards){

    TStopwatch timer;
    timer.Start();
    
    fstream file;
    file.open(filepath, ios::in);

    if (!file.is_open()) {
        cout << "Error while opening file " << filepath << endl;
        return;
    }
    cout << "Opened file: " << filepath << endl;

    string line;
    vector<vector<string>> data, metadata;
    set<size_t> row_sizes = {};

    // read file
    while (getline(file, line)){
        if (line.empty()==false){
            // if present, remove CR
            if (line.back() == '\r') {
                line.pop_back();    }

            if (line[2] == '*'){
                continue;        }

            else if (line[0] == '/'){
                vector<string> row = split_line(line, ':');
                metadata.push_back(row);
            }
            else {       
                vector<string> row = split_line(line, ',');
                data.push_back(row);
                row_sizes.insert(row.size());
            }}
    } 
    cout << "Done reading file." << endl;
    // remove first row (it contains the names of the columns)
    data.erase(data.begin());
    // check consistency of the file (must have a constant number of columns.)
    if (row_sizes.size() != 1){
        cout << "The file " << filepath <<" has inconsistent columns." << endl;
        cout << "No output file was created." << endl;
        return;
    }
    else {cout << "The file has consistent columns, root file can be created." << endl;}
    
    int n_cols = data[0].size();
    // get metadata
    // (up to the acquisition mode all the files have the same format)
    int board_mod = stoi(metadata[0][1]);
    TString file_format = metadata[1][1];
    TString janus_rel = metadata[2][1];
    TString acq_mode = metadata[3][1];

    Int_t run, e_Nbins, hits, ch_ID, pha_lg, pha_hg, board;
    UInt_t time_epoch;
    TString time_UTC, data_type, ch_mask, time_unit;
    Double_t TStamp, time_LSB, cur_tr_T, pr_tr_T;
    unsigned long long Trg_Id, r, cur_tr_ID, pr_tr_ID, ev_start = 0;
    Int_t LG[N_boards][64], HG[N_boards][64], counts[N_boards][64];
    Double_t ToA[N_boards][64], ToT[N_boards][64];

    // initialize all values
    fill(&LG[0][0],&LG[0][0]+N_boards*64, -2);
    fill(&HG[0][0],&HG[0][0]+N_boards*64, -2);    
    fill(&ToA[0][0],&ToA[0][0]+N_boards*64, -2);
    fill(&ToT[0][0],&ToT[0][0]+N_boards*64, -2);
    fill(&counts[0][0],&counts[0][0]+N_boards*64, -2);

    // int ch_ID, pha_lg, pha_hg, board;

    // create TTrees
    TTree *tr_info = new TTree("info","info");
    TTree *tr_data = new TTree("datas","datas");

    // create branches for the info tree and fill it with the metadata
    tr_info->Branch("board_mod", &board_mod);
    tr_info->Branch("file_format", &file_format);
    tr_info->Branch("janus_rel", &janus_rel);
    tr_info->Branch("acq_mode", &acq_mode);
    tr_info->Branch("run", &run, "run/I");
    tr_info->Branch("time_epoch", &time_epoch, "time_epoch/i");
    tr_info->Branch("time_UTC", &time_UTC);

    tr_data->Branch("TStamp",&TStamp, "TStamp/D");
    tr_data->Branch("Num_Hits",&hits, "Num_Hits/I");


    if (acq_mode.CompareTo("Spectroscopy")==0){
        cout << "The acquisition mode is Spectroscopy." << endl;
        // get the remaining metadata
        e_Nbins = stoi(metadata[4][1]);
        run = stoi(metadata[5][1]);
        time_epoch = stoul(metadata[6][1]);
        time_UTC = metadata[7][1];        
        ch_mask = data[1][4];

        // create branches for the info tree and fill it with the metadata
        tr_info->Branch("e_Nbins", &e_Nbins);
        tr_info->Branch("ch_mask", &ch_mask);

        tr_info->Fill(); // this tree needs to be filled only once

        // create branches to store the recorded data
        tr_data->Branch("Trg_Id",&Trg_Id, "Trg_Id/l");
        tr_data->Branch("data_type", &data_type);
        tr_data->Branch("PHA_LG",&LG,Form("PHA_LG[%i][64]/I",N_boards));
        tr_data->Branch("PHA_HG",&HG,Form("PHA_HG[%i][64]/I",N_boards));

        for (r=1; r<data.size(); r++){  // to compare each row to the previous one the first index must be 1
            cur_tr_ID = stoi(data[r][1]);
            pr_tr_ID = stoi(data[r-1][1]);
            if (cur_tr_ID!=pr_tr_ID || r==(data.size()-1)){
                // create the event: collection of recorded data that have the same trigger, so 
                // same trigger, so it's from index ev_start to (r-1)
                auto start = data.begin() + ev_start;  // element coresponding to the start iterator is included
                auto end = data.begin() + r;           // element coresponding to the stop iterator is excluded

                vector<vector<string>> event_block(start, end);
                Trg_Id = pr_tr_ID;
                TStamp = stold(data[r-1][0]);
                hits = stoi(data[r-1][3]);
                data_type = data[r-1][6];
                
                for (long unsigned int i=0; i<event_block.size(); i++){
                    ch_ID = stoi(event_block[i][5]);
                    pha_lg= stoi(event_block[i][7]); 
                    pha_hg= stoi(event_block[i][8]); 
                    board= stoi(event_block[i][2]);
                    if(consistent_ind(board, ch_ID,N_boards)){
                        LG[board][ch_ID] = pha_lg;
                        HG[board][ch_ID] = pha_hg;}
                    else {return;   } 
                }

                tr_data->Fill();
                // assign new value to the start of the next event: its first element
                // is the first one with a different trigger ID
                ev_start = r;
            }
        }
    
    }
    
    
    else if (acq_mode.CompareTo("Spect_Timing")==0){
        cout << "The acquisition mode is Spect_Timing." << endl;

        // get the remaining metadata
        e_Nbins = stoi(metadata[4][1]);
        time_LSB = stod(metadata[5][1]);
        time_unit = metadata[6][1];
        run = stoi(metadata[7][1]);
        time_epoch = stoul(metadata[6][1]);
        time_UTC = metadata[9][1];
        ch_mask = data[1][4];

        // create branches for the info tree and fill it with the metadata
        tr_info->Branch("e_Nbins", &e_Nbins, "e_Nbins/I");
        tr_info->Branch("time_LSB", &time_LSB, "time_LSB/D");
        tr_info->Branch("time_unit", &time_unit);
        tr_info->Branch("ch_mask", &ch_mask);

        tr_info->Fill(); // this tree needs to be filled only once

        // create branches to store the recorded data
        tr_data->Branch("Trg_Id",&Trg_Id, "Trg_Id/l");
        tr_data->Branch("data_type", &data_type);
        tr_data->Branch("PHA_LG",&LG,Form("PHA_LG[%i][64]/I",N_boards));
        tr_data->Branch("PHA_HG",&HG,Form("PHA_HG[%i][64]/I",N_boards));
        tr_data->Branch("ToA",&ToA, Form("ToA[%i][64]/D",N_boards));
        tr_data->Branch("ToT",&ToT, Form("ToT[%i][64]/D",N_boards));

        for (r=1; r<data.size(); r++){  // to compare each row to the previous one the first index must be 1
            cur_tr_ID = stoi(data[r][1]);
            pr_tr_ID = stoi(data[r-1][1]);
            if (cur_tr_ID!=pr_tr_ID || r==(data.size()-1)){
                // create the event: collection of recorded data that have the same trigger, so 
                // same trigger, so it's from index ev_start to (r-1)
                auto start = data.begin() + ev_start;  // element coresponding to the start iterator is included
                auto end = data.begin() + r;           // element coresponding to the stop iterator is excluded

                vector<vector<string>> event_block(start, end);
                Trg_Id = pr_tr_ID;
                TStamp = stold(data[r-1][0]);
                hits = stoi(data[r-1][3]);
                data_type = data[r-1][6];
                
                for (int i=0; i<event_block.size(); i++){
                    board= stoi(event_block[i][2]);
                    ch_ID = stoi(event_block[i][5]);      
                    if(consistent_ind(board, ch_ID,N_boards)){
                        // store the spectroscopy data
                        LG[board][ch_ID] = stoi(event_block[i][7]);
                        HG[board][ch_ID] = stoi(event_block[i][8]);
                        // store the timing data
                        ToA[board][ch_ID] = (Double_t)stod(event_block[i][9]);
                        ToT[board][ch_ID] = (Double_t)stod(event_block[i][10]);
                    }
                    else {return;   } 
                }

                tr_data->Fill();
                // assign new value to the start of the next event: its first element
                // is the first one with a different trigger ID
                ev_start = r;
            }
        }
    
    }
    
    else if (acq_mode.CompareTo("Timing_CStart")==0||acq_mode.CompareTo("Timing_CStop")==0){
        // the acquistion modes Timing_CStart and Timing_CStop have the same structure
        cout << "The acquisition mode is "<<acq_mode<<"." << endl;
        
        // get the remaining metadata
        time_LSB = stod(metadata[4][1]);
        time_unit = metadata[5][1];
        run = stoi(metadata[6][1]);
        time_epoch = stoul(metadata[6][1]);
        time_UTC = metadata[8][1];  
        // create branches for the info tree and fill it with the metadata
        tr_info->Branch("time_LSB", &time_LSB, "time_LSB/D");
        tr_info->Branch("time_unit", &time_unit);

        tr_info->Fill(); // this tree needs to be filled only once

        // create branches to store the recorded data
        tr_data->Branch("data_type", &data_type);
        tr_data->Branch("ToA",&ToA, Form("ToA[%i][64]/D",N_boards));
        tr_data->Branch("ToT",&ToT, Form("ToT[%i][64]/D",N_boards));

        for (r=1; r<data.size(); r++){  // to compare each row to the previous one the first index must be 1
            cur_tr_T = stold(data[r][0]);
            pr_tr_T = stold(data[r-1][0]);
            if (cur_tr_T!=pr_tr_T || r==(data.size()-1)){
                // create the event: collection of recorded data that have the same trigger, so 
                // same trigger, so it's from index ev_start to (r-1)
                auto start = data.begin() + ev_start;  // element coresponding to the start iterator is included
                auto end = data.begin() + r;           // element coresponding to the stop iterator is excluded

                vector<vector<string>> event_block(start, end);
                TStamp = stold(data[r-1][0]);
                hits = stoi(data[r-1][2]);
                data_type = data[r-1][4];
                
                for (int i=0; i<event_block.size(); i++){
                    board= stoi(event_block[i][1]);
                    ch_ID = stoi(event_block[i][3]);                    

                    if(consistent_ind(board, ch_ID,N_boards)){
                        // store the counts
                        counts[board][ch_ID] = stoi(event_block[i][6]);
                    }
                    else {return;   } 
                   
                }

                tr_data->Fill();
                // assign new value to the start of the next event: its first element
                // is the first one with a different trigger ID
                ev_start = r;
            }
        }

    }

    else if (acq_mode.CompareTo("Counting")==0){
        cout << "The acquisition mode is Counting." << endl;

        // get the remaining metadata and fill the tree
        run = stoi(metadata[4][1]);
        time_epoch = stoul(metadata[5][1]);
        time_UTC = metadata[6][1];  
        ch_mask = data[1][4];

        tr_info->Branch("ch_mask", &ch_mask);
        tr_info->Fill(); // this tree needs to be filled only once

        // create branches to store the recorded data
        tr_data->Branch("Trg_Id",&Trg_Id, "Trg_Id/I");
        tr_data->Branch("counts",&counts, Form("counts[%i][64]/I",N_boards));

        for (r=1; r<data.size(); r++){  // to compare each row to the previous one the first index must be 1
            cur_tr_ID = stoi(data[r][1]);
            pr_tr_ID = stoi(data[r-1][1]);
            if (cur_tr_ID!=pr_tr_ID || r==(data.size()-1)){
                // create the event: collection of recorded data that have the same trigger, so 
                // same trigger, so it's from index ev_start to (r-1)
                auto start = data.begin() + ev_start;  // element coresponding to the start iterator is included
                auto end = data.begin() + r;           // element coresponding to the stop iterator is excluded

                vector<vector<string>> event_block(start, end);
                Trg_Id = pr_tr_ID;
                TStamp = stold(data[r-1][0]);
                hits = stoi(data[r-1][3]);
                
                for (int i=0; i<event_block.size(); i++){
                    board= stoi(event_block[i][2]);
                    ch_ID = stoi(event_block[i][5]);  
                    if(consistent_ind(board, ch_ID,N_boards)){
                        // store the timing data
                        counts[board][ch_ID] = stoi(event_block[i][6]);
                    }
                    else {return;   }                     

                }

                tr_data->Fill();
                // assign new value to the start of the next event: its first element
                // is the first one with a different trigger ID
                ev_start = r;
            }
        }
        
    }
    else {cout << "Unable to recognize data acquisition mode " << acq_mode.Data() << endl; 
    return; }


    // write trees in the output file
    TFile fout(outfile.c_str(), "recreate");
    tr_data->Write();
    tr_info->Write();
    // close file
    fout.Close();

    timer.Stop();

    cout << "Output file has been written." << endl;
    cout << "Real time: " << timer.RealTime() << " seconds\n";
    cout << "CPU time: " << timer.CpuTime() << " seconds\n";

}

int main(int argc, char* argv[]){

    struct arguments arguments;
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    if (arguments.outFile.empty()) {
        arguments.outFile = arguments.inFile.substr(0,arguments.inFile.size()-3)+"root";
        cout << "No output file provided. Using default: " << arguments.outFile << endl;
    }
    cout << "Input file: '" << arguments.inFile << "'" << endl;
    cout << "Output file: '" << arguments.outFile << "'" << endl;
    cout << "N_boards: " << arguments.N_boards << endl;
    read_csv(arguments.inFile,arguments.outFile, arguments.N_boards);

    return 0;
}