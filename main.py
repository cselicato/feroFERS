# Main goal of this code: 
# make a ROOT TTree from a csv file
#
#
# This version works only for files saved in the spectroscopy mode,
# that have the following column names:
# "TStamp_us","Trg_Id","Board_Id","Num_Hits","ChannelMask","CH_Id","DataType","PHA_LG","PHA_HG"
# It's also assumed that there is only one FERS board 

import argparse
import numpy as np
import pandas as pd
import ROOT




def decode(file_path):

    colnames = ["TStamp_us","Trg_Id","Board_Id","Num_Hits","ChannelMask","CH_Id","DataType","PHA_LG","PHA_HG"]

    # access data in the csv file
    df = pd.read_csv(file_path, sep=",",comment="/", #nrows=15,
                       dtype={"TStamp_us":float,"Trg_Id":int,"Board_Id":int,
                              "Num_Hits":int,"ChannelMask":str,"CH_Id":int,
                              "DataType":str,"PHA_LG":int,"PHA_HG":int})
    print(df)
    data = {key: df[key].values for key in colnames}

    #create ROOT file and TTree to store the variables
    out_file = ROOT.TFile(file_path.partition(".")[0]+".root", "recreate")
    tree = ROOT.TTree("data","data")
    # create branches
    TStamp_us = np.array([0], dtype=int)
    Trg_Id = np.array([0], dtype=int)
    Board_Id = np.array([0], dtype=int)
    Num_Hits = np.array([0], dtype=int)
    ChannelMask = np.array([0], dtype=int)
    CH_Id = np.array([0], dtype=int)
    DataType = np.array([0], dtype=int)
    PHA_LG = np.array([0], dtype=int)
    PHA_HG = np.array([0], dtype=int)

    tree.Branch("TStamp_us", TStamp_us,  'TStamp_us/D')
    tree.Branch("Trg_Id", Trg_Id,  'Trg_Id/I')
    tree.Branch("Board_Id", Board_Id,  'Board_Id/I')
    tree.Branch("Num_Hits", Num_Hits,  'PHA_HG/I')
    # tree.Branch("ChannelMask", ChannelMask,  'ChannelMask/C')
    tree.Branch("CH_Id", CH_Id,  'CH_Id/I')
    # tree.Branch("DataType", DataType,  'DataType/C')
    tree.Branch("PHA_LG", PHA_LG,  'PHA_LG/I')
    tree.Branch("PHA_HG", PHA_HG,  'PHA_HG/I')


    for i in range(df["PHA_HG"].size):
        TStamp_us[0] = df["TStamp_us"][i]
        Trg_Id[0] = df["Trg_Id"][i]
        Board_Id[0] = df["Board_Id"][i]
        Num_Hits[0] = df["Num_Hits"][i]
        # ChannelMask[0] = df["ChannelMask"][i]
        CH_Id[0] = df["CH_Id"][i]
        # DataType[0] = df["DataType"][i]
        PHA_LG[0] = df["PHA_LG"][i]
        PHA_HG[0] = df["PHA_HG"][i]
      
        tree.Fill()

    # x, y = np.array([1, 2, 3]), np.array([4, 5, 6])
    # rdf = ROOT.RDF.MakeNumpyDataFrame({"x": x, "y": y})


    out_file.Write()
    out_file.Close()

    return




if __name__ == "__main__":

    parser = argparse.ArgumentParser(description=
    "Convert a csv file in a root file. Insert the desired file path.")
    parser.add_argument("file",help = "Insert the path of the csv file.")
    # parser.add_argument("outname",help = "Output file.")

    args = parser.parse_args()

    decode(args.file)