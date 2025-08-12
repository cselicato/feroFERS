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

    // start reading file
    // read file header (exactly the same for each acquisition mode)
    file.read(reinterpret_cast<char*>(&fh), sizeof(FHEADER));
    cout << "file_format: "
                << (int)fh.file_format[0] << " "
                << (int)fh.file_format[1] << endl;
    cout << "janus_rel: "
                << (int)fh.janus_rel[0] << " "
                << (int)fh.janus_rel[1] << " "
                << (int)fh.janus_rel[2] << endl;
    cout << "board_mod: " << fh.board_mod << endl;
    cout << "board_mod in hex: " <<hex<< fh.board_mod<<dec << endl;

    cout << "run: " << fh.run << endl;
    cout << "acq_mode: " << (int)fh.acq_mode << endl;
    cout << "e_Nbins: " << fh.e_Nbins << endl;
    cout << "time_unit: " << (int)fh.time_unit << endl;
    cout << "time_conv: " << fh.time_conv << endl;
    cout << "start_acq: " << fh.time_epoch << endl;
    cout << "acq_mode: " << (int)fh.acq_mode << endl;
    cout << endl;
    modes acq_mod = find_mode(fh.acq_mode);

    // read the events
    streampos ev_start;

    float ToA_ns;
    float ToT_ns;
    uint32_t ToA_LSB;
    uint16_t ToT_LSB;


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

                    cout << "ch_ID: " << (int)event.ch_ID << '\n';
                    cout << "data_type (PHA mode): " << (int)event.data_type << '\n';
                    if(event.data_type&0x1){
                        file.read(reinterpret_cast<char*>(&v.LG[eh.board_ID][event.ch_ID]), 2);
                        cout << "LG: " << to_string(v.LG[eh.board_ID][event.ch_ID]) << '\n';
                    }
                    if(event.data_type&0x2){
                        file.read(reinterpret_cast<char*>(&v.HG[eh.board_ID][event.ch_ID]), 2);
                        cout << "HG: " << to_string(v.HG[eh.board_ID][event.ch_ID]) << '\n';
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
                cout << "T_ref: " << to_string(t_eh.TStamp) << '\n';
                // READ EVENT DATA
                int hits = 0; 
                while (file.tellg()<(t_eh.ev_size+ev_start)){
                    file.read(reinterpret_cast<char*>(&event), sizeof(EDATA));
                    cout << "-------------------------------"<<endl;
                    cout << "ch_ID: " << (int)event.ch_ID << '\n';
                    cout << "data_type in hex (Timing mode): " << hex<<(int)event.data_type <<dec<< '\n';

                    // !! FIX BUG (multiple hits per channel)
                    if(fh.time_unit&0x1){ // times are saved as ns
                        cout << "Tempi in ns:" <<endl;                        
                        if(event.data_type&0x10){ // ToA saved
                            file.read(reinterpret_cast<char*>(&ToA_ns), sizeof(float));
                            v.ToA_timing[t_eh.board_ID][event.ch_ID][0] = ToA_ns;
                            cout << "ToA: " << get<float>(v.ToA_timing[t_eh.board_ID][event.ch_ID][0]) << '\n';

                        }
                        if(event.data_type&0x20){ // ToT saved
                            file.read(reinterpret_cast<char*>(&ToT_ns), sizeof(float));
                            v.ToT_timing[t_eh.board_ID][event.ch_ID][0] = ToT_ns;
                            cout << "ToT: " << get<float>(v.ToT_timing[t_eh.board_ID][event.ch_ID][0]) << '\n';
                        }
                    }
                    else{ // times are saved as LSB
                        cout << "Tempi in LSB:" <<endl;
                        if(event.data_type&0x10){ // ToA saved
                            file.read(reinterpret_cast<char*>(&ToA_LSB), sizeof(uint32_t));
                            v.ToA_timing[t_eh.board_ID][event.ch_ID][0] = ToA_LSB;
                            cout << "ToA: " << get<uint32_t>(v.ToA_timing[t_eh.board_ID][event.ch_ID][0]) << '\n';
                        }
                        if(event.data_type&0x20){ // ToT saved
                            file.read(reinterpret_cast<char*>(&ToT_LSB), sizeof(uint16_t));
                            v.ToT_timing[t_eh.board_ID][event.ch_ID][0] = ToT_LSB;
                            cout << "ToT: " << get<uint16_t>(v.ToT_timing[t_eh.board_ID][event.ch_ID][0]) << '\n';
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
                cout << endl;
                cout << "TStamp: " << to_string(eh.TStamp) << '\n';
                // READ EVENT DATA
                int hits = 0;
                while (file.tellg()<(eh.ev_size+ev_start)){
                    file.read(reinterpret_cast<char*>(&event), sizeof(EDATA));
                    // PHA information
                    cout << "ch_ID: " << (int)event.ch_ID << '\n';
                    cout << "data_type (PHA+Timing mode) in hex: " << hex<<(int)event.data_type <<dec<< '\n';

                    if(event.data_type&0x1){
                        file.read(reinterpret_cast<char*>(&v.LG[eh.board_ID][event.ch_ID]), 2);
                        cout << "LG: " << to_string(v.LG[eh.board_ID][event.ch_ID]) << '\n';
                    }
                    if(event.data_type&0x2){
                        cout <<"!!!!"<<endl;
                        file.read(reinterpret_cast<char*>(&v.HG[eh.board_ID][event.ch_ID]), 2);
                        cout << "HG: " << to_string(v.HG[eh.board_ID][event.ch_ID]) << '\n';
                    }
                    // timing information
                    if(fh.time_unit&0x1){ // times are saved as ns
                        if(event.data_type&0x10){ // ToA saved
                            cout <<"!!!!"<<endl;
                            float ToA_ns = 0;
                            file.read(reinterpret_cast<char*>(&ToA_ns), sizeof(float));
                            v.ToA_timing[t_eh.board_ID][event.ch_ID][0] = ToA_ns;
                            cout << "ToA: " << get<float>(v.ToA_timing[t_eh.board_ID][event.ch_ID][0]) << '\n';
                        }
                        if(event.data_type&0x20){ // ToT saved
                            float ToT_ns = 0;
                            file.read(reinterpret_cast<char*>(&ToT_ns), sizeof(float));
                            v.ToT_timing[t_eh.board_ID][event.ch_ID][0] = ToT_ns;
                            cout << "ToT: " << get<float>(v.ToT_timing[t_eh.board_ID][event.ch_ID][0]) << '\n';
                        }
                    }
                    else{ // times are saved as LSB
                        if(event.data_type&0x10){ // ToA saved
                            uint32_t ToA = 0;
                            file.read(reinterpret_cast<char*>(&ToA), sizeof(uint32_t));
                            v.ToA_timing[t_eh.board_ID][event.ch_ID][0] = ToA;
                            cout << "ToA: " << get<uint32_t>(v.ToA_timing[t_eh.board_ID][event.ch_ID][0]) << '\n';
                        }
                        if(event.data_type&0x20){ // ToT saved
                            uint16_t ToT = 0;
                            file.read(reinterpret_cast<char*>(&ToT), sizeof(uint16_t));
                            v.ToT_timing[t_eh.board_ID][event.ch_ID][0] = ToT;
                            cout << "ToT: " << get<uint16_t>(v.ToT_timing[t_eh.board_ID][event.ch_ID][0]) << '\n';
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
                    uint8_t ch_ID;
                    uint64_t counts;

                    file.read(reinterpret_cast<char*>(&ch_ID), sizeof(uint8_t));
                    file.read(reinterpret_cast<char*>(&counts), sizeof(uint64_t));
                    hits++;
                }
                cout << endl;
            }
            break;
        }

    return 0;
};

int main(){
    get_bin_data("test_files/PHA_timing/Run25_list.dat");

    return 0;
}