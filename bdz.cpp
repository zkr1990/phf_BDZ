#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;
using GraphId = int;
#define GraphVertex int
#define GraphEdge int

typedef struct Data {
        uint64_t key_;
        int g_;
}Data;
using DataVector = vector<Data>;

void reduceDegree(vector<GraphVertex> degreeToVertexes[], vector<GraphEdge> vertexesToEdges[], int node_degree[], const int& u, const int& e) {
    int tmpDegree = node_degree[u];
    // delete the vertex
    vector<GraphVertex>::iterator u_V = find(degreeToVertexes[tmpDegree].begin(), degreeToVertexes[tmpDegree].end(), u);
    degreeToVertexes[tmpDegree].erase(u_V);
    node_degree[u]--;
    tmpDegree--;
    degreeToVertexes[tmpDegree].push_back(u);
    // delete the edge
    vector<GraphEdge>::iterator e_E = find(vertexesToEdges[u].begin(), vertexesToEdges[u].end(), e);
    vertexesToEdges[u].erase(e_E);
};

typedef struct TripleHash {
    int hash0_;
    int hash1_;
    int hash2_;
    int operator[](int index) const {
        if (0 == index) {
            return hash0_;
        }
        if (1 == index) {
            return hash1_;
        }
        if (2 == index) {
            return hash2_;
        }
        throw runtime_error("unexpected index");
    }

    bool operator<(const TripleHash& rhs) const {
        if (hash0_ != rhs.hash0_) {
            return hash0_ < rhs.hash0_;
        }
        if (hash1_ != rhs.hash1_) {
            return hash1_ < rhs.hash1_;
        }
        if (hash2_ != rhs.hash2_) {
            return hash2_ < rhs.hash2_;
        }
        return false;
    }

    bool operator==(const TripleHash& rhs) const {
        return (hash0_ == rhs.hash0_) && (hash1_ == rhs.hash1_) && (hash2_ == rhs.hash2_);
    }
}TripleHash;

// uint64_t h0(uint64_t value)
// {
//     return ((value << 7) + 11) ^ (value >> 57) ^ value;
// }
// uint64_t h1(uint64_t value)
// {
//     return ((value << 13) + 23) ^ (value >> 51) ^ value;
// }
// uint64_t h2(uint64_t value)
// {
//     return ((value << 17) + 113) ^ (value >> 47) ^ value;
// }
uint64_t h0(uint64_t value)
{
    return ((value << 8) + 11) ^ (value >> 56) ^ value;
}
uint64_t h1(uint64_t value)
{
    return ((value << 13) + 23) ^ (value >> 51) ^ value;
}
uint64_t h2(uint64_t value)
{
    return ((value << 16) + 113) ^ (value >> 47) ^ value;
}

using TripleHashes = vector<TripleHash>;

TripleHash FillHash(const uint64_t& key, int m3) {
    TripleHash result;
    result.hash0_ = h0(key) % m3;
    result.hash1_ = (h1(key) % m3) + m3;
    result.hash2_ = (h2(key) % m3) + 2*m3;
    return move(result);
}

TripleHashes FillTripleHashes(const vector<uint64_t>& keys, int m3) {
    TripleHashes hashes(keys.size());
    for (int i = 0; i < keys.size(); ++i) {
        hashes[i] = FillHash(keys[i], m3);
    }
    return move(hashes);
}

bool HasCollision(const vector<uint64_t>& keys, int m3) {
    TripleHashes hashes = FillTripleHashes(keys, m3);
    std::sort(hashes.begin(), hashes.end());
    for (int i = 1; i < hashes.size(); ++i) {
        if (hashes[i - 1] == hashes[i]) {
                return true;
        }
    }
    return false;
}

