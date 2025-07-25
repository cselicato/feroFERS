#include "csv_parser.hpp"

vector<string> split_line(string line, char delimiter){
    vector<string> line_content={};
    stringstream ss(line);
    string value;
    while (getline(ss, value, delimiter)) {
        if (!value.empty()){
        line_content.push_back(value);}    }

    return line_content;
}



vector<vector<string>> get_csv_metadata(fstream &file){
    vector<vector<string>> metadata;
    string line;
    file.clear();
    file.seekg(0);
    while (getline(file, line)){
        if (line.empty()==false){
            // if present, remove CR
            if (line.back() == '\r') {
                line.pop_back();    }

            if (line[2] == '*'){
                continue;        }

            else if (line[0] == '/'){
                vector<string> row = split_line(line, ':');
                metadata.push_back(row);
            }
            else {       
                break;
            }}
    } 

    return metadata;
}

vector<vector<string>> get_csv_data(fstream &file){
    file.clear();
    file.seekg(0);
    vector<vector<string>> data;
    string line;
    set<size_t> row_sizes = {};

    // read file
    while (getline(file, line)){
        if (line.empty()==false){
            // if present, remove CR
            if (line.back() == '\r') {
                line.pop_back();    }

            if (line[2] == '*' || line[0] == '/'){
                continue;        }

            else {       
                vector<string> row = split_line(line, ',');
                data.push_back(row);
                row_sizes.insert(row.size());
            }}
    } 
    
    data.erase(data.begin());   // remove column names
    
    return data;
}
