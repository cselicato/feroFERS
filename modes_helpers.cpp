#include "modes_helpers.hpp"

void is_valid_ind(int board,int ch){
    if((board>=NBOARDS)||(ch>=NCHANNELS)){
        cout << "Board ID " << board << " and ch. ID " << ch << " are invalid."<< endl;
        throw runtime_error("Found invalid board ID or channel ID, unable to produce root file.");
    }
}


modes find_mode(const TString& str) {
    if (str == "Spectroscopy") return modes::Spectroscopy;
    if (str == "Timing_CStart")  return modes::Timing;
    if (str == "Timing_CStop")  return modes::Timing;    
    if (str == "Spect_Timing")  return modes::Spect_Timing;
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

void reset_stored_vars(stored_vars &v, modes &mode){
    switch (mode) {
        case modes::Spectroscopy:
            reset<int16_t>(v.data_type);
            reset<int32_t>(v.LG);
            reset<int32_t>(v.HG);
            break;

        case modes::Timing:
            reset<int16_t>(v.data_type_timing);
            reset<float>(v.ToT_timing);
            reset<float>(v.ToA_timing);
            break;

        case modes::Spect_Timing:
            reset<int16_t>(v.data_type);
            reset<int32_t>(v.LG);
            reset<int32_t>(v.HG);
            reset<float>(v.ToT);
            reset<float>(v.ToA);
            break;

        case modes::Counting:
            reset<int64_t>(v.counts);
            break;

    }
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
    t->Branch("TStamp_us",&v.TStamp, "TStamp_us/D");
    t->Branch("Num_Hits",&v.hits, "Num_Hits/I");

    switch (mode) {
        case modes::Spectroscopy:
            t->Branch("Trg_Id",&v.Trg_Id, "Trg_Id/l");
            t->Branch("ch_mask", &v.ch_mask, "ch_mask/l");
            t->Branch("data_type", *v.data_type,Form("data_type[%i][64]/S",NBOARDS));
            t->Branch("PHA_LG",*v.LG,Form("PHA_LG[%i][64]/I",NBOARDS));
            t->Branch("PHA_HG",*v.HG,Form("PHA_HG[%i][64]/I",NBOARDS));

            break;


        case modes::Timing:
            t->Branch("ToA", **v.ToA_timing, Form("ToA[%i][64][%i]/F",NBOARDS,MAXHITS));
            t->Branch("ToT", **v.ToT_timing, Form("ToT[%i][64][%i]/F",NBOARDS,MAXHITS));
            t->Branch("data_type", **v.data_type_timing,Form("data_type[%i][64][%i]/S",NBOARDS,MAXHITS));

            break;            
        

        case modes::Spect_Timing:
            t->Branch("Trg_Id",&v.Trg_Id, "Trg_Id/l");
            t->Branch("ch_mask", &v.ch_mask, "ch_mask/l");
            t->Branch("data_type", *v.data_type,Form("data_type[%i][64]/S",NBOARDS));
            t->Branch("PHA_LG",*v.LG,Form("PHA_LG[%i][64]/I",NBOARDS));
            t->Branch("PHA_HG",*v.HG,Form("PHA_HG[%i][64]/I",NBOARDS));
            t->Branch("ToA",*v.ToA, Form("ToA[%i][64]/F",NBOARDS));
            t->Branch("ToT",*v.ToT, Form("ToT[%i][64]/F",NBOARDS));

            break;


        case modes::Counting:
            t->Branch("Trg_Id",&v.Trg_Id, "Trg_Id/l");
            t->Branch("ch_mask", &v.ch_mask, "ch_mask/l");
            t->Branch("counts",*v.counts, Form("counts[%i][64]/L",NBOARDS));

            break;

    }
    return t;
}
