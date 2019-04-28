#include <dlpack/dlpack.h>
#include <cvm/runtime/module.h>
#include <cvm/runtime/registry.h>
#include <cvm/runtime/packed_func.h>

#include <fstream>
#include <iterator>
#include <algorithm>
#include <stdio.h>
#include <string.h>

#include <time.h>

int dtype_code = kDLInt;
int dtype_bits = 32;
int dtype_lanes = 1;
int device_type = kDLCPU;
int device_id = 0;

long RunCVM(DLTensor* x, CVMByteArray& params_arr, std::string json_data,
        cvm::runtime::Module &mod_syslib  ,  std::string runtime_name, DLTensor *y, int devicetype) {
		auto t1 = clock();
	// get global function module for graph runtime
    auto mf =  (*cvm::runtime::Registry::Get("cvm." + runtime_name + ".create"));
    cvm::runtime::Module mod = mf(json_data, mod_syslib, static_cast<int>(x->ctx.device_type), device_id);

    // load image data saved in binary
    // std::ifstream data_fin("cat.bin", std::ios::binary);
    // data_fin.read(static_cast<char*>(x->data), 3 * 224 * 224 * 4);

    // get the function from the module(set input data)
    cvm::runtime::PackedFunc set_input = mod.GetFunction("set_input");
    set_input("data", x);


    // get the function from the module(load patameters)
    cvm::runtime::PackedFunc load_params = mod.GetFunction("load_params");
    load_params(params_arr);


		auto t2 = clock();
    // get the function from the module(run it)
    cvm::runtime::PackedFunc run = mod.GetFunction("run");
    run();
    // get the function from the module(get output data)
    cvm::runtime::PackedFunc get_output = mod.GetFunction("get_output");
    get_output(0, y);

//    auto y_iter = static_cast<int*>(y->data);
//    // get the maximum position in output vector
//    auto max_iter = std::max_element(y_iter, y_iter + out_shape[1]);
//    auto max_index = std::distance(y_iter, max_iter);
//    std::cout << "The maximum position in output vector is: " << max_index << std::endl;

//    for (auto i = 0; i < out_shape[1]; i++) {
//        if (i < 1000)
//            std::cout << y_iter[i] << " ";
//    }
//    std::cout << "\n";
//
//
//    cvmArrayFree(y);
}

int main()
{
//    cvm::runtime::Module mod_org = cvm::runtime::Module::LoadFromFile("/tmp/imagenet_llvm.org.so");///tmp/imagenet_llvm.org.so
    cvm::runtime::Module mod_syslib = (*cvm::runtime::Registry::Get("module._GetSystemLib"))();

    for(int in = 0; in < 1; in++){

        std::vector<unsigned long> tshape;
        std::vector<unsigned char> tdata;

        DLTensor* x;
        int in_ndim = 4;
        int64_t in_shape[4] = {1, 3, 224, 224};
        CVMArrayAlloc(in_shape, in_ndim, dtype_code, dtype_bits, dtype_lanes, device_type, device_id, &x);
        auto x_iter = static_cast<int*>(x->data);
        for (auto i = 0; i < 3*224*224; i++) {
            x_iter[i] = i % 225 - 127;
        }

            std::cout << "\n";
        clock_t read_t1 = clock();
        // parameters in binary
        std::ifstream params_in("/tmp/imagenet_cuda_cvm.params", std::ios::binary);
        std::string params_data((std::istreambuf_iterator<char>(params_in)), std::istreambuf_iterator<char>());
        params_in.close();

        // parameters need to be cvmByteArray type to indicate the binary data
        CVMByteArray params_arr;
        params_arr.data = params_data.c_str();
        params_arr.size = params_data.length();

        DLTensor* y1;
        int out_ndim = 2;
        int64_t out_shape[2] = {1, 1000, };

        std::ifstream json_in("/tmp/imagenet_cuda_cvm.json", std::ios::in);
        std::string json_data((std::istreambuf_iterator<char>(json_in)), std::istreambuf_iterator<char>());
        json_in.close();

        DLTensor* gpu_x, *gpu_y;
        CVMArrayAlloc(in_shape, in_ndim, dtype_code, dtype_bits, dtype_lanes, kDLGPU, device_id, &gpu_x);
        CVMArrayAlloc(out_shape, out_ndim, dtype_code, dtype_bits, dtype_lanes, kDLGPU, device_id, &gpu_y);
        CVMStreamHandle stream1;
        CVMStreamCreate(kDLGPU, device_id, &stream1);
        CVMArrayCopyFromTo(x, gpu_x, stream1);

        DLTensor* y2;
        CVMArrayAlloc(out_shape, out_ndim, dtype_code, dtype_bits, dtype_lanes, device_type, device_id, &y2);
        clock_t cvm_start = clock();
        clock_t delta = 0;
        clock_t last;
        for (int i = 0; i < 1; i++) {
            delta += RunCVM(gpu_x, params_arr, json_data, mod_syslib, "cvm_runtime", gpu_y, (int)kDLGPU);
        }
        clock_t cvm_end = clock();
        std::cout << (cvm_end - cvm_start - delta) * 1000 / CLOCKS_PER_SEC << "ms" << std::endl;
        std::cout << "cvm runtime: " << (cvm_end - cvm_start)*1.0 / CLOCKS_PER_SEC << " s" << std::endl;
        CVMArrayCopyFromTo(gpu_y, y2, stream1);
        //cvmArrayFree(y_cpu);
        for(int i = 0; i < 10; i++){
            std::cout << static_cast<int32_t*>(y2->data)[i] << " ";
        }
        std::cout << std::endl;
        int32_t ret[] = {-47, -6, -28, 95, -34, 66, -54, -8, -83, -35};
//        std::cout << (memcmp(y1->data, y2->data, 1000*sizeof(int32_t)) == 0 ? "pass" : "failed") << std::endl;
        CVMArrayFree(x);
        CVMArrayFree(gpu_x);
        CVMArrayFree(gpu_y);
        //    cvmArrayFree(t_gpu_x);
        //    cvmArrayFree(t_gpu_y);
        CVMArrayFree(y2);
    }
    return 0;
}
