#include "csv_parser.hpp"

vector<string> split_line(string line, char delimiter){
    vector<string> line_content={};
    stringstream ss(line);
    string value;
    while (getline(ss, value, delimiter)) {
        if (!value.empty()){
        line_content.push_back(value);}    }

    return line_content;
}


void is_consistent(int size, int exp_size){
    if (size != exp_size){
        throw runtime_error("The file has inconsistent columns, unable to produce root file.");
    }

}


// function to fill the variable stored_vars v with the metadata for the csv case
void fill_info_var(vector<vector<string>> &metadata, modes &mode, stored_vars &v){
    time_t sec_epoch;
    
    v.board_mod = stoi(metadata[0][1]);
    v.file_format = metadata[1][1];
    v.janus_rel = metadata[2][1];
    if ((metadata[3][1] == "Timing_CStart")||(metadata[3][1] == "Timing_CStop")){
        v.acq_mode = "Timing";
    }else{
        v.acq_mode = metadata[3][1];
    }

    switch (mode) {
        case modes::Spectroscopy:
            v.e_Nbins = stoi(metadata[4][1]);
            v.run = stoi(metadata[5][1]);
            v.time_epoch = stoul(metadata[6][1]);
            sec_epoch = v.time_epoch/1000; // convert to seconds
            v.time_UTC = new TTimeStamp(sec_epoch);            

            break;

        case modes::Spect_Timing:
            v.e_Nbins = stoi(metadata[4][1]);
            v.time_conv = stod(metadata[5][1]);
            v.time_unit = metadata[6][1];
            v.run = stoi(metadata[7][1]);
            v.time_epoch = stoul(metadata[8][1]);
            sec_epoch = v.time_epoch/1000; // convert to seconds
            v.time_UTC = new TTimeStamp(sec_epoch);

            break;

        case modes::Timing:
            v.time_conv = stod(metadata[4][1]);
            v.time_unit = metadata[5][1];
            v.run = stoi(metadata[6][1]);
            v.time_epoch = stoul(metadata[6][1]);
            sec_epoch = v.time_epoch/1000; // convert to seconds
            v.time_UTC = new TTimeStamp(sec_epoch); 

            break;

        case modes::Counting:
            v.run = stoi(metadata[4][1]);
            v.time_epoch = stoul(metadata[5][1]);
            sec_epoch = v.time_epoch/1000; // convert to seconds
            v.time_UTC = new TTimeStamp(sec_epoch);  

            break;

    }
}

// function to fill the variable stored_vars v with the event data for the csv case
void fill_data_var(vector<string> &row, modes &mode, stored_vars &v, int hit){
    Int_t ch_ID, board;

    switch (mode) {
        case modes::Spectroscopy:
            v.Trg_Id = stoi(row[1]);
            v.TStamp = stod(row[0]);
            //v.hits = stoi(row[3]);
            v.ch_mask = stoull(row[4], nullptr, 16);
            
            ch_ID = stoi(row[5]);
            board= stoi(row[2]);

            is_valid_ind(board, ch_ID);

            v.data_type[board][ch_ID] = stoi(row[6], nullptr, 16);
            v.LG[board][ch_ID] = stoi(row[7]);
            v.HG[board][ch_ID] = stoi(row[8]);      

            break;

        case modes::Timing:
            //v.TStamp = stod(row[0]);
            //v.hits = stoi(row[2]);

            board= stoi(row[1]);
            ch_ID = stoi(row[3]);    
            
            is_valid_ind(board, ch_ID);

            v.data_type_timing[board][ch_ID][hit] = stoi(row[4], nullptr, 16);
            v.ToA_timing[board][ch_ID][hit] = stof(row[5]);  
            v.ToT_timing[board][ch_ID][hit] = stof(row[6]);  

            //if (hit>=stoi(row[2])){cout << "Something went wrong..."<<endl;}
            hit++;
            break;            

        case modes::Spect_Timing:
            v.Trg_Id = stoull(row[1]);
            v.ch_mask = stoull(row[4], nullptr, 16);
            v.TStamp = stod(row[0]);
            //v.hits = stoi(row[3]);

            board= stoi(row[2]);
            ch_ID = stoi(row[5]); 

            is_valid_ind(board, ch_ID);      
            
            v.data_type[board][ch_ID] = stoi(row[6], nullptr, 16);
            v.LG[board][ch_ID] = stoi(row[7]);
            v.HG[board][ch_ID] = stoi(row[8]);
            v.ToA[board][ch_ID] = stof(row[9]);
            v.ToT[board][ch_ID] = stof(row[10]);          

            break;

        case modes::Counting:
            v.Trg_Id = stoull(row[1]);
            v.TStamp = stod(row[0]);
            v.ch_mask = stoull(row[4], nullptr, 16);
            //v.hits = stoi(row[3]);

            board= stoi(row[2]);
            ch_ID = stoi(row[5]);  

            is_valid_ind(board, ch_ID);

            v.counts[board][ch_ID] = stoi(row[6]);
                
            break;

    }
}


