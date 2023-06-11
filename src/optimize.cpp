#include "optimize/optimize.h"

typedef GCoptimizationGridGraph::EnergyTermType EnergyT;

void OptimizeBySwap(int row,int col,std::vector<module> &modules)
{
    try{
        GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(col, row, NUM_LABELS);
        
        EnergyT *data = new int[modules.size()*NUM_LABELS];
        // first set up data costs individually
        for ( int i = 0; i < modules.size(); i++ ) {
            for ( int l = 0; l < NUM_LABELS; l++ ) {
                // here we need to set the data cost from each module to the label using gc->setDataCost function
                // fill in the data array here
            }
        }

        // next set up smoothness costs individually
        // here we use a 512*512 matrix to represent the smooth cost
        EnergyT *smooth;
        // the outer space should pass a matrix to this module, so just fill in this variable with that argument.
        
        gc->setDataCost(data);
        gc->setSmoothCost(smooth);

        printf("\nBefore optimization energy is %d\n", gc->compute_energy());
        int iter_num = 0;
        while(1) {
            // use alpha-beta swap to do the optimization
            // and here the convergence is ensured using argument -1
            gc->swap(-1);
            
        }
    }
}