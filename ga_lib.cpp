#include<algorithm>
#include<fstream>
#include<iostream>
#include<limits>
#include<sstream>
#include<stdlib.h>
#include<string>
#include<utility>
#include<random>
#include <ctime>

#include"ga_lib.h"
#include"helper_lib.h"

void calculateFitness(Individu *individu){
  double totalDist = 0;
  int routeCount=individu->routeSet->routes.size();
  for (int r=0;r<routeCount;r++){
    totalDist += individu->routeSet->distances[r];
  }
  double fitnessValue = 1.0/(600.0*totalDist + 10000.0*(double)routeCount);
  individu->totalDist = totalDist;
  individu->routeCount = routeCount;
  individu->fitnessValue = fitnessValue;
}

bool cmpIndividuFitness(Individu* a, Individu* b){
  return a->fitnessValue < b->fitnessValue;
}

bool cmpIndividuCrowdingDistance(Individu* a, Individu* b){
  return a->crowdingDistance > b->crowdingDistance;
}

bool cmpIndividuTotalDist(Individu* a, Individu* b){
  return a->totalDist < b->totalDist;
}

bool cmpIndividuRouteCount(Individu* a, Individu* b){
  return a->routeCount < b->routeCount;
}

Individu* create1DArrayIndividu(int size){
  Individu *array = new Individu[size];
  return array;
}

Customer* create1DArrayCustomer(int size){
  Customer *array = new Customer[size];
  return array;
}

RouteSet *decodeKromosom(Config *config, int *kromosom, OrderData *orderData){
  RouteSet* routeSet = new RouteSet;
  std::vector<int> route;
  double totalDist=0;
  int totalOrder=0;
  Coordinate lastCoord=orderData->depot;
  for (int k=0;k<config->nCust;k++){
    int custID = kromosom[k];
    double dist = euclideanDistance(lastCoord, orderData->customerData[custID].coordinate);
    double distToDepot = euclideanDistance(orderData->customerData[custID].coordinate, orderData->depot);
    int odSize = orderData->customerData[custID].orderSize;
    if ((totalOrder+odSize>config->maxCap) || (totalDist+dist+distToDepot>config->maxDist)){
      routeSet->routes.push_back(route);
      totalDist += euclideanDistance(lastCoord, orderData->depot);
      routeSet->distances.push_back(totalDist);
      route.clear();
      totalDist=0;
      totalOrder=0;
      lastCoord=orderData->depot;
      dist = euclideanDistance(lastCoord, orderData->customerData[custID].coordinate);
    }
    route.push_back(custID);
    totalDist += dist;
    totalOrder += orderData->customerData[custID].orderSize;
    lastCoord = orderData->customerData[custID].coordinate;
  }
  routeSet->routes.push_back(route);
  totalDist += euclideanDistance(lastCoord, orderData->depot);
  routeSet->distances.push_back(totalDist);
  return routeSet;
}

Individu* initIndividuRandom(int nCust){
  Individu* individu = new Individu;
  individu->kromosom = create1DArrayInt(nCust);
  for(int i=0;i<nCust;i++){
    individu->kromosom[i]=i;
  }
  random_shuffle(individu->kromosom, individu->kromosom+nCust);
  return individu;
}

bool isDominate(Individu* idvA, Individu* idvB){
  return (idvA->totalDist <= idvB->totalDist) &&
  (idvA->routeCount<=idvB->routeCount) &&
  ((idvA->totalDist<idvB->totalDist) || (idvA->routeCount<idvB->routeCount));
}

Individu* orderCrossover_(Config *config, Individu *parentA, Individu *parentB){
  /*
    First randomize segment points a and b
  */
  int a=rand()%config->nCust;
  int b=rand()%config->nCust;
  if (a>b){
    int c=a;
    a=b;
    b=c;
  }

  /*
    copy parentA segment (a,b) to offspring segment(a,b)
  */
  bool *genExistFlag = create1DArrayBool(config->nCust);
  Individu* offspring = new Individu;
  offspring->kromosom = create1DArrayInt(config->nCust);
  for (int c=a;c<=b;c++){
    int custID = parentA->kromosom[c];
    offspring->kromosom[c] = custID;
    genExistFlag[custID] = true;
  }

  /*
    and then add parentB's gens
    not yet contained by the offspring
  */
  int ofIdx=b+1;
  for (int genBIdx=b+1;ofIdx<a || ofIdx>b;genBIdx = (genBIdx+1)%config->nCust){
    int gen = parentB->kromosom[genBIdx];
    if (genExistFlag[gen]){
      continue;
    }
    offspring->kromosom[ofIdx]=gen;

    genExistFlag[gen]=true;
    ofIdx = (ofIdx+1)%config->nCust;
  }
  return offspring;
}

pair<Individu*,Individu*> orderCrossover(Config *config, pair<Individu*,Individu*> parents){
  pair<Individu*,Individu*> offs;
  offs.first = orderCrossover_(config, parents.first, parents.second);
  offs.second = orderCrossover_(config, parents.second, parents.first);
  return offs;
}

void rsMutation(Config *config, Individu *individu){
  /*
    First randomize Mutation-segment points a and b
  */
  int a=rand()%config->nCust;
  int b=rand()%config->nCust;
  //Switch Mutation-segment points if a is higher than b
  if (a>b){
    int c=a;
    a=b;
    b=c;
  }
  int indxMutA = a;
  int indxMutB = b;

  //Swapping Algorithm
  while(indxMutA<indxMutB){
    int custID = individu->kromosom[indxMutA];
    individu->kromosom[indxMutA] = individu->kromosom[indxMutB];
    individu->kromosom[indxMutB] = custID;
    indxMutA++;indxMutB--;
  }
}

OrderData* readOrderData(Config *config){
  ifstream dataFile(config->fileName);
  OrderData* odData = new OrderData;
  odData->customerData = create1DArrayCustomer(config->nCust);
  dataFile >> odData->depot.x >> odData->depot.y;
  for (int c=0;c<config->nCust;c++){
    dataFile >> odData->customerData[c].coordinate.x;
    dataFile >> odData->customerData[c].coordinate.y;
    dataFile >> odData->customerData[c].orderSize;
  }
  dataFile.close();
  return odData;
}

// Roulette Wheel
int spinRouletteWheel(double *probs, int probSize){
	float 	select; 	// Variable Random Number
	int 	result ; 	// Variable Result
	int c, r, z;	 	//variable batas
	srand(time(0));
	//Random Number
	select = (double) rand()/RAND_MAX;

			// NORMALISASI NILAI FITNESS
// diketahui fitness 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0
//	for (c = 0; c< Probsize; c++){
//		probs [c] = (double) (c+1)/10;
//	}
	r = 0;
	c = 0;
	z = 0;
	//code selected colom
	while(c<10){
		if (select - probs[c] < 0){
			result = c;
			z++;
		}
		c++;
	}
	return result;

}
