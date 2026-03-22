#include "modes_helpers.hpp"
#include "bin_parser.hpp"


// function to fill the variable stored_vars v for the binary file case
void fill_info_var(FHEADER &fh, stored_vars &v, modes &mode){
    // fill v with the metadata cointained in the file header
    v.board_mod = fh.board_mod;
    v.file_format = to_string(fh.file_format[0])+"."+to_string(fh.file_format[1]);
    v.janus_rel = to_string(fh.janus_rel[0])+"."+to_string(fh.janus_rel[1])+"."+to_string(fh.janus_rel[2]);
    switch (mode) {
        case modes::Spectroscopy:
            v.acq_mode = "Spectroscopy";
            break;
        case modes::Spect_Timing:
            v.acq_mode = "Spect_Timing";
            break;
        case modes::Timing:
            v.acq_mode = "Timing";
            break;
        case modes::Counting:
            v.acq_mode = "Counting";
            break;
    }
    v.run = fh.run;
    v.e_Nbins = fh.e_Nbins;
    v.time_epoch = fh.time_epoch;
    time_t sec_epoch = v.time_epoch/1000; // convert to seconds
    v.time_UTC = new TTimeStamp(sec_epoch);    
    v.time_conv = fh.time_conv;
    if(fh.time_unit&0x1){
        v.time_unit = "ns";   
    }
    else {v.time_unit = "LSB";}
}

// PHA and PHA+Timing and counting
void fill_data_var(EHEADER &eh, stored_vars &v){
    v.TStamp = eh.TStamp;    
    v.Trg_Id = eh.Trg_Id;   
    v.ch_mask = eh.ch_mask; 
}

