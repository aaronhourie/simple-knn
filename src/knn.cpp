#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <math.h>

#include "dataset.cpp"
#include "nearest.cpp"

int default_k = 3;
int response_index = 0;
int verbosity = 1;
bool find_k = false;
int max_k = 0;
std::string train_filename;
std::string test_filename;

void parse_args(int argc, char *argv[]);
void build_dataset_from_file(std::string filename, Dataset *dataset);
bool validate_datasets(Dataset *train_dataset, Dataset *test_dataset);
float get_mse(int k, Dataset *train_dataset, Dataset* test_dataset); 
float predict(int k, Dataset *train_dataset, Dataset *test_dataset, int test_index);
float euc_distance(std::vector<float> origin, 
        std::vector<float> point, int ignore_index);


int main(int argc, char *argv[]) {
    
    parse_args(argc, argv);

    Dataset train_dataset;
    Dataset test_dataset;

    build_dataset_from_file(train_filename, &train_dataset);
    build_dataset_from_file(test_filename, &test_dataset);

    // Use the user defined k-value
    if (!find_k) { 
        // Calculate MSE
        float mse = get_mse(default_k, &train_dataset, &test_dataset);
        // Report.
        if (verbosity >= 1) {
            std::cout << "MSE: " << mse << 
                " (k=" << default_k << ")" << std::endl;
        }
        else {
            std::cout << mse << std::endl;
        }
    }
    // Search up to a user defined value for the best k-value
    else {
        // Best MSE is the lowest value.
        bool first = true;
        float best_mse = 0;
        int best_k = 2;
        // Loop through all values, starting with 2 (knn doesn't work right with fewer)
        for (int k = 2; k < max_k; k++) {
            // Use the first MSE value as the current "lowest"
            if (first) {
                best_mse = get_mse(k, &train_dataset, &test_dataset);
                best_k = k;
            }
            // Check if the value is better than our current best.
            else {
                float mse = get_mse(k, &train_dataset, &test_dataset);
                if (mse < best_mse) {
                    // If so, update it.
                    best_mse = mse; 
                    best_k = k;
                }
            }
        }

        // Report.
        if (verbosity >= 1) {
            std::cout << "Best MSE: " << best_mse << "(k=" << best_k << ")" << std::endl;
        }
        else {
            std::cout << best_mse << std::endl;
        }
    }

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
                case 'r' :
                    i++;
                    train_filename = argv[i];
                    break;
                // Get the name of the file to read test data from.
                case 's' :
                    i++;
                    test_filename = argv[i];
                    break;
                // Get the value for defined value of k.
                case 'k' :
                    i++;
                    default_k = atoi(argv[i]);
                    break;
                // Get the index of the response variable
                case 'i' :
                    i++;
                    response_index = atoi(argv[i]);
                    break;
                // Get the level of verbosity.
                case 'v' :
                    i++;
                    verbosity = atoi(argv[i]);
                    break;
                case 'x' :
                    i++;
                    find_k = true;
                    max_k = atoi(argv[i]);
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

// Builds a dataset object using a tab or space seperated file.
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

// Calculates the Mean Squared Error for a test and training dataset
// for a particular value of k
float get_mse(int k, Dataset *train_dataset, Dataset* test_dataset) {

    float squared_error_sum = 0;

    for (int i = 0; i < test_dataset->num_rows(); i++) {
        // Check to make sure datasets have the same column length.
        validate_datasets(train_dataset, test_dataset);
        // Predict a value for the response column
        float prediction = predict(k, train_dataset, test_dataset, i);
        // Get the truth value.
        float truth = test_dataset->get_row(i).at(response_index);
        // Calculate the error
        float error = prediction - truth;
        // Add the square to the sum.
        squared_error_sum += pow(error, 2);
        // Report
        if (verbosity >= 3) {
            std::cout << "Row " << i << ": ";
            std::cout << prediction; 
            std::cout << " -- error: " << error; 
            std::cout << " (Truth data: " << truth << ")"; 
            std::cout << std::endl;
        }
    }

    // Mean squared error -- the average of the squared errors.
    float mean_squared_error = (squared_error_sum / test_dataset->num_rows());

    // Report
    if (find_k && verbosity >= 2) {
        std::cout << "Mean Squared Error: " << mean_squared_error;
        std::cout << "(k=" << k << ")";
        std::cout << std::endl;
    }

    return mean_squared_error; 
}

// Predicts a value for the response column based on a training dataset.
float predict(int k, Dataset *train_dataset, Dataset *test_dataset, int test_index) {

    Nearest nearest(k);
    // For every test data, we want to calculate the 
    // euclidian distance to all points in the train data.
    std::vector<float> test_norm  = test_dataset->get_norm_values(test_index);
    // Loop over all rows in the train dataset (calculate distance)
    for (int i = 0; i < train_dataset->num_rows(); i++) {
        std::vector<float> train_norm = train_dataset->get_norm_values(test_index);
        float distance = euc_distance(test_norm, train_norm, response_index);
        nearest.add_neighbour(i, distance);
    }
    std::vector<int> neighbours = nearest.get_neighbours();
    float sum = 0;
    for (int i = 0; i < neighbours.size(); i++) {
        std::vector<float> row = train_dataset->get_row(neighbours.at(i));
        sum += row.at(response_index);
    }
    return sum / neighbours.size();
}

// Calculates the euclidian distance between two multi-dimensional points.
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
