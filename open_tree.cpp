#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TH2D.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"

using namespace std;

void using_tr(){
    int N_boards=1;
    
    string filepath="test_files/PHA/Run10_list_O.root";
    
    TFile * file = new TFile(filepath.c_str(), "read");
    if (!file || file->IsZombie()) {
        cerr << "Unable to open ROOT file." << endl;
        return;    
        }
    
    TTreeReader r = TTreeReader("datas", file);
    TTreeReaderArray<Int_t> LG(r, "PHA_LG.LG");
    TTreeReaderArray<Int_t> HG(r, "PHA_HG.HG");

    TH1I * h_lg = new TH1I("h_lg","Low gain; Value [a.u]; Counts",4096,-5,5002);
    while (r.Next()){
        for (int b=0; b<N_boards; b++){
            for (int j=0; j<64; j++){
                Int_t val = LG[b*64 +j];
                h_lg->Fill(val);
            }
        }
    }
    TCanvas * c_1 = new TCanvas();
    h_lg->Draw();
}

int open_tree(){
    int N_boards=1;
    
    string filepath;
    cin >> filepath;
    
    TFile * file = new TFile(filepath.c_str(), "read");
    if (!file || file->IsZombie()) {
        cerr << "Unable to open ROOT file." << endl;
        return 0;    
        }
    
    TTreeReader r = TTreeReader("datas", file);
    TTreeReaderArray<Int_t> LG(r, "PHA_LG");
    TTreeReaderArray<Int_t> HG(r, "PHA_HG");

    TH1F * h_lg = new TH1F("h_lg","Low gain; Value [a.u]; Counts",4096,-5,5002);
    TH1F * h_hg = new TH1F("h_hg","High gain; Value [a.u]; Counts",4096,-5,5002);

    TH2D * h_2 = new TH2D("hg_vs_c","hg_vs_c",4096,0,2602956.56,4096,0,4096);

    while (r.Next()){
        for (int b=0; b<N_boards; b++){
            for (int j=0; j<64; j++){
                Int_t val = LG[b*64 +j];
                h_lg->Fill(val);
            }
        }
    }
    
    TCanvas * c_1 = new TCanvas();
    h_lg->Draw();
    TCanvas * c_2 = new TCanvas();
    h_hg->Draw();
    TCanvas * c_3 = new TCanvas();
    h_2->Draw();

    // check information tree
    TTree * tree_info = (TTree *)file->Get("info");

    int board_mod;
    int e_Nbins;
    int run;
    UInt_t time_epoch;
    // TString * file_format, *janus_rel, *acq_mode, *time_UTC;

    TString * file_format = new TString(); 
    TString * janus_rel = new TString(); 
    TString * acq_mode = new TString(); 
    TString * time_UTC = new TString(); 
    TString * ch_mask = new TString(); 

    tree_info->SetBranchAddress("board_mod", &board_mod);
    tree_info->SetBranchAddress("file_format", &file_format);
    tree_info->SetBranchAddress("janus_rel", &janus_rel);
    tree_info->SetBranchAddress("acq_mode", &acq_mode);
    tree_info->SetBranchAddress("e_Nbins", &e_Nbins);
    tree_info->SetBranchAddress("run", &run);
    tree_info->SetBranchAddress("time_epoch", &time_epoch);
    tree_info->SetBranchAddress("time_UTC", &time_UTC);
    tree_info->SetBranchAddress("ch_mask", &ch_mask);

    tree_info->GetEntry(0);
    cout << "board model: "<< board_mod << endl;
    cout << "file_format: "<< *file_format << endl;
    cout << "janus_rel: "<< *janus_rel << endl;
    cout << "acq_mode: "<< *acq_mode << endl;
    cout << "e_Nbins: "<< e_Nbins << endl;
    cout << "run: "<< run << endl;
    cout << "time_epoch: "<< time_epoch << endl;
    cout << "time_UTC: "<< *time_UTC << endl;
    cout << "ch_mask: "<< *ch_mask << endl;


    return 1;
}