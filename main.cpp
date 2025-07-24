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
  int N_boards=-1;  
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
    case ARGP_KEY_END:
        if (arguments->inFile.empty() || arguments->N_boards == -1){
            argp_failure(state, 1, 0, "Missing input file or number of boards. See --help for more information.");
            exit(ARGP_ERR_UNKNOWN);
        }    
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

void is_valid_ind(int board,int ch,int N_boards){
    if((board>=N_boards)||(ch>=64)){
        cout << "Board ID " << board << " and ch. ID " << ch << " are invalid."<< endl;
        throw runtime_error("Found invalid board ID or channel ID, unable to produce root file.");
    }
}



vector<vector<string>> get_csv_metadata(fstream &file){
    vector<vector<string>> metadata;
    string line;
    file.clear();
    file.seekg(0);
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
                break;
            }}
    } 

    return metadata;
}

vector<vector<string>> get_csv_data(fstream &file){
    file.clear();
    file.seekg(0);
    vector<vector<string>> data;
    string line;
    set<size_t> row_sizes = {};

    // read file
    while (getline(file, line)){
        if (line.empty()==false){
            // if present, remove CR
            if (line.back() == '\r') {
                line.pop_back();    }

            if (line[2] == '*' || line[0] == '/'){
                continue;        }

            else {       
                vector<string> row = split_line(line, ',');
                data.push_back(row);
                row_sizes.insert(row.size());
            }}
    } 
    
    data.erase(data.begin());   // remove column names
    
    return data;
}

void is_consistent(vector<vector<string>>& data){
    string line;
    set<size_t> row_sizes = {};

    for (vector<string> row : data){
        row_sizes.insert(row.size());
    }

    if (row_sizes.size() != 1){
        throw runtime_error("The file has inconsistent columns, unable to produce root file.");
    }

}

vector<vector<string>> get_event(vector<vector<string>>& data, unsigned long long& r, unsigned long long& ev_start){
    // an event is the collection of recorded data with the same trigger
    vector<vector<string>>::iterator end;
    if(r==data.size()-1){end = data.begin() + r + 1;    }
    else {end = data.begin() + r;}
    vector<vector<string>> event_block(data.begin() + ev_start, end);

    return event_block;
}

enum class StringCode {
    Spectroscopy,
    Spect_Timing,
    Timing,
    Counting
};

StringCode hashString(const TString& str) {
    if (str == "Spectroscopy") return StringCode::Spectroscopy;
    if (str == "Spect_Timing")  return StringCode::Spect_Timing;
    if (str == "Timing_CStart")  return StringCode::Timing;
    if (str == "Timing_CStop")  return StringCode::Timing;
    if (str == "Counting")  return StringCode::Counting;

    else throw runtime_error("Unknown acquisition mode, unable to produce root file.");
}