int MinimalPerfectHash(const vector<uint64_t>& keys, DataVector& data_) {
    int m = ((keys.size() * 4) / 9 + 1) * 3;
    while (HasCollision(keys, m / 3)) {
        m += 3;
    }
    int m3 = m / 3;
    const TripleHashes hashes = FillTripleHashes(keys, m3);

    int node_degree[m] = {0};
    vector<int> delete_order(keys.size());

    vector<GraphEdge> vertexesToEdges[m]; // edge
    vector<GraphVertex> degreeToVertexes[keys.size() + 1]; // degree

    // for each edge, find three vertexes: 0,1,2
    // then for each of 0,1,2, add vector of edges and increase its degree
    for (int e = 0; e < hashes.size(); ++e) {
        vertexesToEdges[hashes[e].hash0_].push_back(e);
        vertexesToEdges[hashes[e].hash1_].push_back(e);
        vertexesToEdges[hashes[e].hash2_].push_back(e);
        node_degree[hashes[e].hash0_]++;
        node_degree[hashes[e].hash1_]++;
        node_degree[hashes[e].hash2_]++;
    }
    
    // store information of all vertexes that have the same degree
    for (int i = 0; i < m; i++) {
        degreeToVertexes[ node_degree[i] ].push_back(i);
    }

    // delete edge one by one, reduce the degree accordingly
    // each loop, start from vertex that belongs to only one edge
    for (int e = 0; e < hashes.size(); ++e) {
        if (degreeToVertexes[1].empty()) {
            // The deletion cause all remaining vertexes with degree = 2
            // The graph can't not be constructed using current h0,h1,h2 functions
            // need new h0,h1,h2 functions
            throw runtime_error("unexpected graph, please change h0,h1,h2.");
        }
        int u;

        u = *degreeToVertexes[1].begin();
        if (1 != vertexesToEdges[u].size()) {
            // The current vertex has more than one edge, error
            throw std::runtime_error("graph invariant is busted 1");
        }
        int edge_id = *vertexesToEdges[u].begin();

        for (int h=0; h<3; h++) {
            reduceDegree(degreeToVertexes, vertexesToEdges, node_degree, hashes[edge_id][h], edge_id);
        }
        delete_order[e] = edge_id;
        if (0 != vertexesToEdges[u].size()) {
            // after the deletion, there still exists edge(s), error
            throw std::runtime_error("graph invariant is busted 0");
        }
    }
    
    // build data_ 
    data_.resize(m);
    for (auto& item: data_) {
        item.g_ = 3;
    }
    // track vertexes that are visited
    vector<bool> visited(m);
    // determine the g() for each edge(key)
    for (int e = keys.size() - 1; e >= 0; e--) {
        int u = delete_order[e];
        int choice = 0;
        while ((choice < 3) && visited[hashes[u][choice]] == true) {
            choice++;
        }
        if (3 == choice) {
            throw runtime_error("all visited");
        }
        int hashSum = 0;
        for (int j = 0; j < 3; j++) {
            hashSum += data_[hashes[u][j]].g_;
        }
        data_[hashes[u][choice]].g_ = (9 + choice - hashSum) % 3;
        // for each edge, we find its choice vertex
        // no need to visit the other two vertexes, their g() remain unchanged
        for (size_t j = 0; j < 3; ++j) {
            visited[hashes[u][j]] = true;
        }
        // visited[hashes[u][choice]] = 1;
    }
    
    /*update key_ attribute for data_, optional
    for (int i = 0; i < keys.size(); ++i) {
        uint64_t item = keys[i];
        int index = GetIndex(item, data_);
        data_[index].key_ = item;
    }*/
    return m;
}


int GetIndex(const uint64_t& key, DataVector& data, const int m3_) {
    TripleHash hash = FillHash(key, m3_);
    int gIndex = data[hash.hash0_].g_ + data[hash.hash1_].g_ + data[hash.hash2_].g_;
    int index = hash[gIndex % 3];
    return index;
}

int main()
{
    uint64_t input_k[4000];
    vector<uint64_t> input_keys;
    DataVector dataV;
    // import from input file
	int datalen = 0;
	ifstream input("./input_file/input.txt", ios::in);
	string line;
	while (getline(input, line)) {
		stringstream ss;
		ss << line;
		if (!ss.eof()) {
			ss >> input_k[datalen];
            input_keys.push_back(input_k[datalen]);
            datalen++;
		}
	}
    
    // int input_keys[] = {6,9,2,3,4,5};
	// uint64_t input_keys[] = {24,12,2,3,4,5,6,7,8,9,13,14,15,16,19};
	// uint64_t input_keys[] = {24,12,2,3,4,5,6,7,8,9,13,14,15,16,19,375,321,532,842,25,68,71,29};
	int n = input_keys.size();

    int m = MinimalPerfectHash(input_keys, dataV);
    cout << "set function complete----------------------" << endl << endl;
	cout << "n = " << n << endl;
	cout << "m = " << m << endl;
    for(int e = 0; e < n; e++)
    {
        cout << input_keys[e] << " => " << GetIndex(input_keys[e], dataV, m/3) << endl;
    }
    return 0;
}
