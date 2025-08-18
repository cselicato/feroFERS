#include "modes_helpers.hpp"

void is_valid_ind(int board,int ch,int N_boards){
    if((board>=N_boards)||(ch>=64)){
        cout << "Board ID " << board << " and ch. ID " << ch << " are invalid."<< endl;
        throw runtime_error("Found invalid board ID or channel ID, unable to produce root file.");
    }
}

vector<vector<string>> get_event(vector<vector<string>>& data, unsigned long long r, unsigned long long& ev_start){
    // an event is the collection of recorded data with the same trigger
    vector<vector<string>>::iterator end;
    if(r==data.size()-1){end = data.begin() + r + 1;    }
    else {end = data.begin() + r;}
    vector<vector<string>> event_block(data.begin() + ev_start, end);

    return event_block;
}


modes find_mode(const TString& str) {
    if (str == "Spectroscopy") return modes::Spectroscopy;
    if (str == "Spect_Timing")  return modes::Spect_Timing;
    if (str == "Timing_CStart")  return modes::Timing;
    if (str == "Timing_CStop")  return modes::Timing;
    if (str == "Counting")  return modes::Counting;

    else throw runtime_error("Unknown acquisition mode, unable to produce root file.");
}

modes find_mode(uint8_t acq_mode) {
    if ((acq_mode & 0x1) && !(acq_mode & 0x2)) return modes::Spectroscopy;
    if ((acq_mode & 0x2) && !(acq_mode & 0x1))  return modes::Timing;
    if ((acq_mode & 0x3)  && (acq_mode & 0x2))  return modes::Spect_Timing;
    if (acq_mode & 0x4)  return modes::Counting;

    else throw runtime_error("Unknown acquisition mode, unable to produce root file.");
}

TTree * make_branches_info(TTree * t, const modes& mode, stored_vars &v){
    // create common branches
    t->Branch("board_mod", &v.board_mod, "board_mod/s");
    t->Branch("file_format", &v.file_format);
    t->Branch("janus_rel", &v.janus_rel);
    t->Branch("acq_mode", &v.acq_mode);
    t->Branch("run", &v.run, "run/s");
    t->Branch("time_epoch", &v.time_epoch, "time_epoch/l");
    t->Branch("time_UTC", &v.time_UTC);

    switch (mode) {
        case modes::Spectroscopy:
            t->Branch("e_Nbins", &v.e_Nbins, "e_Nbins/s");
            break;

        case modes::Spect_Timing:
            t->Branch("e_Nbins", &v.e_Nbins, "e_Nbins/s");
            t->Branch("time_LSB_ns", &v.time_conv, "time_LSB_ns/D");
            t->Branch("time_unit", &v.time_unit);
            break;

        case modes::Timing:
            t->Branch("time_LSB_ns", &v.time_conv, "time_LSB_ns/D");
            t->Branch("time_unit", &v.time_unit);
            break;

        case modes::Counting:
            break;

    }
    return t;
}

