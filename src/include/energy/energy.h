#include "pre_process/pre_process.h"
#include "GCoptimization.h"
#include "LinkedBlockList.h"
#include "graph.h"

#include<math.h>
typedef GCoptimizationGridGraph::EnergyTermType EnergyType;
EnergyType GetDataCost(module m,int PatternId);
EnergyType GetSmoothCost(int PatternId1,int PatternId2);
EnergyType GetEdgeWeight(int data1,int data2);