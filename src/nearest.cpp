#include <vector>

class Nearest {

    int k=0;
    float max=0;
    int max_index=0;

    std::vector<int> indices;
    std::vector<float> euc_distances;

    public:
        Nearest (int k);
        void add_neighbour (int index, float euc_distance);
        std::vector<int> get_neighbours();

};

Nearest::Nearest ( int _k ) {
    k  = _k;
}

void Nearest::add_neighbour (int index, float euc_distance) {

    // Fewer than K items, it goes in automatically.
    if (euc_distances.size() < k) {
        indices.push_back(index);
        euc_distances.push_back(euc_distance);
        // If the new one we added is larger, adjusted the tracker.
        if (euc_distance > max) {
            max = euc_distance;
            max_index = euc_distances.size() - 1;
        }
    }
    // If it is smaller than our largest value, replace the largest.
    else if (euc_distance < max) {
        euc_distances[max_index] = euc_distance; 
        indices[max_index] = index;
        max = euc_distance;
        for (int i = 0; i < euc_distances.size(); i++) {
            if (euc_distances.at(i) > max) {
                max_index = i;
                max = euc_distances.at(i);
            }
        }
    }
}

std::vector<int> Nearest::get_neighbours() {
    return indices;
}
