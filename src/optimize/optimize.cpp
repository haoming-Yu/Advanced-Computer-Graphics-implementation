#include "optimize/optimize.h"
#include "energy/energy.h"

void OptimizeBySwap(int row,int col,std::vector<module> &modules,std::vector<int> &res)
{
    row /= 3;
    col /= 3;
    try{
        GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(col, row, NUM_LABELS);
        
        EnergyType *data = new EnergyType[modules.size()*NUM_LABELS];
        // first set up data costs individually
        for ( int i = 0; i < modules.size(); i++ ) {
            for ( int l = 0; l < NUM_LABELS; l++ ) {
                // here we need to set the data cost from each module to the label using gc->setDataCost function
                // fill in the data array here
                EnergyType data_pair = GetDataCost(modules[i],l);
                data[i*NUM_LABELS + l] = data_pair;
            }
        }

        // next set up smoothness costs individually
        // here we use a 512*512 matrix to represent the smooth cost
        EnergyType *smooth = new EnergyType[NUM_LABELS*NUM_LABELS];
        for ( int i = 0; i < NUM_LABELS; i++ ) {
            for ( int j = 0; j < NUM_LABELS; j++ ) {
                smooth[i*NUM_LABELS + j] = GetSmoothCost(i, j);
            }
        }
        // the outer space should pass a matrix to this module, so just fill in this variable with that argument.
        
        gc->setDataCost(data);
        gc->setSmoothCost(smooth);

        printf("\nBefore optimization energy is %lld\n", gc->compute_energy());
        gc->swap(-1);
        printf("\nAfter optimization energy is %lld\n", gc->compute_energy());
        
        for ( int i = 0; i < modules.size(); i++ ) {
            res.push_back(gc->whatLabel(i));
        }
        delete gc;
        delete[] data;
        delete[] smooth;
    }
    catch (GCException e){
		e.Report();
	}

}