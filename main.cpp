#include "modes_helpers.hpp"
#include "csv_parser.hpp"

using namespace std;

const char *argp_program_version = "converter 1.0";
const char *argp_program_bug_address = "<carmen.selicato@cern.ch>";
static char doc[] = "Code to convert a csv file written by the Janus software to a root file.";
static char args_doc[] = "inputFile N_boards";

static struct argp_option options[] = {
  {"output", 'o', "FILE", 0, "Optional output file name"},
  { 0 }
};

struct arguments {
  string inFile;
  int N_boards=-1;  
  string outFile;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = static_cast<struct arguments*>(state->input);
    switch (key) {
    case 'o':
        arguments->outFile = arg;
        break;
    case ARGP_KEY_ARG:
      switch (state->arg_num) {
        case 0: arguments->inFile = arg; break;
        case 1: arguments->N_boards = stoi(arg); break;
        default: return ARGP_ERR_UNKNOWN;
      }
      break;
    case ARGP_KEY_END:
        if (arguments->inFile.empty() || arguments->N_boards == -1){
            argp_failure(state, 1, 0, "Missing input file or number of boards. See --help for more information.");
            exit(ARGP_ERR_UNKNOWN);
        }    
    default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };


void is_consistent(vector<vector<string>>& data){
    string line;
    set<size_t> row_sizes = {};

    for (vector<string> row : data){
        row_sizes.insert(row.size());
    }

    if (row_sizes.size() != 1){
        throw runtime_error("The file has inconsistent columns, unable to produce root file.");
    }

}


int main(int argc, char* argv[]){

    TStopwatch timer;
    timer.Start();

    struct arguments arguments;
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    if (arguments.outFile.empty()) {
        arguments.outFile = arguments.inFile.substr(0,arguments.inFile.size()-3)+"root";
        cout << "No output file provided. Using default: " << arguments.outFile << endl;
        cout << endl;
    }

    fstream file;
    file.open(arguments.inFile, ios::in);

    if (!file.is_open()) {
        cout << "Error while opening file " << arguments.inFile << endl;
        return 1;
    }
    cout << "Opened file: " << arguments.inFile << endl;

    vector<vector<string>> metadata = get_csv_metadata(file);
    vector<vector<string>> data = get_csv_data(file);

    cout << "Done reading file." << endl;

    try {
        is_consistent(data);
        cout << "The file has consistent columns."<<endl;
    }
    catch(const exception& e){
        cerr << e.what() << '\n';
        return 1;
    }
    

    TString acq_mode = metadata[3][1];   
    TTree *tr_data, *tr_info;   
    try {
        tr_data = make_data_tree(data,acq_mode, arguments.N_boards);
        tr_info = make_info_tree(metadata,acq_mode);
    }
    catch(const exception& e){
        cerr << e.what() << '\n';
        return 1;
    }

    // write trees in the output file
    TFile fout(arguments.outFile.c_str(), "recreate");
    tr_data->Write();
    tr_info->Write();
    // close file
    fout.Close();

    timer.Stop();

    cout << "Output file has been written." << endl;
    cout << endl;
    cout << "Real time: " << timer.RealTime() << " seconds\n";
    cout << "CPU time: " << timer.CpuTime() << " seconds\n";
    cout << endl;

    return 0;
}