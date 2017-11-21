//
//  main.cpp
//  OpenCL
//
//  Created by wmy on 2017/9/19.
//  Copyright © 2017年 wmy. All rights reserved.
//


#include <OpenCL/OpenCL.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/time.h>
#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include <mach/mach_time.h>
#include <boost/algorithm/string.hpp>


using namespace std;

//const int ARRAY_SIZE = 100000;


//4*3---3*5

const int midle = 512;
const int heightA = 512;

const int widthB = 512;
//const int heightB = 3;

//一、 选择OpenCL平台并创建一个上下文
cl_context CreateContext()
{
    cl_int errNum;
    cl_uint numPlatforms;
    cl_platform_id firstPlatformId;
    cl_context context = NULL;
    
    //选择可用的平台中的第一个
    errNum = clGetPlatformIDs(1, &firstPlatformId, &numPlatforms);
    if (errNum != CL_SUCCESS || numPlatforms <= 0)
    {
        std::cerr << "Failed to find any OpenCL platforms." << std::endl;
        return NULL;
    }
    
    //创建一个OpenCL上下文环境
    cl_context_properties contextProperties[] =
    {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)firstPlatformId,
        0
    };
    context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU,
                                      NULL, NULL, &errNum);
    
    return context;
}


//二、 创建设备并创建命令队列
cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device)
{
    cl_int errNum;
    cl_device_id *devices;
    cl_command_queue commandQueue = NULL;
    size_t deviceBufferSize = -1;
    
    // 获取设备缓冲区大小
    errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize);
    
    if (deviceBufferSize <= 0)
    {
        std::cerr << "No devices available.";
        return NULL;
    }
    
    // 为设备分配缓存空间
    devices = new cl_device_id[deviceBufferSize / sizeof(cl_device_id)];
    printf("deviceBufferSize / sizeof(cl_device_id)=%ld\n",deviceBufferSize / sizeof(cl_device_id));
    errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize, devices, NULL);
    
    
    
  
//    size_t      valueSize;
//    clGetDeviceInfo(devices[0], CL_DEVICE_NAME, 0, NULL, &valueSize);
//    char* value = (char*) malloc(valueSize);
//    clGetDeviceInfo(devices[0], CL_DEVICE_NAME, valueSize, value, NULL);
//    printf("Device1 Name: %s\n", value);
//    free(value);
//    
//    clGetDeviceInfo(devices[1], CL_DEVICE_NAME, 0, NULL, &valueSize);
//    value = (char*) malloc(valueSize);
//    clGetDeviceInfo(devices[1], CL_DEVICE_NAME, valueSize, value, NULL);
//    printf("Device2 Name: %s\n", value);
//    free(value);
    
    
    //选取可用设备中的第一个
    commandQueue = clCreateCommandQueue(context, devices[1], 0, NULL);
    
    *device = devices[0];
    delete[] devices;
    return commandQueue;
}


// 三、创建和构建程序对象
cl_program CreateProgram(cl_context context, cl_device_id device, const char* fileName)
{
    cl_int errNum;
    cl_program program;
    
    std::ifstream kernelFile(fileName, std::ios::in);
    if (!kernelFile.is_open())
    {
        std::cerr << "Failed to open file for reading: " << fileName << std::endl;
        return NULL;
    }
    
    std::ostringstream oss;
    oss << kernelFile.rdbuf();
    
    std::string srcStdStr = oss.str();
    const char *srcStr = srcStdStr.c_str();
    program = clCreateProgramWithSource(context, 1,
                                        (const char**)&srcStr,
                                        NULL, NULL);
    
    errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    
    return program;
}

//创建和构建程序对象
bool CreateMemObjects(cl_context context, cl_mem memObjects[3],
                      int *a, int *b)
{
    memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                   sizeof(int) * midle*heightA, a, NULL);
    memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                   sizeof(int) * widthB*midle, b, NULL);
    memObjects[2] = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                   sizeof(int) * widthB*heightA, NULL, NULL);
    return true;
}


// 释放OpenCL资源
void Cleanup(cl_context context, cl_command_queue commandQueue,
             cl_program program, cl_kernel kernel, cl_mem memObjects[3])
{
    for (int i = 0; i < 3; i++)
    {
        if (memObjects[i] != 0)
            clReleaseMemObject(memObjects[i]);
    }
    if (commandQueue != 0)
        clReleaseCommandQueue(commandQueue);
    
    if (kernel != 0)
        clReleaseKernel(kernel);
    
    if (program != 0)
        clReleaseProgram(program);
    
    if (context != 0)
        clReleaseContext(context);
}


