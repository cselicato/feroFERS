#include "modes_helpers.hpp"
#include "bin_parser.hpp"

#include "csv_parser.hpp"


void fill_info_var(FHEADER &fh, stored_vars &v){
    // fill v with the metadata cointained in the file header
    v.board_mod = fh.board_mod;
    v.file_format = to_string(fh.file_format[0])+"."+to_string(fh.file_format[1]);
    v.janus_rel = to_string(fh.janus_rel[0])+"."+to_string(fh.janus_rel[1])+"."+to_string(fh.janus_rel[2]);
    switch (find_mode(fh.acq_mode)) {
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
    v.time_epoch = fh.time_epoch ;
    v.time_conv = fh.time_conv;
    // v.time_UTC = fh. ;
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

int get_bin_data(string inFile, string outFile, stored_vars &v){
    // open file
    fstream file(inFile, ios::in|ios::binary|ios::ate);
    if (!file) {
        cerr << "Failed to open file\n";
        return 1;
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
    file.read(reinterpret_cast<char*>(&fh), sizeof(FHEADER));
    modes acq_mod = find_mode(fh.acq_mode);
    if(fh.time_unit&0x1){v.time_unit = "ns";}
    else {v.time_unit = "LSB";}

    // make trees 
    TTree *tr_info = new TTree("info","info");
    TTree *tr_data = new TTree("datas","datas");
    make_branches_info(tr_info, acq_mod, v);
    make_branches_data(tr_data, acq_mod, v);

    // fill variables that need to be stored in the info tree
    fill_info_var(fh, v);
    tr_info->Fill();

    // read the events
    streampos ev_start;

    switch (acq_mod) {
        case modes::Spectroscopy:
            while(file.tellg()<file_size){
                ev_start = file.tellg();
                // READ EVENT HEADER
                file.read(reinterpret_cast<char*>(&eh), sizeof(EHEADER));
                
                int hits = 0;
                reset<int16_t>(v.data_type, v);
                reset<int32_t>(v.LG, v);
                reset<int32_t>(v.HG, v);

                // READ EVENT DATA
                while (file.tellg()<(eh.ev_size+ev_start)){
                    file.read(reinterpret_cast<char*>(&event), sizeof(EDATA));

                    v.data_type[eh.board_Id][event.ch_ID] = event.data_type;

                    if(event.data_type&0x1){ // LG amplitude saved
                        file.read(reinterpret_cast<char*>(&r.LG), sizeof(r.LG));
                        v.LG[eh.board_Id][event.ch_ID] = r.LG;
                    }
                    if(event.data_type&0x2){ // HG amplitude saved
                        file.read(reinterpret_cast<char*>(&r.HG), sizeof(r.HG));
                        v.HG[eh.board_Id][event.ch_ID] = r.HG;
                    }
                    hits++;
                }
                fill_data_var(eh, v);
                v.hits = hits;

                tr_data->Fill();
            }
            break;

        case modes::Timing:
            while(file.tellg()<file_size){
                ev_start = file.tellg();
                // READ EVENT HEADER
                file.read(reinterpret_cast<char*>(&t_eh), sizeof(T_EHEADER));

                int hits = 0; 
                reset<int16_t>(v.data_type, v);
                // reset<float>(v.ToT_timing, v);
                // reset<float>(v.ToA_timing, v);
                reset<float>(v.ToT, v);
                reset<float>(v.ToA, v);

                // READ EVENT DATA
                while (file.tellg()<(t_eh.ev_size+ev_start)){
                    file.read(reinterpret_cast<char*>(&event), sizeof(EDATA));

                    v.data_type[eh.board_Id][event.ch_ID] = event.data_type;

                    // !! FIX BUG (multiple hits per channel)
                    if(fh.time_unit&0x1){ // times are saved as ns
                        if(event.data_type&0x10){ // ToA saved
                            file.read(reinterpret_cast<char*>(&r.ToA_ns), sizeof(r.ToA_ns));
                            // v.ToA_timing[t_eh.board_ID][event.ch_ID][0] = r.ToA_ns;
                            v.ToA[t_eh.board_ID][event.ch_ID] = r.ToA_ns;

                        }
                        if(event.data_type&0x20){ // ToT saved
                            file.read(reinterpret_cast<char*>(&r.ToT_ns), sizeof(r.ToT_ns));
                            // v.ToT_timing[t_eh.board_ID][event.ch_ID][0] = r.ToT_ns;
                            v.ToT[t_eh.board_ID][event.ch_ID] = r.ToT_ns;
                        }
                    }
                    else{ // times are saved as LSB
                        if(event.data_type&0x10){ // ToA saved
                            file.read(reinterpret_cast<char*>(&r.ToA_LSB), sizeof(r.ToA_LSB));
                            // v.ToA_timing[t_eh.board_ID][event.ch_ID][0] = r.ToA_LSB;
                            v.ToA[t_eh.board_ID][event.ch_ID] = r.ToA_LSB;
                        }
                        if(event.data_type&0x20){ // ToT saved
                            file.read(reinterpret_cast<char*>(&r.ToT_LSB), sizeof(r.ToT_LSB));
                            // v.ToT_timing[t_eh.board_ID][event.ch_ID][0] = r.ToT_LSB;
                            v.ToT[t_eh.board_ID][event.ch_ID] = r.ToT_LSB;
                        }
                    }

                    hits++;
                }

                v.TStamp = t_eh.TStamp;
                v.hits = hits;

                tr_data->Fill();
            }
            break;

        case modes::Spect_Timing:
            while(file.tellg()<file_size){
                ev_start = file.tellg();
                // READ EVENT HEADER
                file.read(reinterpret_cast<char*>(&eh), sizeof(EHEADER));

                int hits = 0;
                reset<int16_t>(v.data_type, v);
                reset<int32_t>(v.LG, v);
                reset<int32_t>(v.HG, v);
                reset<float>(v.ToT_timing, v);
                reset<float>(v.ToA_timing, v);
                reset<float>(v.ToT, v);
                reset<float>(v.ToA, v);

                // READ EVENT DATA
                while (file.tellg()<(eh.ev_size+ev_start)){
                    file.read(reinterpret_cast<char*>(&event), sizeof(EDATA));

                    v.data_type[eh.board_Id][event.ch_ID] = event.data_type;
                       
                    // PHA information
                    if(event.data_type&0x1){ // LG amplitude saved
                        file.read(reinterpret_cast<char*>(&r.LG), sizeof(r.LG));
                        v.LG[eh.board_Id][event.ch_ID] = (int32_t)r.LG;
                    }
                    if(event.data_type&0x2){ // HG amplitude saved
                        file.read(reinterpret_cast<char*>(&r.HG), sizeof(r.HG));
                        v.HG[eh.board_Id][event.ch_ID] = (int32_t)r.HG;
                    }
                    
                    // timing information
                    if(fh.time_unit&0x1){ // times are saved as ns
                        if(event.data_type&0x10){ // ToA saved
                            file.read(reinterpret_cast<char*>(&r.ToA_ns), sizeof(float));
                            // v.ToA_timing[eh.board_Id][event.ch_ID][0] = (float)r.ToA_ns;
                            v.ToA[eh.board_Id][event.ch_ID] = (float)r.ToA_ns;

                        }
                        if(event.data_type&0x20){ // ToT saved
                            file.read(reinterpret_cast<char*>(&r.ToT_ns), sizeof(float));
                            // v.ToT_timing[eh.board_Id][event.ch_ID][0] = (float)r.ToT_ns;
                            v.ToT[eh.board_Id][event.ch_ID] = (float)r.ToT_ns;
                        }
                    }
                    else{ // times are saved as LSB
                        if(event.data_type&0x10){ // ToA saved
                            file.read(reinterpret_cast<char*>(&r.ToA_LSB), sizeof(r.ToA_LSB));
                            // v.ToA_timing[eh.board_Id][event.ch_ID][0] = (float)r.ToA_LSB;
                            v.ToA[eh.board_Id][event.ch_ID] = (float)r.ToA_LSB;
                        }
                        if(event.data_type&0x20){ // ToT saved
                            file.read(reinterpret_cast<char*>(&r.ToT_LSB), sizeof(r.ToT_LSB));
                            // v.ToT_timing[eh.board_Id][event.ch_ID][0] = (float)r.ToT_LSB;
                            v.ToT[eh.board_Id][event.ch_ID] = (float)r.ToT_LSB;
                        }
                    }
                    hits++;
                }
                fill_data_var(eh, v);
                v.hits = hits;

                tr_data->Fill();
            }
            break;

        case modes::Counting:
            while(file.tellg()<file_size){
                ev_start = file.tellg();
                // READ EVENT HEADER
                file.read(reinterpret_cast<char*>(&eh), sizeof(EHEADER));
                // READ EVENT DATA
                int hits = 0;
                reset<int64_t>(v.counts, v);

                while (file.tellg()<(eh.ev_size+ev_start)){
                    file.read(reinterpret_cast<char*>(&r.ch_ID), sizeof(uint8_t));
                    file.read(reinterpret_cast<char*>(&r.counts), sizeof(uint64_t));
                    v.counts[eh.board_Id][r.ch_ID] = (int64_t)r.counts;
                    hits++;
                }

                fill_data_var(eh, v);
                v.hits = hits;

                tr_data->Fill();
            }
            break;
        }


    TFile fout(outFile.c_str(), "recreate");
    tr_data->Write();
    tr_info->Write();
    // close file
    fout.Close();

    return 0;
};