#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"

using namespace std;

int open_tree(){
    int N_boards=1;
    
    TFile * file = new TFile("test_files/test.root", "read");
    if (!file || file->IsZombie()) {
        cerr << "Unable to open ROOT file." << endl;
        return 0;    
        }
    
    TTree * tree = (TTree *)file->Get("data");
    int entries = tree->GetEntries();

    int LG[1][64];
    int HG[1][64];
    tree->SetBranchAddress("PHA_LG", &LG);
    tree->SetBranchAddress("PHA_HG", &HG);

    TH1F * h_lg = new TH1F("h_lg","h_lg",4096,0,4096);
    TH1F * h_hg = new TH1F("h_hg","h_hg",4096,0,4096);

    for (int i=0; i<entries; i++){
        tree->GetEntry(i);
        for (int j=0; j<64; j++){
            h_lg->Fill(LG[0][j]);
            h_hg->Fill(HG[0][j]);
        }
    }
    TCanvas * c_1 = new TCanvas();
    h_lg->Draw();
    TCanvas * c_2 = new TCanvas();
    h_hg->Draw();

    return 1;
}