
#pragma OPENCL EXTENSION cl_khr_fp64: enable
#pragma OPENCL EXTENSION cl_amd_printf : enable


#define TS 16

__kernel void hello_kernel(const __global int* A,
                     const __global int* B,
                     __global int* C, int M,  int N,  int K) {
    
    // Thread identifiers
   // int size = get_local_size(1);
    const int row = get_local_id(0); // Local row ID (max: TS)
    const int col = get_local_id(1); // Local col ID (max: TS)
    const int globalRow = TS*get_group_id(0) + row; // Row ID of C (0..M)
    const int globalCol = TS*get_group_id(1) + col; // Col ID of C (0..N)

    
    //barrier(CLK_LOCAL_MEM_FENCE);
    printf("here\n");
    //printf("group row=%d,col=%d\n",get_group_id(0),get_group_id(1));
    // Local memory to fit a tile of TS*TS elements of A and B
    __local int Asub[TS][TS];
    __local int Bsub[TS][TS];
    
    // Initialise the accumulation register
    int acc = 0;
    
    // Loop over all tiles
    const int numTiles = K/TS;
    for (int t=0; t<numTiles; t++) {
        
        // Load one tile of A and B into local memory
        const int tiledRow = TS*t + row;
        const int tiledCol = TS*t + col;
        Asub[col][row] = A[tiledCol*M + globalRow];
        Bsub[col][row] = B[globalCol*K + tiledRow];
        printf("Asub[%d][%d]=A[%d]=%d\t",col,row,tiledCol*M + globalRow,A[tiledCol*M + globalRow]);
        // Synchronise to make sure the tile is loaded
        barrier(CLK_LOCAL_MEM_FENCE);
        //for(int j=0;j<)
        // Perform the computation for a single tile
        for (int k=0; k<TS; k++) {
            acc += Asub[k][row] * Bsub[col][k];
            //printf("acc[%d][%d]=%d\n",k,row,Asub[k][row]);
        }
       // printf("acc = %d\n",acc);
        // Synchronise before loading the next tile
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    
    // Store the final result in C
    C[globalCol*M + globalRow] = acc;
}