TTree * make_info_tree(vector<vector<string>>& metadata, const TString& mode){
    TTree *t = new TTree("info","info");
    // get metadata
    int board_mod = stoi(metadata[0][1]);
    TString file_format = metadata[1][1];
    TString janus_rel = metadata[2][1];
    TString acq_mode = metadata[3][1];

    Int_t run, e_Nbins;
    UInt_t time_epoch;
    Double_t time_LSB;
    TString time_UTC, time_unit;

    // create common branches
    t->Branch("board_mod", &board_mod);
    t->Branch("file_format", &file_format);
    t->Branch("janus_rel", &janus_rel);
    t->Branch("acq_mode", &acq_mode);
    t->Branch("run", &run, "run/I");
    t->Branch("time_epoch", &time_epoch, "time_epoch/i");
    t->Branch("time_UTC", &time_UTC);

    switch (hashString(mode)) {
        case StringCode::Spectroscopy:
            e_Nbins = stoi(metadata[4][1]);
            run = stoi(metadata[5][1]);
            time_epoch = stoul(metadata[6][1]);
            time_UTC = metadata[7][1]+":"+metadata[7][2]+":"+metadata[7][3];        

            t->Branch("e_Nbins", &e_Nbins);
            t->Fill();
            break;

        case StringCode::Spect_Timing:
            e_Nbins = stoi(metadata[4][1]);
            time_LSB = stod(metadata[5][1]);
            time_unit = metadata[6][1];
            run = stoi(metadata[7][1]);
            time_epoch = stoul(metadata[8][1]);
            time_UTC = metadata[9][1]+":"+metadata[9][2]+":"+metadata[9][3];

            t->Branch("e_Nbins", &e_Nbins, "e_Nbins/I");
            t->Branch("time_LSB_ns", &time_LSB, "time_LSB_ns/D");
            t->Branch("time_unit", &time_unit);
            t->Fill();
            break;
        case StringCode::Timing:
            time_LSB = stod(metadata[4][1]);
            time_unit = metadata[5][1];
            run = stoi(metadata[6][1]);
            time_epoch = stoul(metadata[6][1]);
            time_UTC = metadata[8][1]+":"+metadata[8][2]+":"+metadata[8][3];  

            t->Branch("time_LSB_ns", &time_LSB, "time_LSB_ns/D");
            t->Branch("time_unit", &time_unit);
            t->Fill();
            break;
        case StringCode::Counting:
            run = stoi(metadata[4][1]);
            time_epoch = stoul(metadata[5][1]);
            time_UTC = metadata[6][1]+":"+metadata[6][2]+":"+metadata[6][3];  

            t->Fill();
            break;

    }
    return t;
}


