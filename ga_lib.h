#ifndef GA_LIB_H
#define GA_LIB_H
#include<string>
#include<utility>
#include<vector>

#include"helper_lib.h"
using namespace std;

struct RouteSet{
  vector<vector<int> > routes;
  vector<double> distances;
};

struct Individu{
  int *kromosom;
  RouteSet routeSet;
  double totalDist;
  int routeCount;
  double fitnessValue;
  double crowdingDistance;
  int dominationCount;
  int dominatedCount;
};

struct Customer{
  Coordinate coordinate;
  int orderSize;
};

struct OrderData{
  Customer *customerData;
  Coordinate depot;
};

void calculateFitness(Individu *individu);
bool cmpIndividuFitness(Individu a, Individu b);
bool cmpIndividuCrowdingDistance(Individu a, Individu b);
bool cmpIndividuTotalDist(Individu a, Individu b);
bool cmpIndividuRouteCount(Individu a, Individu b);
Individu* create1DArrayIndividu(int size);
Customer* create1DArrayCustomer(int size);
RouteSet decodeKromosom(Config config, int *kromosom, OrderData orderData);
Individu initIndividuRandom(int nCust);
pair<Individu,Individu> orderCrossover(Config config, pair<Individu,Individu> parents);
void rsMutation(Config config, Individu *individu);
OrderData readOrderData(Config config);
Individu* selectionNSGA2(Config config, Individu *population);
void sortCrowdingDistance(Individu *population, int populationSize);
int spinRouletteWheel(double *probs);


#endif