TTree * make_branches_data(TTree * t, const modes& mode, stored_vars &v){
    int N_boards = v.get_N_boards();
    int max_hits = v.get_max_hits();

    t->Branch("TStamp_us",&v.TStamp, "TStamp_us/D");
    t->Branch("Num_Hits",&v.hits, "Num_Hits/I");

    switch (mode) {
        case modes::Spectroscopy:
            t->Branch("Trg_Id",&v.Trg_Id, "Trg_Id/l");
            t->Branch("ch_mask", &v.ch_mask, "ch_mask/l");
            t->Branch("data_type", *v.data_type,Form("data_type[%i][64]/S",N_boards));
            t->Branch("PHA_LG",*v.LG,Form("PHA_LG[%i][64]/I",N_boards));
            t->Branch("PHA_HG",*v.HG,Form("PHA_HG[%i][64]/I",N_boards));

            break;

        
        case modes::Spect_Timing:
            t->Branch("Trg_Id",&v.Trg_Id, "Trg_Id/l");
            t->Branch("ch_mask", &v.ch_mask, "ch_mask/l");
            t->Branch("data_type", *v.data_type,Form("data_type[%i][64]/S",N_boards));
            t->Branch("PHA_LG",*v.LG,Form("PHA_LG[%i][64]/I",N_boards));
            t->Branch("PHA_HG",*v.HG,Form("PHA_HG[%i][64]/I",N_boards));
            t->Branch("ToA",*v.ToA, Form("ToA[%i][64]/F",N_boards));
            t->Branch("ToT",*v.ToT, Form("ToT[%i][64]/F",N_boards));

            break;


        case modes::Timing:
            t->Branch("ToA",*v.ToA, Form("ToA[%i][64]/F",N_boards));
            t->Branch("ToT",*v.ToT, Form("ToT[%i][64]/F",N_boards));
            t->Branch("data_type", *v.data_type,Form("data_type[%i][64]/S",N_boards));

            break;


        case modes::Counting:
            t->Branch("Trg_Id",&v.Trg_Id, "Trg_Id/l");
            t->Branch("ch_mask", &v.ch_mask, "ch_mask/l");
            t->Branch("counts",*v.counts, Form("counts[%i][64]/L",N_boards));

            break;

    }
    return t;
}