TTree * make_data_tree(vector<vector<string>>& data, const TString& mode, int N_boards){
    TTree *t = new TTree("datas","datas");

    Int_t hits, ch_ID, pha_lg, pha_hg, board;
    Int_t LG[N_boards][64], HG[N_boards][64], counts[N_boards][64];    
    Double_t TStamp, cur_tr_T, pr_tr_T;
    Double_t ToA[N_boards][64], ToT[N_boards][64];
    unsigned long long Trg_Id, r, cur_tr_ID, pr_tr_ID, ev_start = 0;    
    TString data_type, ch_mask, time_unit;

    t->Branch("TStamp_us",&TStamp, "TStamp_us/D");
    t->Branch("Num_Hits",&hits, "Num_Hits/I");

    switch (hashString(mode)) {
        case StringCode::Spectroscopy:
            cout << "Acquisition mode: Spectroscopy." << endl;

            t->Branch("Trg_Id",&Trg_Id, "Trg_Id/l");
            t->Branch("ch_mask", &ch_mask);
            t->Branch("data_type", &data_type);
            t->Branch("PHA_LG",&LG,Form("PHA_LG[%i][64]/I",N_boards));
            t->Branch("PHA_HG",&HG,Form("PHA_HG[%i][64]/I",N_boards));

            for (r=1; r<data.size(); r++){  // compare each row to the previous one
                cur_tr_ID = stoi(data[r][1]);
                pr_tr_ID = stoi(data[r-1][1]);
                if (cur_tr_ID!=pr_tr_ID || r==(data.size()-1)){
                    vector<vector<string>> event_block = get_event(data, r, ev_start);
                    Trg_Id = pr_tr_ID;
                    TStamp = stold(data[r-1][0]);
                    hits = stoi(data[r-1][3]);
                    ch_mask = data[1][4];
                    data_type = data[r-1][6];

                    fill(&LG[0][0],&LG[0][0]+N_boards*64, -2);
                    fill(&HG[0][0],&HG[0][0]+N_boards*64, -2);    

                    for (long unsigned int i=0; i<event_block.size(); i++){
                        ch_ID = stoi(event_block[i][5]);
                        pha_lg= stoi(event_block[i][7]); 
                        pha_hg= stoi(event_block[i][8]); 
                        board= stoi(event_block[i][2]);

                        is_valid_ind(board, ch_ID,N_boards);

                        LG[board][ch_ID] = pha_lg;
                        HG[board][ch_ID] = pha_hg;
                    }

                    t->Fill();
                    // assign value to the start index of the next event
                    ev_start = r;
                }
            }
            break;

        
        case StringCode::Spect_Timing:
            cout << "Acquisition mode: Spect_Timing." << endl;

            t->Branch("Trg_Id",&Trg_Id, "Trg_Id/l");
            t->Branch("ch_mask", &ch_mask);
            t->Branch("data_type", &data_type);
            t->Branch("PHA_LG",&LG,Form("PHA_LG[%i][64]/I",N_boards));
            t->Branch("PHA_HG",&HG,Form("PHA_HG[%i][64]/I",N_boards));
            if (time_unit=="LSB"){
                t->Branch("ToA_LSB",&ToA, Form("ToA_LSB[%i][64]/D",N_boards));
                t->Branch("ToT_LSB",&ToT, Form("ToT_LSB[%i][64]/D",N_boards));
            }
            else {
                t->Branch("ToA_ns",&ToA, Form("ToA_ns[%i][64]/D",N_boards));
                t->Branch("ToT_ns",&ToT, Form("ToT_ns[%i][64]/D",N_boards));
            }

            for (r=1; r<data.size(); r++){  // compare each row to the previous one
                cur_tr_ID = stoi(data[r][1]);
                pr_tr_ID = stoi(data[r-1][1]);
                if (cur_tr_ID!=pr_tr_ID || r==(data.size()-1)){
                    vector<vector<string>> event_block = get_event(data, r, ev_start);

                    Trg_Id = pr_tr_ID;
                    ch_mask = data[1][4];
                    TStamp = stold(data[r-1][0]);
                    hits = stoi(data[r-1][3]);
                    data_type = data[r-1][6];

                    fill(&LG[0][0],&LG[0][0]+N_boards*64, -2);
                    fill(&HG[0][0],&HG[0][0]+N_boards*64, -2);    
                    fill(&ToA[0][0],&ToA[0][0]+N_boards*64, -2);
                    fill(&ToT[0][0],&ToT[0][0]+N_boards*64, -2);

                    for (int i=0; i<event_block.size(); i++){
                        board= stoi(event_block[i][2]);
                        ch_ID = stoi(event_block[i][5]);     
                        is_valid_ind(board, ch_ID,N_boards);                      
                        LG[board][ch_ID] = stoi(event_block[i][7]);
                        HG[board][ch_ID] = stoi(event_block[i][8]);
                        ToA[board][ch_ID] = (Double_t)stod(event_block[i][9]);
                        ToT[board][ch_ID] = (Double_t)stod(event_block[i][10]);
                    }

                    t->Fill();
                    // assign value to the start index of the next event
                    ev_start = r;
                }
            }
            break;


        case StringCode::Timing:
            // the acquistion modes Timing_CStart and Timing_CStop have the same structure
            cout << "The acquisition mode is "<<mode<<"." << endl;

            // create branches to store the recorded data
            if (time_unit=="LSB"){
                t->Branch("ToA_LSB",&ToA, Form("ToA_LSB[%i][64]/D",N_boards));
                t->Branch("ToT_LSB",&ToT, Form("ToT_LSB[%i][64]/D",N_boards));
            }
            else {
                t->Branch("ToA_ns",&ToA, Form("ToA_ns[%i][64]/D",N_boards));
                t->Branch("ToT_ns",&ToT, Form("ToT_ns[%i][64]/D",N_boards));
            }       
            t->Branch("data_type", &data_type);

            for (r=1; r<data.size(); r++){  // compare each row to the previous one
                cur_tr_T = stold(data[r][0]);
                pr_tr_T = stold(data[r-1][0]);
                if (cur_tr_T!=pr_tr_T || r==(data.size()-1)){
                    vector<vector<string>> event_block = get_event(data, r, ev_start);

                    TStamp = stold(data[r-1][0]);
                    hits = stoi(data[r-1][2]);
                    data_type = data[r-1][4];
                
                    fill(&ToA[0][0],&ToA[0][0]+N_boards*64, -2);
                    fill(&ToT[0][0],&ToT[0][0]+N_boards*64, -2);

                    for (int i=0; i<event_block.size(); i++){
                        board= stoi(event_block[i][1]);
                        ch_ID = stoi(event_block[i][3]);                    
                        is_valid_ind(board, ch_ID,N_boards);
                        ToA[board][ch_ID] = stoi(event_block[i][5]);
                        ToT[board][ch_ID] = stoi(event_block[i][6]);

                    }

                    t->Fill();
                    // assign value to the start index of the next event
                    ev_start = r;
                }
            }
            break;


        case StringCode::Counting:
            cout << "The acquisition mode is Counting." << endl;

            // create branches to store the recorded data
            t->Branch("Trg_Id",&Trg_Id, "Trg_Id/I");
            t->Branch("ch_mask", &ch_mask);
            t->Branch("counts",&counts, Form("counts[%i][64]/I",N_boards));

            for (r=1; r<data.size(); r++){  // compare each row to the previous one
                cur_tr_ID = stoi(data[r][1]);
                pr_tr_ID = stoi(data[r-1][1]);
                if (cur_tr_ID!=pr_tr_ID || r==(data.size()-1)){
                    vector<vector<string>> event_block = get_event(data, r, ev_start);

                    Trg_Id = pr_tr_ID;
                    TStamp = stold(data[r-1][0]);
                    ch_mask = data[1][4];
                    hits = stoi(data[r-1][3]);

                    fill(&counts[0][0],&counts[0][0]+N_boards*64, -2);

                    for (int i=0; i<event_block.size(); i++){
                        board= stoi(event_block[i][2]);
                        ch_ID = stoi(event_block[i][5]);  
                        is_valid_ind(board, ch_ID,N_boards);
                        counts[board][ch_ID] = stoi(event_block[i][6]);
                    }

                    t->Fill();
                    // assign value to the start index of the next event
                    ev_start = r;
                }
            }
            break;

    }
    return t;
}

