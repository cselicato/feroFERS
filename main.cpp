#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

void read_csv(const string& filepath){
    fstream file;
    file.open(filepath, ios::in);

    if (!file.is_open()) {
        cout << "Error while opening file " << filepath << endl;
        return;
    }

    int line_count = 0;
    string line;
    vector<vector<string>> data;

    while (getline(file, line)){
        if (line[0] == *"/"){
            continue;
        }
        vector<string> row;
        stringstream ss(line);
        string value;

        while (getline(ss, value, ',')) {
            row.push_back(value);
        }

        data.push_back(row);
        line_count+=1;
    }

    cout << "Numero di righe lette: " << line_count << endl;
    cout << " " << data[0][0] << endl;

}


int main(){

    read_csv("test_files/test.csv");

    return 1;
}