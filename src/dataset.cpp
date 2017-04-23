#include<iostream>
#include<vector>

class Dataset {

    std::vector<std::vector<float>> data_table;
    std::vector<float> min;
    std::vector<float> max;
    int cols = 0;

    void init_min_max(std::vector<float> row);
    void check_min_max(std::vector<float> row);

    public:
        void add_row(std::vector<float> row);
        std::vector<float> get_row(int index);
        float get_min(int index);
        float get_max(int index);
        int num_rows();
        int num_cols();
        std::vector<float> get_norm_values(int row);
};

void Dataset::add_row(std::vector<float> row) {
    
    // When we add our first row from the dataset.
    if (cols == 0) {
        init_min_max(row);
    }
    else {
        check_min_max(row);
    }

    data_table.push_back(row);
}

void Dataset::init_min_max(std::vector<float> row) {

    cols = row.size();


    for (int i = 0; i < row.size(); i++) {
        min.push_back(row.at(i));
        max.push_back(row.at(i));
    }
}

void Dataset::check_min_max(std::vector<float> row) {
    for (int i = 0; i < row.size(); i++) {
        if (row.at(i) < min.at(i)) {
            min[i] = row.at(i);
        }
        else if (row.at(i) >  max.at(i)) {
            max[i] = row.at(i);
        }
    }
}

std::vector<float> Dataset::get_row(int index) {
    return data_table.at(index);
}

float Dataset::get_min(int index) {
    return min.at(index);
}

float Dataset::get_max(int index) {
    return max.at(index);
}

int Dataset::num_rows() {
    return data_table.size();
}

int Dataset::num_cols() {
    return cols;
}

std::vector<float> Dataset::get_norm_values(int row) {

    std::vector<float> values;
    
    for (int col = 0; col < data_table.at(row).size(); col++){

        float value = data_table.at(row).at(col);
        float col_min = min.at(col);
        float col_max = max.at(col);

        values.push_back((value - col_min) / (col_max - col_min));
    }

    return values;
}

