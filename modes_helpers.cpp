#include "modes_helpers.hpp"

void is_valid_ind(int board,int ch,int N_boards){
    if((board>=N_boards)||(ch>=64)){
        cout << "Board ID " << board << " and ch. ID " << ch << " are invalid."<< endl;
        throw runtime_error("Found invalid board ID or channel ID, unable to produce root file.");
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


modes hashString(const TString& str) {
    if (str == "Spectroscopy") return modes::Spectroscopy;
    if (str == "Spect_Timing")  return modes::Spect_Timing;
    if (str == "Timing_CStart")  return modes::Timing;
    if (str == "Timing_CStop")  return modes::Timing;
    if (str == "Counting")  return modes::Counting;

    else throw runtime_error("Unknown acquisition mode, unable to produce root file.");
}



// COSE NUOVE
// -------------------------------------------------------------------------------

// per fillare?
// for (int i = 0; i < N_boards; ++i) {
    // std::fill(LG[i], LG[i] + 64, -2);
// }


TTree * make_branches_info(TTree * t, const TString& mode, stored_vars &v){
    // create common branches
    t->Branch("board_mod", &v.board_mod);
    t->Branch("file_format", &v.file_format);
    t->Branch("janus_rel", &v.janus_rel);
    t->Branch("acq_mode", &v.acq_mode);
    t->Branch("run", &v.run, "run/I");
    t->Branch("time_epoch", &v.time_epoch, "time_epoch/i");
    t->Branch("time_UTC", &v.time_UTC);

    switch (hashString(mode)) {
        case modes::Spectroscopy:
            t->Branch("e_Nbins", &v.e_Nbins);
            break;

        case modes::Spect_Timing:
            t->Branch("e_Nbins", &v.e_Nbins, "e_Nbins/I");
            t->Branch("time_LSB_ns", &v.time_LSB, "time_LSB_ns/D");
            t->Branch("time_unit", &v.time_unit);
            break;

        case modes::Timing:
            t->Branch("time_LSB_ns", &v.time_LSB, "time_LSB_ns/D");
            t->Branch("time_unit", &v.time_unit);
            break;

        case modes::Counting:
            break;

    }
    return t;
}

TTree * make_branches_data(TTree * t, const TString& mode, stored_vars &v){
    int N_boards = v.get_N_boards();

    t->Branch("TStamp_us",&v.TStamp, "TStamp_us/D");
    t->Branch("Num_Hits",&v.hits, "Num_Hits/I");

    switch (hashString(mode)) {
        case modes::Spectroscopy:
            t->Branch("Trg_Id",&v.Trg_Id, "Trg_Id/l");
            t->Branch("ch_mask", &v.ch_mask);
            t->Branch("data_type", &v.data_type);
            t->Branch("PHA_LG",&v.LG,Form("PHA_LG[%i][64]/I",N_boards));
            t->Branch("PHA_HG",&v.HG,Form("PHA_HG[%i][64]/I",N_boards));

            break;

        
        case modes::Spect_Timing:
            t->Branch("Trg_Id",&v.Trg_Id, "Trg_Id/l");
            t->Branch("ch_mask", &v.ch_mask);
            t->Branch("data_type", &v.data_type);
            t->Branch("PHA_LG",&v.LG,Form("PHA_LG[%i][64]/I",N_boards));
            t->Branch("PHA_HG",&v.HG,Form("PHA_HG[%i][64]/I",N_boards));
            if (v.time_unit=="LSB"){
                t->Branch("ToA_LSB",&v.ToA, Form("ToA_LSB[%i][64]/D",N_boards));
                t->Branch("ToT_LSB",&v.ToT, Form("ToT_LSB[%i][64]/D",N_boards));
            }
            else {
                t->Branch("ToA_ns",&v.ToA, Form("ToA_ns[%i][64]/D",N_boards));
                t->Branch("ToT_ns",&v.ToT, Form("ToT_ns[%i][64]/D",N_boards));
            }

            break;


        case modes::Timing:
            if (v.time_unit=="LSB"){
                t->Branch("ToA_LSB",&v.ToA, Form("ToA_LSB[%i][64]/D",N_boards));
                t->Branch("ToT_LSB",&v.ToT, Form("ToT_LSB[%i][64]/D",N_boards));
            }
            else {
                t->Branch("ToA_ns",&v.ToA, Form("ToA_ns[%i][64]/D",N_boards));
                t->Branch("ToT_ns",&v.ToT, Form("ToT_ns[%i][64]/D",N_boards));
            }       
            t->Branch("data_type", &v.data_type);

            break;


        case modes::Counting:
            t->Branch("Trg_Id",&v.Trg_Id, "Trg_Id/I");
            t->Branch("ch_mask", &v.ch_mask);
            t->Branch("counts",&v.counts, Form("counts[%i][64]/I",N_boards));

            break;

    }
    return t;
}


// -------------------------------------------------------------------------------
// FINE COSE NUOVE



TTree * make_info_tree(vector<vector<string>>& metadata, const TString& mode, stored_vars &v){
    TTree *t = new TTree("info","info");
    make_branches_info(t, mode, v);

    // get metadata
    v.board_mod = stoi(metadata[0][1]);
    v.file_format = metadata[1][1];
    v.janus_rel = metadata[2][1];
    v.acq_mode = metadata[3][1];


    switch (hashString(mode)) {
        case modes::Spectroscopy:
            v.e_Nbins = stoi(metadata[4][1]);
            v.run = stoi(metadata[5][1]);
            v.time_epoch = stoul(metadata[6][1]);
            v.time_UTC = metadata[7][1]+":"+metadata[7][2]+":"+metadata[7][3];        

            t->Fill();
            break;

        case modes::Spect_Timing:
            v.e_Nbins = stoi(metadata[4][1]);
            v.time_LSB = stod(metadata[5][1]);
            v.time_unit = metadata[6][1];
            v.run = stoi(metadata[7][1]);
            v.time_epoch = stoul(metadata[8][1]);
            v.time_UTC = metadata[9][1]+":"+metadata[9][2]+":"+metadata[9][3];

            t->Fill();
            break;

        case modes::Timing:
            v.time_LSB = stod(metadata[4][1]);
            v.time_unit = metadata[5][1];
            v.run = stoi(metadata[6][1]);
            v.time_epoch = stoul(metadata[6][1]);
            v.time_UTC = metadata[8][1]+":"+metadata[8][2]+":"+metadata[8][3];  

            t->Fill();
            break;

        case modes::Counting:
            v.run = stoi(metadata[4][1]);
            v.time_epoch = stoul(metadata[5][1]);
            v.time_UTC = metadata[6][1]+":"+metadata[6][2]+":"+metadata[6][3];  

            t->Fill();
            break;

    }
    return t;
}


TTree * make_data_tree(vector<vector<string>>& data, const TString& mode, stored_vars &v){
    TTree *t = new TTree("datas","datas");
    make_branches_data(t, mode, v);
    int N_boards = v.get_N_boards();

    Int_t ch_ID, pha_lg, pha_hg, board;
    Double_t cur_tr_T, pr_tr_T;
    unsigned long long r, cur_tr_ID, pr_tr_ID, ev_start = 0;    

    switch (hashString(mode)) {
        case modes::Spectroscopy:
            cout << "Acquisition mode: Spectroscopy." << endl;

            for (r=1; r<data.size(); r++){  // compare each row to the previous one
                cur_tr_ID = stoi(data[r][1]);
                pr_tr_ID = stoi(data[r-1][1]);
                if (cur_tr_ID!=pr_tr_ID || r==(data.size()-1)){
                    vector<vector<string>> event_block = get_event(data, r, ev_start);
                    v.Trg_Id = pr_tr_ID;
                    v.TStamp = stold(data[r-1][0]);
                    v.hits = stoi(data[r-1][3]);
                    v.ch_mask = data[1][4];
                    v.data_type = data[r-1][6];

                    for (int i = 0; i < N_boards; ++i) {fill(v.LG[i], v.LG[i] + 64, -2);}
                    for (int i = 0; i < N_boards; ++i) {fill(v.HG[i], v.HG[i] + 64, -2);}

                    for (long unsigned int i=0; i<event_block.size(); i++){
                        ch_ID = stoi(event_block[i][5]);
                        pha_lg= stoi(event_block[i][7]); 
                        pha_hg= stoi(event_block[i][8]); 
                        board= stoi(event_block[i][2]);

                        is_valid_ind(board, ch_ID,N_boards);

                        v.LG[board][ch_ID] = pha_lg;
                        v.HG[board][ch_ID] = pha_hg;
                    }

                    t->Fill();
                    // assign value to the start index of the next event
                    ev_start = r;
                }
            }
            break;

        
        case modes::Spect_Timing:
            cout << "Acquisition mode: Spect_Timing." << endl;

            for (r=1; r<data.size(); r++){  // compare each row to the previous one
                cur_tr_ID = stoi(data[r][1]);
                pr_tr_ID = stoi(data[r-1][1]);
                if (cur_tr_ID!=pr_tr_ID || r==(data.size()-1)){
                    vector<vector<string>> event_block = get_event(data, r, ev_start);

                    v.Trg_Id = pr_tr_ID;
                    v.ch_mask = data[1][4];
                    v.TStamp = stold(data[r-1][0]);
                    v.hits = stoi(data[r-1][3]);
                    v.data_type = data[r-1][6];

                    for (int i = 0; i < N_boards; ++i) {fill(v.LG[i], v.LG[i] + 64, -2);}
                    for (int i = 0; i < N_boards; ++i) {fill(v.HG[i], v.HG[i] + 64, -2);}
                    for (int i = 0; i < N_boards; ++i) {fill(v.ToA[i], v.ToA[i] + 64, -2);}
                    for (int i = 0; i < N_boards; ++i) {fill(v.ToT[i], v.ToT[i] + 64, -2);}

                    for (int i=0; i<event_block.size(); i++){
                        board= stoi(event_block[i][2]);
                        ch_ID = stoi(event_block[i][5]);     
                        is_valid_ind(board, ch_ID,N_boards);                      
                        v.LG[board][ch_ID] = stoi(event_block[i][7]);
                        v.HG[board][ch_ID] = stoi(event_block[i][8]);
                        v.ToA[board][ch_ID] = (Double_t)stod(event_block[i][9]);
                        v.ToT[board][ch_ID] = (Double_t)stod(event_block[i][10]);
                    }

                    t->Fill();
                    // assign value to the start index of the next event
                    ev_start = r;
                }
            }
            break;


        case modes::Timing:
            // the acquistion modes Timing_CStart and Timing_CStop have the same structure
            cout << "The acquisition mode is "<<mode<<"." << endl;

            for (r=1; r<data.size(); r++){  // compare each row to the previous one
                cur_tr_T = stold(data[r][0]);
                pr_tr_T = stold(data[r-1][0]);
                if (cur_tr_T!=pr_tr_T || r==(data.size()-1)){
                    vector<vector<string>> event_block = get_event(data, r, ev_start);

                    v.TStamp = stold(data[r-1][0]);
                    v.hits = stoi(data[r-1][2]);
                    v.data_type = data[r-1][4];
                
                    for (int i = 0; i < N_boards; ++i) {fill(v.ToA[i], v.ToA[i] + 64, -2);}
                    for (int i = 0; i < N_boards; ++i) {fill(v.ToT[i], v.ToT[i] + 64, -2);}

                    for (int i=0; i<event_block.size(); i++){
                        board= stoi(event_block[i][1]);
                        ch_ID = stoi(event_block[i][3]);                    
                        is_valid_ind(board, ch_ID,N_boards);
                        v.ToA[board][ch_ID] = stoi(event_block[i][5]);
                        v.ToT[board][ch_ID] = stoi(event_block[i][6]);

                    }

                    t->Fill();
                    // assign value to the start index of the next event
                    ev_start = r;
                }
            }
            break;


        case modes::Counting:
            cout << "The acquisition mode is Counting." << endl;

            for (r=1; r<data.size(); r++){  // compare each row to the previous one
                cur_tr_ID = stoi(data[r][1]);
                pr_tr_ID = stoi(data[r-1][1]);
                if (cur_tr_ID!=pr_tr_ID || r==(data.size()-1)){
                    vector<vector<string>> event_block = get_event(data, r, ev_start);

                    v.Trg_Id = pr_tr_ID;
                    v.TStamp = stold(data[r-1][0]);
                    v.ch_mask = data[1][4];
                    v.hits = stoi(data[r-1][3]);

                    for (int i = 0; i < N_boards; ++i) {fill(v.counts[i], v.counts[i] + 64, -2);}

                    for (int i=0; i<event_block.size(); i++){
                        board= stoi(event_block[i][2]);
                        ch_ID = stoi(event_block[i][5]);  
                        is_valid_ind(board, ch_ID,N_boards);
                        v.counts[board][ch_ID] = stoi(event_block[i][6]);
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