int parse_bin(string inFile, bool isNotFileHeader, string inFileInfo, TTree * tr_info, TTree * tr_data, stored_vars &v){

    bool isFileHeader = not isNotFileHeader;
  
    // open file
    fstream file(inFile, ios::in|ios::binary|ios::ate);
    if (!file) {
        throw runtime_error("Failed to open file.");
    }
    // find file size and go back to the beginning
    Int_t file_size = file.tellg();
    file.seekg(0, ios::beg);

    FHEADER fh;
    EHEADER eh;
    T_EHEADER t_eh;
    EDATA event;
    read_vars r;
    
    // start reading file
    // read file header (exactly the same for each acquisition mode)
    modes acq_mod;
    TTree* tr_info_ref;
    TFile * fileinfo = nullptr;
    TString * leaf_info_ref = nullptr;
    if(isFileHeader){
        file.read(reinterpret_cast<char*>(&fh), sizeof(FHEADER));
        acq_mod = find_mode(fh.acq_mode);
    }else{
        fileinfo = new TFile(inFileInfo.c_str(), "read");
        if (!fileinfo || fileinfo->IsZombie()) {
            cerr << "Unable to open ROOT info reference file." << endl;
            return 0;
        }
	tr_info_ref=(TTree*)fileinfo->Get("info");
	tr_info_ref->SetBranchAddress("acq_mode", &leaf_info_ref);
	tr_info_ref->GetEntry(0);
        acq_mod = find_mode( *leaf_info_ref );
    }
	
    // make trees' branches
    stored_vars v_ref;
    if(isFileHeader){
        make_branches_info(tr_info, acq_mod, v);
    }else{
        make_branches_info(tr_info, acq_mod, v_ref);
	open_branches_info(tr_info_ref, acq_mod, v_ref);
    }
    make_branches_data(tr_data, acq_mod, v);

    // fill variables that need to be stored in the info tree
    if(isFileHeader){
        fill_info_var(fh, v, acq_mod);
        tr_info->Fill();
    }else{
	tr_info_ref->GetEntry(0);
	v_ref.acq_mode= *leaf_info_ref;
	tr_info->Fill();
	fileinfo->Close();
    }
	
    // read the events
    streampos ev_start;

    int ifrag = 0;
    int old_Trg_Id = -1;
    float TStamp_cut = (fh.time_unit&0x1) ? 1 : 2 ; // TStamp separation threshold in ns : LSB;
    float old_TStamp = -2*TStamp_cut; // only used in timing, other modes use Trg_Id
    
    int hits, hits_frag_timing;

    switch (acq_mod) {
        case modes::Spectroscopy:
            while(file.tellg()<file_size){
                ev_start = file.tellg();
                // READ EVENT HEADER
                file.read(reinterpret_cast<char*>(&eh), sizeof(EHEADER));
                
                // if there is a new Trg_Id...
                if (eh.Trg_Id!=old_Trg_Id){
                    
                    // if this is not the first event read, finalise the stored_data object and fill the tree
                    if (ifrag>0){
                        v.hits = hits;
                        tr_data->Fill();
                    }
                    
                    // in any case, updateold_Trg_Id, reset hit count and update/reset various datas entries
                    old_Trg_Id = eh.Trg_Id;
                    hits = 0;
                    reset_stored_vars(v, acq_mod);
                    fill_data_var(eh, v);
                }

                // READ EVENT DATA
                while (file.tellg()<(eh.ev_size+ev_start)){
                    file.read(reinterpret_cast<char*>(&event), sizeof(EDATA));

                    is_valid_ind(eh.board_Id, event.ch_Id);

                    v.data_type[eh.board_Id][event.ch_Id] = event.data_type;

                    if(event.data_type&0x1){ // LG amplitude saved
                        file.read(reinterpret_cast<char*>(&r.LG), sizeof(r.LG));
                        v.LG[eh.board_Id][event.ch_Id] = r.LG;
                    }
                    if(event.data_type&0x2){ // HG amplitude saved
                        file.read(reinterpret_cast<char*>(&r.HG), sizeof(r.HG));
                        v.HG[eh.board_Id][event.ch_Id] = r.HG;
                    }
                    hits++;
                }
                ifrag++;
            }
            
            //special update/fill for last event
            v.hits = hits;
            tr_data->Fill();
            
            break;

        case modes::Timing:
            while(file.tellg()<file_size){
                ev_start = file.tellg();
                // READ EVENT HEADER
                file.read(reinterpret_cast<char*>(&t_eh), sizeof(T_EHEADER));

                // if there is a "new" TStamp (difference within TStamp_cut set above)...
                if ((t_eh.TStamp-old_TStamp)*(t_eh.TStamp-old_TStamp)>TStamp_cut*TStamp_cut){
                    
                    // if this is not the first event read, finalise the stored_data object and fill the tree
                    if (ifrag>0){
                        v.hits = hits;
                        tr_data->Fill();
                    }
                    
                    // in any case, updateold_Trg_Id, reset hit count and update/reset various datas entries
                    old_TStamp = t_eh.TStamp;
                    hits = 0;
                    reset_stored_vars(v, acq_mod);
                    v.TStamp = t_eh.TStamp;
                }

                // READ EVENT DATA
                int hits_frag_timing = 0;
                while (file.tellg()<(t_eh.ev_size+ev_start)){
                    file.read(reinterpret_cast<char*>(&event), sizeof(EDATA));
                    is_valid_ind(t_eh.board_Id, event.ch_Id);

                    v.data_type_timing[t_eh.board_Id][event.ch_Id][hits_frag_timing] = (int16_t)event.data_type;

                    if(fh.time_unit&0x1){ // times are saved as ns
                        if(event.data_type&0x10){ // ToA saved
                            file.read(reinterpret_cast<char*>(&r.ToA_ns), sizeof(r.ToA_ns));
                            v.ToA_timing[t_eh.board_Id][event.ch_Id][hits_frag_timing] = r.ToA_ns;
                        }
                        if(event.data_type&0x20){ // ToT saved
                            file.read(reinterpret_cast<char*>(&r.ToT_ns), sizeof(r.ToT_ns));
                            v.ToT_timing[t_eh.board_Id][event.ch_Id][hits_frag_timing] = r.ToT_ns;
                        }
                    }
                    else{ // times are saved as LSB
                        if(event.data_type&0x10){ // ToA saved
                            file.read(reinterpret_cast<char*>(&r.ToA_LSB), sizeof(r.ToA_LSB));
                            v.ToA_timing[t_eh.board_Id][event.ch_Id][hits_frag_timing] = r.ToA_LSB;
                        }
                        if(event.data_type&0x20){ // ToT saved
                            file.read(reinterpret_cast<char*>(&r.ToT_LSB), sizeof(r.ToT_LSB));
                            v.ToT_timing[t_eh.board_Id][event.ch_Id][hits_frag_timing] = r.ToT_LSB;
                        }
                    }
                    
                    // if (hits>=v.hits){throw runtime_error("Something went wrong with the counting of the number of hits.");}
                    hits++;
                    hits_frag_timing++;
                }
                ifrag++;
            }
            
            //special update/fill for last event
            v.hits = hits;
            tr_data->Fill();

            break;

        case modes::Spect_Timing:
            while(file.tellg()<file_size){
                ev_start = file.tellg();
                // READ EVENT HEADER
                file.read(reinterpret_cast<char*>(&eh), sizeof(EHEADER));

                // if there is a new Trg_Id...
                if (eh.Trg_Id!=old_Trg_Id){
                    
                    // if this is not the first event read, finalise the stored_data object and fill the tree
                    if (ifrag>0){
                        v.hits = hits;
                        tr_data->Fill();
                    }
                    
                    // in any case, updateold_Trg_Id, reset hit count and update/reset various datas entries
                    old_Trg_Id = eh.Trg_Id;
                    hits = 0;
                    reset_stored_vars(v, acq_mod);
                    fill_data_var(eh, v);
                }

                // READ EVENT DATA
                while (file.tellg()<(eh.ev_size+ev_start)){
                    file.read(reinterpret_cast<char*>(&event), sizeof(EDATA));

                    is_valid_ind(eh.board_Id, event.ch_Id);

                    v.data_type[eh.board_Id][event.ch_Id] = event.data_type;
                       
                    // PHA information
                    if(event.data_type&0x1){ // LG amplitude saved
                        file.read(reinterpret_cast<char*>(&r.LG), sizeof(r.LG));
                        v.LG[eh.board_Id][event.ch_Id] = (int32_t)r.LG;
                    }
                    if(event.data_type&0x2){ // HG amplitude saved
                        file.read(reinterpret_cast<char*>(&r.HG), sizeof(r.HG));
                        v.HG[eh.board_Id][event.ch_Id] = (int32_t)r.HG;
                    }
                    
                    // timing information
                    if(fh.time_unit&0x1){ // times are saved as ns
                        if(event.data_type&0x10){ // ToA saved
                            file.read(reinterpret_cast<char*>(&r.ToA_ns), sizeof(float));
                            v.ToA[eh.board_Id][event.ch_Id] = (float)r.ToA_ns;

                        }
                        if(event.data_type&0x20){ // ToT saved
                            file.read(reinterpret_cast<char*>(&r.ToT_ns), sizeof(float));
                            v.ToT[eh.board_Id][event.ch_Id] = (float)r.ToT_ns;
                        }
                    }
                    else{ // times are saved as LSB
                        if(event.data_type&0x10){ // ToA saved
                            file.read(reinterpret_cast<char*>(&r.ToA_LSB), sizeof(r.ToA_LSB));
                            v.ToA[eh.board_Id][event.ch_Id] = (float)r.ToA_LSB;
                        }
                        if(event.data_type&0x20){ // ToT saved
                            file.read(reinterpret_cast<char*>(&r.ToT_LSB), sizeof(r.ToT_LSB));
                            v.ToT[eh.board_Id][event.ch_Id] = (float)r.ToT_LSB;
                        }
                    }
                    hits++;
                }
                ifrag++;
            }
            
		    //special update/fill for last event
            v.hits = hits;
            tr_data->Fill();
            
            break;

        case modes::Counting:
            while(file.tellg()<file_size){
                ev_start = file.tellg();
                // READ EVENT HEADER
                file.read(reinterpret_cast<char*>(&eh), sizeof(EHEADER));
                // READ EVENT DATA
                hits = 0;
                reset_stored_vars(v, acq_mod);

                while (file.tellg()<(eh.ev_size+ev_start)){
                    file.read(reinterpret_cast<char*>(&r.ch_Id), sizeof(uint8_t));
                    file.read(reinterpret_cast<char*>(&r.counts), sizeof(uint64_t));

                    is_valid_ind(eh.board_Id, event.ch_Id);

                    v.counts[eh.board_Id][r.ch_Id] = (int64_t)r.counts;
                    hits++;
                }

                fill_data_var(eh, v);
                v.hits = hits;

                tr_data->Fill();
            }
            break;
        }

    return 0;
};