TTree * make_info_tree(vector<vector<string>>& metadata, const modes& mode, stored_vars &v){
    TTree *t = new TTree("info","info");
    make_branches_info(t, mode, v);

    // get metadata
    v.board_mod = stoi(metadata[0][1]);
    v.file_format = metadata[1][1];
    v.janus_rel = metadata[2][1];
    v.acq_mode = metadata[3][1];


    switch (mode) {
        case modes::Spectroscopy:
            v.e_Nbins = stoi(metadata[4][1]);
            v.run = stoi(metadata[5][1]);
            v.time_epoch = stoul(metadata[6][1]);
            v.time_UTC = metadata[7][1]+":"+metadata[7][2]+":"+metadata[7][3];        

            t->Fill();
            break;

        case modes::Spect_Timing:
            v.e_Nbins = stoi(metadata[4][1]);
            v.time_conv = stod(metadata[5][1]);
            v.time_unit = metadata[6][1];
            v.run = stoi(metadata[7][1]);
            v.time_epoch = stoul(metadata[8][1]);
            v.time_UTC = metadata[9][1]+":"+metadata[9][2]+":"+metadata[9][3];

            t->Fill();
            break;

        case modes::Timing:
            v.time_conv = stod(metadata[4][1]);
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


TTree * make_data_tree(vector<vector<string>>& data, const modes& mode, stored_vars &v){
    TTree *t = new TTree("datas","datas");
    make_branches_data(t, mode, v);
    int N_boards = v.get_N_boards();

    Int_t ch_ID, board;
    Double_t cur_tr_T, pr_tr_T;
    unsigned long long r, cur_tr_ID, pr_tr_ID, ev_start = 0;    

    switch (mode) {
        case modes::Spectroscopy:
        {
            cout << "Acquisition mode: Spectroscopy." << endl;

            reset<int16_t>(v.data_type, v);
            reset<Int_t>(v.LG, v);
            reset<Int_t>(v.HG, v);

            for (int i=0; i<data.size(); i++){
                vector<string> row = data[i];
                v.Trg_Id = stoi(row[1]);
                v.TStamp = stod(row[0]);
                v.hits = stoi(row[3]);
                v.ch_mask = stoull(row[4], nullptr, 16);
                
                ch_ID = stoi(row[5]);
                board= stoi(row[2]);

                is_valid_ind(board, ch_ID,N_boards);

                v.data_type[board][ch_ID] = stoi(row[6], nullptr, 16);
                v.LG[board][ch_ID] = stoi(row[7]);
                v.HG[board][ch_ID] = stoi(row[8]);

                if (i==data.size()-1){
                    t->Fill();
                    continue;
                }                

                else if (v.TStamp != stod(data[i+1][0])){
                    t->Fill();
                    reset<int16_t>(v.data_type, v);
                    reset<Int_t>(v.LG, v);
                    reset<Int_t>(v.HG, v);
                }
            }
            break;
        }
        
        case modes::Spect_Timing:
        {
            cout << "Acquisition mode: Spect_Timing." << endl;

            reset<int16_t>(v.data_type, v);
            reset<Int_t>(v.LG, v);
            reset<Int_t>(v.HG, v);
            reset<float>(v.ToA, v);
            reset<float>(v.ToT, v);

            for (int i=0; i<data.size(); i++){
                vector<string> row = data[i];
                v.Trg_Id = stoull(row[1]);
                v.ch_mask = stoull(row[4], nullptr, 16);
                v.TStamp = stod(row[0]);
                v.hits = stoi(row[3]);

                board= stoi(row[2]);
                ch_ID = stoi(row[5]); 

                is_valid_ind(board, ch_ID,N_boards);      
                
                v.data_type[board][ch_ID] = stoi(row[6], nullptr, 16);
                v.LG[board][ch_ID] = stoi(row[7]);
                v.HG[board][ch_ID] = stoi(row[8]);
                v.ToA[board][ch_ID] = stof(row[9]);
                v.ToT[board][ch_ID] = stof(row[10]);

                if (i==data.size()-1){
                    t->Fill();
                    continue;
                } 

                else if (v.TStamp != stod(data[i+1][0])){
                    t->Fill();
                    reset<int16_t>(v.data_type, v);
                    reset<Int_t>(v.LG, v);
                    reset<Int_t>(v.HG, v);
                    reset<float>(v.ToA, v);
                    reset<float>(v.ToT, v);
                }
            }
            break;
        }



        case modes::Timing:
        {
            // the acquistion modes Timing_CStart and Timing_CStop have the same structure
            cout << "The acquisition mode is Timing." << endl;

            reset<int16_t>(v.data_type, v);
            reset<float>(v.ToA, v);
            reset<float>(v.ToT, v);

            for (int i=0; i<data.size(); i++){
                vector<string> row = data[i];
                v.TStamp = stod(row[0]);
                v.hits = stoi(row[2]);

                board= stoi(row[1]);
                ch_ID = stoi(row[3]);     

                is_valid_ind(board, ch_ID,N_boards);

                v.data_type[board][ch_ID] = stoi(row[4], nullptr, 16);
                v.ToA[board][ch_ID] = stof(row[5]);    
                v.ToT[board][ch_ID] = stof(row[6]);   
        
                if (i==data.size()-1){
                    t->Fill();
                    continue;
                }

                else if (v.TStamp != stod(data[i+1][0])){
                    t->Fill();
                    reset<int16_t>(v.data_type, v);
                    reset<float>(v.ToA, v);
                    reset<float>(v.ToT, v);
                }
            }
            break;
        }

        case modes::Counting:
        {
            cout << "The acquisition mode is Counting." << endl;

            reset<int64_t>(v.counts, v);

            for (int i=0; i<data.size(); i++){
                vector<string> row = data[i];
                v.Trg_Id = stoull(row[1]);
                v.TStamp = stod(row[0]);
                v.ch_mask = stoull(row[4], nullptr, 16);
                v.hits = stoi(row[3]);

                board= stoi(row[2]);
                ch_ID = stoi(row[5]);  
                is_valid_ind(board, ch_ID,N_boards);
                v.counts[board][ch_ID] = stoi(row[6]);

                if (i==data.size()-1){
                    t->Fill();
                    continue;
                } 

                else if (v.TStamp != stod(data[i+1][0])){
                    t->Fill();
                    reset<int64_t>(v.counts, v);
                }
            }
            break;
        }
    }
    return t;
}
