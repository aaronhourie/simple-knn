#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <math.h>

#include "dataset.cpp"
#include "nearest.cpp"

int k = 3;
int response_index = 0;
bool verbose = false;
std::string train_filename;
std::string test_filename;

void parse_args(int argc, char *argv[]);
void build_dataset_from_file(std::string filename, Dataset *dataset);
bool validate_datasets(Dataset *train_dataset, Dataset *test_dataset);
float predict(Dataset *train_dataset, Dataset *test_dataset);
float euc_distance(std::vector<float> origin, 
        std::vector<float> point, int ignore_index);


int main(int argc, char *argv[]) {
    
    parse_args(argc, argv);

    Dataset train_dataset;
    Dataset test_dataset;

    build_dataset_from_file(train_filename, &train_dataset);
    build_dataset_from_file(test_filename, &test_dataset);

    validate_datasets(&train_dataset, &test_dataset);
    float prediction = predict(&train_dataset, &test_dataset);

    std::cout << prediction << std::endl;
    return 0;
}




/**
 * Parses the arguments from the command line
 * and assigns values to the variables needed for analysis.
 */
void parse_args(int argc, char *argv[]) {
    // Loop through all options
    // Skip the program name.
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            // Pull out the - in a dash option.
            switch(argv[i][1]) {
                // Get the name of the file to read training data from.
                case 't' :
                    i++;
                    train_filename = argv[i];
                    break;
                // Get the name of the file to read test data from.
                case 's' :
                    i++;
                    test_filename = argv[i];
                    break;
                // Get the value for k.
                case 'k' :
                    i++;
                    k = atoi(argv[i]);
                    break;
                // Get the index of the response variable
                case 'r' :
                    i++;
                    response_index = atoi(argv[i]);
                    break;
                case 'v' :
                    i++;
                    verbose = true;
                    break;
                // Unkown argument. Display error.
                default : 
                    i++;
                    std::cerr << "Error: Unknown argument: ";
                    std::cerr << argv[i];
                    std::cerr << std::endl;
                    break;
            }
        }
        // Malformed arguments, display error
        else {
            std::cerr << "Error: Malformed arguments: '";
            std::cerr << argv[i];
            std::cerr << "' arguments must have the format:  -x option";
            std::cerr << std::endl;
            break;
        }
    }
}

void build_dataset_from_file(std::string filename, Dataset *dataset) {

    std::ifstream input_file;
    std::string line;
    input_file.open(filename);

    // Read each line of the file.
    while(std::getline(input_file, line)) {
        // Read each line as a stream.
        std::istringstream string_stream(line);
        float cell;
        // Going to store it in a row.
        std::vector<float> row;
        // Iterate over all entries in the row.
        while (string_stream >> cell) {
            // Add the cell to the row
            row.push_back(cell);
        }
        // Save this row to the dataset
        dataset->add_row(row);
        
    }
    // Close the stream.
    input_file.close();
}

bool validate_datasets(Dataset *train_dataset, Dataset *test_dataset) {
    return train_dataset->num_cols() == test_dataset->num_cols();
}

float predict(Dataset *train_dataset, Dataset *test_dataset) {

    Nearest nearest(k);
    // For every test data, we want to calculate the 
    // euclidian distance to all points in the train data.
    for (int i = 0; i < test_dataset->num_rows(); i++) {
        std::vector<float> test_norm  = test_dataset->get_norm_values(i);
        // Loop over all rows in the train dataset (calculate distance)
        for (int j = 0; j < train_dataset->num_rows(); j++) {
            std::vector<float> train_norm = train_dataset->get_norm_values(j);
            float distance = euc_distance(test_norm, train_norm, response_index);
            nearest.add_neighbour(j, distance);
        }
    }
    std::vector<int> neighbours = nearest.get_neighbours();
    float sum = 0;
    for (int i = 0; i < neighbours.size(); i++) {
        std::vector<float> row = train_dataset->get_row(neighbours.at(i));
        sum += row.at(response_index);
    }
    return sum / neighbours.size();
}

float euc_distance(std::vector<float> origin, 
        std::vector<float> point, int ignore_index) {

    float sum = 0;

    for (int col = 0; col < origin.size(); col++) {
        // Ignore the response variable 
        if (col != ignore_index) {
            // Calculate the difference.
            float diff = origin.at(col) - point.at(col);
            sum += pow(diff, 2);
        }
    }

    return sqrt(sum);
}
