#include "optimize/optimize.h"
#include "energy/energy.h"

void OptimizeBySwap(int row,int col,std::vector<module> &modules,std::vector<int> &res)
{
    row /= 3;
    col /= 3;
    try{
        GCoptimizationGeneralGraph *gc = new GCoptimizationGeneralGraph(modules.size(),512);
        
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
        // for ( int i = 0; i < NUM_LABELS; i++ ) {
        //     for ( int j = 0; j < NUM_LABELS; j++ ) {
        //         smooth[i*NUM_LABELS + j] = GetSmoothCost(i, j);
        //     }
        // }
        // In order to use the GCO lib, we have to ensure that the matrix is symmetric
        for ( int y = 0; y < NUM_LABELS; y++ ) {
            for (int x = y; x < NUM_LABELS; x++) {
                EnergyType tmp_store = GetSmoothCost(x, y);
                smooth[x*NUM_LABELS + y] = tmp_store;
                if (x != y) {
                    smooth[y*NUM_LABELS + x] = tmp_store;
                }
            }
        }
        // the outer space should pass a matrix to this module, so just fill in this variable with that argument.
        
        gc->setDataCost(data);
        gc->setSmoothCost(smooth);
        // add edge
        // row edge
        for(int i=0;i<row ;++i)
        {
            for(int j=0;j<col-1;++j)
            {
                gc->setNeighbors(i*row+j,i*row+j+1,GetEdgeWeight(modules[i*row+j].data,modules[i*row+j+1].data));
            }
        }
        for(int i=0;i<row -1;++i)
        {
            for(int j=0;j<col;++j)
            {
                gc->setNeighbors(i*row+j,(i+1)*row+j,GetEdgeWeight(modules[i*row+j].data,modules[(i+1)*row+j].data));
            }
        }
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