int parse_csv(string inFile, TTree * tr_info,TTree * tr_data, stored_vars &v){

    // open file
    fstream file(inFile, ios::in);
    if (!file) {
        throw runtime_error("Failed to open file.");
    }

    cout << "Opened file: " << inFile << endl;

    // get metadata
    string line;
    vector<vector<string>> metadata;
    vector<string> col_names;
    file.clear();
    file.seekg(0);
    while (getline(file, line)){
        if (line.empty()==false){
            
            if (line.back() == '\r'){ // if present, remove CR
                line.pop_back();    }

            if (line[2] == '*'){
                continue;   }

            else if (line[0] == '/'){
                vector<string> row = split_line(line, ':');
                metadata.push_back(row); 

            }
            else {
                col_names = split_line(line, ',');
                break;    }
        }
    } 
    cout << "Done reading metadata." << endl;

    int exp_size = col_names.size(); // expected size of the rows in the CSV file
    // find acquisition mode
    modes acq_mod = find_mode(metadata[3][1]);

    // make trees' branches
    make_branches_info(tr_info, acq_mod, v);
    make_branches_data(tr_data, acq_mod, v);

    // fill variables that need to be stored in the info tree
    fill_info_var(metadata, acq_mod, v);
    tr_info->Fill();

    // read the events
    float new_TStamp = 0;
    float TStamp_cut = (v.time_unit=="ns") ? 1 : 2 ; // TStamp separation threshold in ns : LSB;
    float old_TStamp = -2*TStamp_cut; // used in all data modes

    int board_now;

    int hit=0, hit_tot=0;
    int hit_frag_timing[NBOARDS] = {0};
    while (getline(file, line)){
        if (line.empty()==false){
            
            if (line.back() == '\r'){ // if present, remove CR
                line.pop_back();    }

            vector<string> row = split_line(line, ',');
            is_consistent(row.size(), exp_size);
 
	    board_now = stof(row[1]); 
            new_TStamp = stof(row[0]); 
            
            if ((new_TStamp-old_TStamp)*(new_TStamp-old_TStamp)>TStamp_cut*TStamp_cut){ // event is different OR first one, update old_TStamp
                if (hit_tot>0){ // event is different: fill tree and LATER fill v
                    v.hits = hit; // hits updated separately from fill_data_var for manual evaluation
                    tr_data->Fill();
                    hit = 0;
                    memset(hit_frag_timing, 0, NBOARDS*sizeof(int));
                }
                old_TStamp = new_TStamp;
                reset_stored_vars(v, acq_mod);  // RESET v and hit BEFORE FILLING AGAIN
		v.TStamp = stod(row[0]); // TStamp updated separately from fill_data_var for compatibility w/ binary files
            }
            fill_data_var(row, acq_mod, v, hit_frag_timing[board_now]);
            //if (hit>=v.hits){throw runtime_error("Something went wrong with the counting of the number of hits.");}
            hit++;  
            hit_tot++;
            hit_frag_timing[board_now]++;      
        }
    } 
    
    tr_data->Fill();    // FILL TREE TO SAVE THE LAST EVENT!

    return 0;
}
