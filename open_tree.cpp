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

using namespace std;

int open_tree(){
    int N_boards=1;
    
    TFile * file = new TFile("test_files/PHA/Run14_list.root", "read");
    if (!file || file->IsZombie()) {
        cerr << "Unable to open ROOT file." << endl;
        return 0;    
        }
    
    TTree * tree = (TTree *)file->Get("datas");
    int entries = tree->GetEntries();
    tree->Print();

    int LG[1][64];
    int HG[1][64];
    double TStamp;
    tree->SetBranchAddress("PHA_LG", &LG);
    tree->SetBranchAddress("PHA_HG", &HG);
    tree->SetBranchAddress("TStamp",&TStamp);

    TH1F * h_lg = new TH1F("h_lg","Low gain; Value [a.u]; Counts",4096,-5,5002);
    TH1F * h_hg = new TH1F("h_hg","High gain; Value [a.u]; Counts",4096,-5,5002);

    TH2D * h_2 = new TH2D("hg_vs_c","hg_vs_c",4096,0,2602956.56,4096,0,4096);

    for (int i=0; i<entries; i++){
        tree->GetEntry(i);
        for (int b=0; b<N_boards; b++){
            for (int j=0; j<64; j++){
                h_lg->Fill(LG[b][j]);
                h_hg->Fill(HG[b][j]);
                h_2->Fill(TStamp,HG[0][j]);
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

    tree_info->SetBranchAddress("board_mod", &board_mod);
    tree_info->SetBranchAddress("file_format", &file_format);
    tree_info->SetBranchAddress("janus_rel", &janus_rel);
    tree_info->SetBranchAddress("acq_mode", &acq_mode);
    tree_info->SetBranchAddress("e_Nbins", &e_Nbins);
    tree_info->SetBranchAddress("run", &run);
    tree_info->SetBranchAddress("time_epoch", &time_epoch);
    tree_info->SetBranchAddress("time_UTC", &time_UTC);

    tree_info->GetEntry(0);
    cout << "board model: "<< board_mod << endl;
    cout << "file_format: "<< *file_format << endl;
    cout << "janus_rel: "<< *janus_rel << endl;
    cout << "acq_mode: "<< *acq_mode << endl;
    cout << "e_Nbins: "<< e_Nbins << endl;
    cout << "run: "<< run << endl;
    cout << "time_epoch: "<< time_epoch << endl;
    cout << "time_UTC: "<< *time_UTC << endl;

    return 1;
}