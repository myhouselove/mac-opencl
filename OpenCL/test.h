//
//  test.h
//  OpenCL
//
//  Created by 王明勇 on 2017/11/6.
//  Copyright © 2017年 王明勇. All rights reserved.
//

#ifndef test_h
#define test_h

__kernel void hello_kernel(__global const int *a,
                           __global const int *b,
                           __global int *result,int  ha,
                           int  wb,int  wa)
{
//    int row = get_global_id(0);
//    //int col = get_global_id(1);
//    
//    
//    int sum = 0;
//    
//    
//    for(int j=0;j<wb;j++){
//        sum=0;
//        for(int i=0;i<wa;i++)
//        {
//            sum += a[row*wa+i] * b[i*wb+j];
//        }
//        result[row*wb+j] = sum;
//    }
    int i=get_global_id(0);
    result[i]=a[i]+b[i];
    
    
}
//__kernel void hello_kernel(__global const int *a,
//                           __global const int *b,
//                           __global float *result,int  ha,int  wa,
//                           int  wb)
//{
//    int row = get_global_id(1);//4
//    int col = get_global_id(0);//5
//
//
//    int sum = 0;
//    for(int i=0;i<wa;i++)
//    {
//        sum += a[row*wa+i] * b[i*wb+col];
//    }
//
//    result[row*wb+col] = 2;
//}

#endif /* test_h */