int main(int argc, char* argv[]){

    TStopwatch timer;
    timer.Start();

    struct arguments arguments;
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    if (arguments.outFile.empty()) {
        arguments.outFile = arguments.inFile.substr(0,arguments.inFile.size()-3)+"root";
        cout << "No output file provided. Using default: " << arguments.outFile << endl;
        cout << endl;
    }

    fstream file;
    file.open(arguments.inFile, ios::in);

    if (!file.is_open()) {
        cout << "Error while opening file " << arguments.inFile << endl;
        return 1;
    }
    cout << "Opened file: " << arguments.inFile << endl;

    vector<vector<string>> metadata = get_csv_metadata(file);
    vector<vector<string>> data = get_csv_data(file);

    cout << "Done reading file." << endl;

    try
    {
        is_consistent(data);
        cout << "The file has consistent columns."<<endl;
    }
    catch(const exception& e)
    {
        cerr << e.what() << '\n';
        return 1;
    }
    

    TString acq_mode = metadata[3][1];   
    TTree *tr_data, *tr_info;   
    try
    {
        tr_data = make_data_tree(data,acq_mode, arguments.N_boards);
        tr_info = make_info_tree(metadata,acq_mode);

    }
    catch(const exception& e)
    {
        cerr << e.what() << '\n';
        return 1;
    }
     

    // write trees in the output file
    TFile fout(arguments.outFile.c_str(), "recreate");
    tr_data->Write();
    tr_info->Write();
    // close file
    fout.Close();

    timer.Stop();

    cout << "Output file has been written." << endl;
    cout << endl;
    cout << "Real time: " << timer.RealTime() << " seconds\n";
    cout << "CPU time: " << timer.CpuTime() << " seconds\n";

    return 0;
}