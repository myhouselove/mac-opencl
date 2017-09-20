//
//  main.cpp
//  OpenCL
//
//  Created by 王明勇 on 2017/9/19.
//  Copyright © 2017年 王明勇. All rights reserved.
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


const int ARRAY_SIZE = 100000;

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
    errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize, devices, NULL);
    
    //选取可用设备中的第一个
    commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);
    
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
                      float *a, float *b)
{
    memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                   sizeof(float) * ARRAY_SIZE, a, NULL);
    memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                   sizeof(float) * ARRAY_SIZE, b, NULL);
    memObjects[2] = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                   sizeof(float) * ARRAY_SIZE, NULL, NULL);
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

class TimerDelta{
public:
    TimerDelta(){
//        //loat time_use=0;
//        
//        //struct timezone tz; //后面有说明
//        gettimeofday(&start,NULL);//gettimeofday(&start,&tz);结果一样
//        //printf("start.tv_sec:%d\n",start.tv_sec);
//       // printf("start.tv_usec:%d\n",start.tv_usec);
//        sleep(3);
//        gettimeofday(&end,NULL);
//        printf("end.tv_sec:%d\n",end.tv_sec);
//        printf("end.tv_usec:%d\n",end.tv_usec);
//        time_use=(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);//微秒
//        printf("time_use is %f\n",time_use);
//        //输出：time_use is 3001410.000000
//        //下面的采用指针的方式也可以，但是要注意指针类型若不分配内存的话，编译正确，但是运行结果会不对

    }
    ~TimerDelta(){}
    
    void Start(){
        gettimeofday(&start,NULL);
        printf("start.tv_sec:%d\n",start.tv_sec);
        printf("start.tv_usec:%d\n",start.tv_usec);
    
    }
    void Stop(){
        gettimeofday(&end,NULL);
        printf("end.tv_sec:%d\n",end.tv_sec);
        printf("end.tv_usec:%d\n",end.tv_usec);
        delta=(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);//微秒
    
    }
    struct timeval start;
    struct timeval end;
    float delta;

};

int main(int argc, char** argv)
{
    cl_context context = 0;
    cl_command_queue commandQueue = 0;
    cl_program program = 0;
    cl_device_id device = 0;
    cl_kernel kernel = 0;
    cl_mem memObjects[3] = { 0, 0, 0 };
    cl_int errNum;
    const char* filename = "/Users/wangmingyong/Projects/OpenCL/OpenCL/HelloWorld.cl";
    // 一、选择OpenCL平台并创建一个上下文
    context = CreateContext();
    
    // 二、 创建设备并创建命令队列
    commandQueue = CreateCommandQueue(context, &device);
    
    //创建和构建程序对象
    program = CreateProgram(context, device, filename);//"HelloWorld.cl");
    
    // 四、 创建OpenCL内核并分配内存空间
    kernel = clCreateKernel(program, "hello_kernel", NULL);
    
    //创建要处理的数据
    float result[ARRAY_SIZE];
    float a[ARRAY_SIZE];
    float b[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        a[i] = (float)i;
        b[i] = (float)(ARRAY_SIZE - i);
    }
    TimerDelta *timer = new TimerDelta();
    timer->Start();
    
//    for(int j = 0;j <  ARRAY_SIZE;j++){
//        result[j] = a[j]*b[j]+a[j+1]*b[j+1];
//    }
//    timer->Stop();
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
    
    size_t globalWorkSize[1] = { ARRAY_SIZE };
    size_t localWorkSize[1] = { 1 };
    
    errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
                                    globalWorkSize, localWorkSize,
                                    0, NULL, NULL);
    
    // 六、 读取执行结果并释放OpenCL资源
    errNum = clEnqueueReadBuffer(commandQueue, memObjects[2], CL_TRUE,
                                 0, ARRAY_SIZE * sizeof(float), result,
                                 0, NULL, NULL);
    timer->Stop();
    std::cout<<"the delta is = "<<timer->delta<<std::endl;
    
//    for (int i = 0; i < ARRAY_SIZE; i++)
//    {
//        std::cout << result[i] << " ";
//    }
    std::cout << std::endl;
    std::cout << "Executed program succesfully." << std::endl;
    getchar();
    Cleanup(context, commandQueue, program, kernel, memObjects);
    
    return 0;
}