void checkError(cl_int error, int line) {
    if (error != CL_SUCCESS) {
        switch (error) {
            case CL_DEVICE_NOT_FOUND:                 printf("-- Error at %d:  Device not found.\n", line); break;
            case CL_DEVICE_NOT_AVAILABLE:             printf("-- Error at %d:  Device not available\n", line); break;
            case CL_COMPILER_NOT_AVAILABLE:           printf("-- Error at %d:  Compiler not available\n", line); break;
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:    printf("-- Error at %d:  Memory object allocation failure\n", line); break;
            case CL_OUT_OF_RESOURCES:                 printf("-- Error at %d:  Out of resources\n", line); break;
            case CL_OUT_OF_HOST_MEMORY:               printf("-- Error at %d:  Out of host memory\n", line); break;
            case CL_PROFILING_INFO_NOT_AVAILABLE:     printf("-- Error at %d:  Profiling information not available\n", line); break;
            case CL_MEM_COPY_OVERLAP:                 printf("-- Error at %d:  Memory copy overlap\n", line); break;
            case CL_IMAGE_FORMAT_MISMATCH:            printf("-- Error at %d:  Image format mismatch\n", line); break;
            case CL_IMAGE_FORMAT_NOT_SUPPORTED:       printf("-- Error at %d:  Image format not supported\n", line); break;
            case CL_BUILD_PROGRAM_FAILURE:            printf("-- Error at %d:  Program build failure\n", line); break;
            case CL_MAP_FAILURE:                      printf("-- Error at %d:  Map failure\n", line); break;
            case CL_INVALID_VALUE:                    printf("-- Error at %d:  Invalid value\n", line); break;
            case CL_INVALID_DEVICE_TYPE:              printf("-- Error at %d:  Invalid device type\n", line); break;
            case CL_INVALID_PLATFORM:                 printf("-- Error at %d:  Invalid platform\n", line); break;
            case CL_INVALID_DEVICE:                   printf("-- Error at %d:  Invalid device\n", line); break;
            case CL_INVALID_CONTEXT:                  printf("-- Error at %d:  Invalid context\n", line); break;
            case CL_INVALID_QUEUE_PROPERTIES:         printf("-- Error at %d:  Invalid queue properties\n", line); break;
            case CL_INVALID_COMMAND_QUEUE:            printf("-- Error at %d:  Invalid command queue\n", line); break;
            case CL_INVALID_HOST_PTR:                 printf("-- Error at %d:  Invalid host pointer\n", line); break;
            case CL_INVALID_MEM_OBJECT:               printf("-- Error at %d:  Invalid memory object\n", line); break;
            case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:  printf("-- Error at %d:  Invalid image format descriptor\n", line); break;
            case CL_INVALID_IMAGE_SIZE:               printf("-- Error at %d:  Invalid image size\n", line); break;
            case CL_INVALID_SAMPLER:                  printf("-- Error at %d:  Invalid sampler\n", line); break;
            case CL_INVALID_BINARY:                   printf("-- Error at %d:  Invalid binary\n", line); break;
            case CL_INVALID_BUILD_OPTIONS:            printf("-- Error at %d:  Invalid build options\n", line); break;
            case CL_INVALID_PROGRAM:                  printf("-- Error at %d:  Invalid program\n", line); break;
            case CL_INVALID_PROGRAM_EXECUTABLE:       printf("-- Error at %d:  Invalid program executable\n", line); break;
            case CL_INVALID_KERNEL_NAME:              printf("-- Error at %d:  Invalid kernel name\n", line); break;
            case CL_INVALID_KERNEL_DEFINITION:        printf("-- Error at %d:  Invalid kernel definition\n", line); break;
            case CL_INVALID_KERNEL:                   printf("-- Error at %d:  Invalid kernel\n", line); break;
            case CL_INVALID_ARG_INDEX:                printf("-- Error at %d:  Invalid argument index\n", line); break;
            case CL_INVALID_ARG_VALUE:                printf("-- Error at %d:  Invalid argument value\n", line); break;
            case CL_INVALID_ARG_SIZE:                 printf("-- Error at %d:  Invalid argument size\n", line); break;
            case CL_INVALID_KERNEL_ARGS:              printf("-- Error at %d:  Invalid kernel arguments\n", line); break;
            case CL_INVALID_WORK_DIMENSION:           printf("-- Error at %d:  Invalid work dimensionsension\n", line); break;
            case CL_INVALID_WORK_GROUP_SIZE:          printf("-- Error at %d:  Invalid work group size\n", line); break;
            case CL_INVALID_WORK_ITEM_SIZE:           printf("-- Error at %d:  Invalid work item size\n", line); break;
            case CL_INVALID_GLOBAL_OFFSET:            printf("-- Error at %d:  Invalid global offset\n", line); break;
            case CL_INVALID_EVENT_WAIT_LIST:          printf("-- Error at %d:  Invalid event wait list\n", line); break;
            case CL_INVALID_EVENT:                    printf("-- Error at %d:  Invalid event\n", line); break;
            case CL_INVALID_OPERATION:                printf("-- Error at %d:  Invalid operation\n", line); break;
            case CL_INVALID_GL_OBJECT:                printf("-- Error at %d:  Invalid OpenGL object\n", line); break;
            case CL_INVALID_BUFFER_SIZE:              printf("-- Error at %d:  Invalid buffer size\n", line); break;
            case CL_INVALID_MIP_LEVEL:                printf("-- Error at %d:  Invalid mip-map level\n", line); break;
            case -1024:                               printf("-- Error at %d:  *clBLAS* Functionality is not implemented\n", line); break;
            case -1023:                               printf("-- Error at %d:  *clBLAS* Library is not initialized yet\n", line); break;
            case -1022:                               printf("-- Error at %d:  *clBLAS* Matrix A is not a valid memory object\n", line); break;
            case -1021:                               printf("-- Error at %d:  *clBLAS* Matrix B is not a valid memory object\n", line); break;
            case -1020:                               printf("-- Error at %d:  *clBLAS* Matrix C is not a valid memory object\n", line); break;
            case -1019:                               printf("-- Error at %d:  *clBLAS* Vector X is not a valid memory object\n", line); break;
            case -1018:                               printf("-- Error at %d:  *clBLAS* Vector Y is not a valid memory object\n", line); break;
            case -1017:                               printf("-- Error at %d:  *clBLAS* An input dimension (M,N,K) is invalid\n", line); break;
            case -1016:                               printf("-- Error at %d:  *clBLAS* Leading dimension A must not be less than the size of the first dimension\n", line); break;
            case -1015:                               printf("-- Error at %d:  *clBLAS* Leading dimension B must not be less than the size of the second dimension\n", line); break;
            case -1014:                               printf("-- Error at %d:  *clBLAS* Leading dimension C must not be less than the size of the third dimension\n", line); break;
            case -1013:                               printf("-- Error at %d:  *clBLAS* The increment for a vector X must not be 0\n", line); break;
            case -1012:                               printf("-- Error at %d:  *clBLAS* The increment for a vector Y must not be 0\n", line); break;
            case -1011:                               printf("-- Error at %d:  *clBLAS* The memory object for Matrix A is too small\n", line); break;
            case -1010:                               printf("-- Error at %d:  *clBLAS* The memory object for Matrix B is too small\n", line); break;
            case -1009:                               printf("-- Error at %d:  *clBLAS* The memory object for Matrix C is too small\n", line); break;
            case -1008:                               printf("-- Error at %d:  *clBLAS* The memory object for Vector X is too small\n", line); break;
            case -1007:                               printf("-- Error at %d:  *clBLAS* The memory object for Vector Y is too small\n", line); break;
            case -1001:                               printf("-- Error at %d:  Code -1001: no GPU available?\n", line); break;
            default:                                  printf("-- Error at %d:  Unknown with code %d\n", line, error);
        }
        exit(1);
    }
}
#define TIMES 10
#define TS 16

