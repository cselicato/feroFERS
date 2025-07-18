#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <argp.h>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TH2D.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TText.h"
#include "TPavesText.h"
#include "TFrame.h"
#include "TPaveStats.h"

using namespace std;

TH1F * make_hist(const string& name, int n_bins, vector<float> vec, bool mask){
    float start = *min_element(vec.begin(), vec.end());
    float stop = *max_element(vec.begin(), vec.end());
    TH1F *h = new TH1F(name.c_str(), name.c_str(),n_bins, start-stop*0.02, stop+stop*0.02);
    for (auto x : vec){
        if (mask){
            if (x>0){h->Fill(x); } 
        }
        else {h->Fill(x); }
        }
    return h;
}

int display_data(){
    cout << "Please write filepath..." << endl;
    string inFile;
    cin >> inFile;
    
    bool masked = true;

    TFile * file = new TFile(inFile.c_str(), "read");
    if (!file || file->IsZombie()) {
        cerr << "Unable to open ROOT file." << endl;
        return 0;    
        }

    // read info tree (contains metadata of the acquisition)
    TTree * tree_info = (TTree*)file->Get("info");

    tree_info->GetEntry(0);
    cout << "METADATA:" << endl;
    TPavesText *paves = new TPavesText(0.0085,0.88,0.35,0.98,1,"NDC");
    for (auto* b : * tree_info->GetListOfBranches()){
        TLeaf * leaf = tree_info->GetLeaf(b->GetName());
        string t_name = leaf->GetTypeName();
        string b_name = b->GetName();

        if (t_name == "TString"){
            TString* str_val = (TString*)leaf->GetValuePointer();
            cout << b->GetName() <<": "<< *str_val << endl;
            if (b_name=="time_UTC"){
                paves->AddText(*str_val);   }
        }
        else {
            cout << b->GetName() <<": "<< leaf->GetValue() << endl;
            if (b_name=="run"){
                paves->AddText(Form("Run#: %i",(int)leaf->GetValue())); }
        }
    }
    cout << endl;

    TTree * tree_data = (TTree*)file->Get("datas");
    // tree_data->Print();
    int N = tree_data->GetEntriesFast();
    int n_b = tree_data->GetNbranches();
    TH1F * h[n_b];
    TCanvas * c[n_b];
    gStyle->SetOptStat("erm");

    // loop on branches
    int b_count=0;
    for (auto* b : * tree_data->GetListOfBranches()){
        TLeaf * leaf = tree_data->GetLeaf(b->GetName());
        string t_name = leaf->GetTypeName();
        
        if (t_name == "TString"){
            continue;        }
        vector<float> b_content = {};


        for (int i=0;i<N;i++){
            tree_data->GetEntry(i);
            int len = leaf->GetLenStatic();
            if (len == 1){
                auto val = leaf->GetValue();
                b_content.push_back(val);
            }
            else {
                for (int j=0;j<len;j++){
                    auto val = leaf->GetValue(j);
                    b_content.push_back(val);
                }
            }
        }

        h[b_count] = make_hist(b->GetName(), 60, b_content,masked);
        // draw the histogram only if it has entries
        if (h[b_count]->GetEntries()>0){
            c[b_count] = new TCanvas(b->GetName(),b->GetName());
            c[b_count]->SetTopMargin(0.165);
            c[b_count]->SetGrid();
            h[b_count]->Draw();
            paves->Draw();  
            c[b_count]->Update();
            TPaveStats *st = (TPaveStats*)h[b_count]->FindObject("stats");
            st->SetX1NDC(0.77);
            st->SetX2NDC(0.97);
            st->SetY1NDC(0.85);
            st->SetY2NDC(0.97);
        }
        b_count+=1;
    }
    return 0;
}