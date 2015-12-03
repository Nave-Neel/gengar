#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>
#include <vector>

#include "Node.h"

std::normal_distribution<double>* network_bandwidth;
std::normal_distribution<double>* disk_bandwidth;
std::vector<Node>* cluster;
