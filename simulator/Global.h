#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>
#include <vector>

class Client;

#include "Node.h"

std::default_random_engine generator;
std::normal_distribution<double>* network_bandwidth;
std::normal_distribution<double>* disk_bandwidth;
std::vector<Node>* cluster;
Client* client;
