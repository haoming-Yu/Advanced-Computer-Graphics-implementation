#include "energy/energy.h"
const double lambda = 0.3;
const double weight_s = 0.2;
const double penalty_beta = 1;
EnergyType GetDataCost(module m,int PatternId)
{
    // double r = 
    int pattern_color = (PatternId&0x10) * 255;
    double r = (pattern_color==m.center)? GetReliability(PatternId):0;
    double penalty_cost = int(pattern_color!=m.center) * penalty_beta;  
    double r_cost = exp(-m.importance) * (1-r);
    double simlirity_cost =  GetSimilirary(m.data,PatternId);
    return cv::saturate_cast<EnergyType>(simlirity_cost + r_cost + penalty_cost);
}
EnergyType GetSmoothCost(int PatternId1,int PatternId2)
{
    return cv::saturate_cast<EnergyType>(GetSimilirary(PatternId1,PatternId2));
}
EnergyType GetEdgeWeight(int data1,int data2)
{
	return cv::saturate_cast<EnergyType>(exp(-GetSimilirary(data1,data2)));

}