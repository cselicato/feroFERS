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

            break;

        case modes::Spect_Timing:
            v.e_Nbins = stoi(metadata[4][1]);
            v.time_conv = stod(metadata[5][1]);
            v.time_unit = metadata[6][1];
            v.run = stoi(metadata[7][1]);
            v.time_epoch = stoul(metadata[8][1]);
            v.time_UTC = metadata[9][1]+":"+metadata[9][2]+":"+metadata[9][3];

            break;

        case modes::Timing:
            v.time_conv = stod(metadata[4][1]);
            v.time_unit = metadata[5][1];
            v.run = stoi(metadata[6][1]);
            v.time_epoch = stoul(metadata[6][1]);
            v.time_UTC = metadata[8][1]+":"+metadata[8][2]+":"+metadata[8][3];  

            break;

        case modes::Counting:
            v.run = stoi(metadata[4][1]);
            v.time_epoch = stoul(metadata[5][1]);
            v.time_UTC = metadata[6][1]+":"+metadata[6][2]+":"+metadata[6][3];  

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
            v.hits = stoi(row[3]);
            v.ch_mask = stoull(row[4], nullptr, 16);
            
            ch_ID = stoi(row[5]);
            board= stoi(row[2]);

            is_valid_ind(board, ch_ID);

            v.data_type[board][ch_ID] = stoi(row[6], nullptr, 16);
            v.LG[board][ch_ID] = stoi(row[7]);
            v.HG[board][ch_ID] = stoi(row[8]);      

            break;

        case modes::Timing:
            v.TStamp = stod(row[0]);
            v.hits = stoi(row[2]);

            board= stoi(row[1]);
            ch_ID = stoi(row[3]);    
            
            is_valid_ind(board, ch_ID);

            v.data_type_timing[board][ch_ID][hit] = stoi(row[4], nullptr, 16);
            v.ToA_timing[board][ch_ID][hit] = stof(row[5]);  
            v.ToT_timing[board][ch_ID][hit] = stof(row[6]);  

            if (hit>=v.hits){cout << "Something went wrong..."<<endl;}
            hit++;
            break;            

        case modes::Spect_Timing:
            v.Trg_Id = stoull(row[1]);
            v.ch_mask = stoull(row[4], nullptr, 16);
            v.TStamp = stod(row[0]);
            v.hits = stoi(row[3]);

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
            v.hits = stoi(row[3]);

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
    float old_TStamp = -1;
    float new_TStamp = 0;

    int hit=0;
    while (getline(file, line)){
        if (line.empty()==false){
            
            if (line.back() == '\r'){ // if present, remove CR
                line.pop_back();    }

            vector<string> row = split_line(line, ',');
            is_consistent(row.size(), exp_size);

            new_TStamp = stof(row[0]); 
            
            if (new_TStamp != old_TStamp){ // event is different OR first one, update old_TStamp
                if (old_TStamp != -1){ // event is different: fill tree and LATER fill v
                    tr_data->Fill();
                    hit = 0;
                }
                old_TStamp = new_TStamp;
                reset_stored_vars(v, acq_mod);  // RESET v and hit BEFORE FILLING AGAIN

            }
            fill_data_var(row, acq_mod, v, hit);
            if (hit>=v.hits){throw runtime_error("Something went wrong with the counting of the number of hits.");}
            hit++;
        
            
        }
    } 
    
    tr_data->Fill();    // FILL TREE TO SAVE THE LAST EVENT!

    return 0;
}