int main(int argc, char** argv)
{
    

    
    cl_context context = 0;
    cl_command_queue commandQueue = 0;
    cl_program program = 0;
    cl_device_id device = 0;
    cl_kernel kernel = 0;
    cl_mem memObjects[3] = { 0, 0, 0 };
    cl_int errNum;
   // uint64_t t1,t2,t3;
    clock_t t1,t2,t3,t4;

    
    const char* filename = "/Users/wangmingyong/Projects/OpenCL/OpenCL/HelloWorld.cl";
    // 一、选择OpenCL平台并创建一个上下文
    context = CreateContext();
    
    // 二、 创建设备并创建命令队列
    commandQueue = CreateCommandQueue(context, &device);
    
    
    size_t      maxWorkItemPerGroup;
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE,sizeof(maxWorkItemPerGroup), &maxWorkItemPerGroup, NULL);
    printf("maxWorkItemPerGroup: %zd\n", maxWorkItemPerGroup);
    
    size_t      valueSize;
    clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &valueSize);
    char* value = (char*) malloc(valueSize);
    clGetDeviceInfo(device, CL_DEVICE_NAME, valueSize, value, NULL);
    printf("Device Name: %s\n", value);
    free(value);
    
    //创建和构建程序对象
    program = CreateProgram(context, device, filename);//"HelloWorld.cl");
    
    // 四、 创建OpenCL内核并分配内存空间
    kernel = clCreateKernel(program, "hello_kernel", NULL);
    
    //创建要处理的数据
    int result[widthB*heightA]{0};
    int a[midle*heightA];
    int b[widthB*midle];
    for (int i = 0; i < heightA; i++)
    {
        for(int j = 0;j < midle;j++)
        {
            a[i*midle+j]=2;//10.0f * ((int) rand() / (int) RAND_MAX);
        }
        
    }
    
    
    for (int k = 0; k < midle; k++)
    {
        for(int m = 0;m < widthB;m++)
        {
            b[k*widthB+m]=3;//10.0f * ((int) rand() / (int) RAND_MAX);
        }
        
    }

    t1 = clock();  //mach_absolute_time();
    //printf("t1 = %.8f\n",(double)t1);
    
    for(int tt=0;tt<TIMES;tt++){
        for(int l=0;l<heightA;l++){
            for(int n = 0;n<widthB;n++){
                for(int q=0;q<midle;q++){
                    result[l*widthB+n] +=a [l*midle+q]*b[q*widthB+n];
                    
                }
                //std::cout<<"r = "<<result[l*widthB+n]<<std::endl;
            }
        }
    }
    t2 = clock(); //mach_absolute_time();
    //printf("t2 = %.8f\n",(double)t2);
    
    //创建内存对象
    if (!CreateMemObjects(context, memObjects, a, b))
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }
    
    // 五、 设置内核数据并执行内核
    errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]);
    errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);
    errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]);
    errNum |= clSetKernelArg(kernel, 3, sizeof(int), &heightA);
    errNum |= clSetKernelArg(kernel, 4, sizeof(int), &widthB);
    errNum |= clSetKernelArg(kernel, 5, sizeof(int), &midle);
    
    size_t globalWorkSize[2];
    globalWorkSize[0]= heightA;
    globalWorkSize[1]=widthB;
    size_t localWorkSize[2] ;
    localWorkSize[0]= TS;
    localWorkSize[1]= TS;
    t3 = clock();
    for(int run=0;run<TIMES;run++){
        errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL,
                                        globalWorkSize, localWorkSize,
                                        0, NULL, NULL);
        checkError(errNum,__LINE__);
        if(errNum == CL_SUCCESS)
            cout<<"enqueue success!"<<endl;
        else
            printf("errNum= %d\n",errNum);
        
        //mach_absolute_time();
        // 六、 读取执行结果并释放OpenCL资源
        errNum = clEnqueueReadBuffer(commandQueue, memObjects[2], CL_TRUE,
                                     0, widthB*heightA * sizeof(int), result,
                                     0, NULL, NULL);
        //    for(int p=0;p<20;p++){
        //        cout<<"new ="<<result[p];
        //    }

    }
    

        t4 = clock();


    
    printf("cpu t = %.8f\n",(float)(t2-t1)/CLOCKS_PER_SEC/TIMES);
    printf("gpu t = %.8f \n",(double)(t4-t3)/CLOCKS_PER_SEC/TIMES);

    std::cout << std::endl;
    std::cout << "Executed program succesfully." << std::endl;
    getchar();
    Cleanup(context, commandQueue, program, kernel, memObjects);
    
    return 0;
}
