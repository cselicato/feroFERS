#include "bin_parser.hpp"
#include "modes_helpers.hpp"


int get_bin_data(string inFile){
    // open file
    fstream file(inFile, ios::in|ios::binary|ios::ate);
    if (!file) {
        cerr << "Failed to open file\n";
        return 1;
    }
    // find file size and go back to the beginning
    streampos file_size = file.tellg();
    file.seekg(0, ios::beg);

    FHEADER fh;
    EHEADER eh;
    T_EHEADER t_eh;
    EDATA event;
    stored_vars_b v(1,100);
    read_vars r;

    // start reading file
    // read file header (exactly the same for each acquisition mode)
    file.read(reinterpret_cast<char*>(&fh), sizeof(FHEADER));
    modes acq_mod = find_mode(fh.acq_mode);

    // read the events
    streampos ev_start;

    switch (acq_mod) {
        case modes::Spectroscopy:
            while(file.tellg()<file_size){
                ev_start = file.tellg();
                // READ EVENT HEADER
                file.read(reinterpret_cast<char*>(&eh), sizeof(EHEADER));
                // READ EVENT DATA
                int hits = 0;
                while (file.tellg()<(eh.ev_size+ev_start)){
                    file.read(reinterpret_cast<char*>(&event), sizeof(EDATA));
                    if(event.data_type&0x1){ // LG amplitude saved
                        file.read(reinterpret_cast<char*>(&r.LG), sizeof(r.LG));
                        v.LG[eh.board_ID][event.ch_ID] = r.LG;
                    }
                    if(event.data_type&0x2){ // HG amplitude saved
                        file.read(reinterpret_cast<char*>(&r.HG), sizeof(r.HG));
                        v.HG[eh.board_ID][event.ch_ID] = r.HG;
                    }
                    hits++;
                }
            }
            break;

        case modes::Timing:
            while(file.tellg()<file_size){
                ev_start = file.tellg();
                // READ EVENT HEADER
                file.read(reinterpret_cast<char*>(&t_eh), sizeof(T_EHEADER));
                // READ EVENT DATA
                int hits = 0; 
                while (file.tellg()<(t_eh.ev_size+ev_start)){
                    file.read(reinterpret_cast<char*>(&event), sizeof(EDATA));
                    // !! FIX BUG (multiple hits per channel)
                    if(fh.time_unit&0x1){ // times are saved as ns
                        if(event.data_type&0x10){ // ToA saved
                            file.read(reinterpret_cast<char*>(&r.ToA_ns), sizeof(r.ToA_ns));
                            v.ToA_timing[t_eh.board_ID][event.ch_ID][0] = r.ToA_ns;
                        }
                        if(event.data_type&0x20){ // ToT saved
                            file.read(reinterpret_cast<char*>(&r.ToT_ns), sizeof(r.ToT_ns));
                            v.ToT_timing[t_eh.board_ID][event.ch_ID][0] = r.ToT_ns;
                        }
                    }
                    else{ // times are saved as LSB
                        if(event.data_type&0x10){ // ToA saved
                            file.read(reinterpret_cast<char*>(&r.ToA_LSB), sizeof(r.ToA_LSB));
                            v.ToA_timing[t_eh.board_ID][event.ch_ID][0] = r.ToA_LSB;
                        }
                        if(event.data_type&0x20){ // ToT saved
                            file.read(reinterpret_cast<char*>(&r.ToT_LSB), sizeof(r.ToT_LSB));
                            v.ToT_timing[t_eh.board_ID][event.ch_ID][0] = r.ToT_LSB;
                        }
                    }

                    hits++;
                }
            }
            break;

        case modes::Spect_Timing:
            while(file.tellg()<file_size){
                ev_start = file.tellg();
                // READ EVENT HEADER
                file.read(reinterpret_cast<char*>(&eh), sizeof(EHEADER));
                // READ EVENT DATA
                int hits = 0;
                while (file.tellg()<(eh.ev_size+ev_start)){
                    file.read(reinterpret_cast<char*>(&event), sizeof(EDATA));
                    // PHA information
                    if(event.data_type&0x1){
                    if(event.data_type&0x1){ // LG amplitude saved
                        file.read(reinterpret_cast<char*>(&r.LG), sizeof(r.LG));
                        v.LG[eh.board_ID][event.ch_ID] = r.LG;
                    }
                    if(event.data_type&0x2){ // HG amplitude saved
                        file.read(reinterpret_cast<char*>(&r.HG), sizeof(r.HG));
                        v.HG[eh.board_ID][event.ch_ID] = r.HG;
                    }
                    // timing information
                    if(fh.time_unit&0x1){ // times are saved as ns
                        if(event.data_type&0x10){ // ToA saved
                            float ToA_ns = 0;
                            file.read(reinterpret_cast<char*>(&ToA_ns), sizeof(float));
                            v.ToA_timing[t_eh.board_ID][event.ch_ID][0] = ToA_ns;
                        }
                        if(event.data_type&0x20){ // ToT saved
                            float ToT_ns = 0;
                            file.read(reinterpret_cast<char*>(&ToT_ns), sizeof(float));
                            v.ToT_timing[t_eh.board_ID][event.ch_ID][0] = ToT_ns;
                        }
                    }
                    else{ // times are saved as LSB
                        if(event.data_type&0x10){ // ToA saved
                            file.read(reinterpret_cast<char*>(&r.ToA_LSB), sizeof(r.ToA_LSB));
                            v.ToA_timing[t_eh.board_ID][event.ch_ID][0] = r.ToA_LSB;
                        }
                        if(event.data_type&0x20){ // ToT saved
                            file.read(reinterpret_cast<char*>(&r.ToT_LSB), sizeof(r.ToT_LSB));
                            v.ToT_timing[t_eh.board_ID][event.ch_ID][0] = r.ToT_LSB;
                        }
                    }

                    }
                    hits++;
                }
            }
            break;

        case modes::Counting:
            while(file.tellg()<file_size){
                ev_start = file.tellg();
                // READ EVENT HEADER
                file.read(reinterpret_cast<char*>(&eh), sizeof(EHEADER));
                // READ EVENT DATA
                int hits = 0;
                while (file.tellg()<(eh.ev_size+ev_start)){
                    file.read(reinterpret_cast<char*>(&r.ch_ID), sizeof(uint8_t));
                    file.read(reinterpret_cast<char*>(&r.counts), sizeof(uint64_t));
                    hits++;
                }
            }
            break;
        }

    return 0;
};

// int main(){
    // get_bin_data("test_files/PHA_timing/Run25_list.dat");
// 
    // return 0;
// }