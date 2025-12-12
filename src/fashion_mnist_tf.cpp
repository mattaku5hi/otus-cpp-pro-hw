#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <tensorflow/c/c_api.h>
#include <vector>

#include "csv_parser.h"


static void DeallocateTensor(void* data, size_t, void*) 
{ 
    ::free(data); 
}

// Создание тензора из данных
static TF_Tensor* MakeTensor(TF_DataType type, const std::vector<int64_t>& dims, const void* data, size_t len) 
{ 
    void* pBuf = ::malloc(len);
    if(pBuf == nullptr)
    {
        return nullptr;
    }

    std::memcpy(pBuf, data, len);
    return TF_NewTensor(type, dims.data(), static_cast<int>(dims.size()), pBuf, len, &DeallocateTensor, nullptr);
}

// Класс-обёртка для TF_Status.
struct TFStatus 
{
    TFStatus() : 
        s(TF_NewStatus()) 
    {

    }
    ~TFStatus() 
    { 
        TF_DeleteStatus(s); 
    }
    TF_Status* s;
    bool ok() const 
    { 
        return TF_GetCode(s) == TF_OK; 
    }
    const char* msg() const 
    { 
        return TF_Message(s); 
    }
};


// Поиск входного Placeholder в графе SavedModel
static TF_Output find_input_placeholder(TF_Graph* graph) 
{ 
    size_t pos = 0;
    TF_Operation* op = nullptr;
    TF_Output none{nullptr, 0};
    while((op = TF_GraphNextOperation(graph, &pos)) != nullptr)
    {
        const char* type = TF_OperationOpType(op);
        if(std::strcmp(type, "Placeholder") == 0)
        {
            const char* name = TF_OperationName(op);
            if(std::strstr(name, "input") != nullptr)
            {
                return TF_Output{op, 0};
            }
        }
    }
    pos = 0;
    while((op = TF_GraphNextOperation(graph, &pos)) != nullptr)
    {
        const char* type = TF_OperationOpType(op);
        if(std::strcmp(type, "Placeholder") == 0)
        {
            return TF_Output{op, 0};
        }
    }
    return none;
}

// Поиск выходного узла (предпочтительно Softmax; fallback на другие варианты)
static TF_Output find_output_op(TF_Graph* graph) 
{ 
    size_t pos = 0;
    TF_Operation* op = nullptr;
    TF_Output none{nullptr, 0};
    while((op = TF_GraphNextOperation(graph, &pos)) != nullptr)
    {
        const char* type = TF_OperationOpType(op);
        if(std::strcmp(type, "Softmax") == 0)
        {
            return TF_Output{op, 0};
        }
    }
    pos = 0;
    while((op = TF_GraphNextOperation(graph, &pos)) != nullptr)
    {
        const char* type = TF_OperationOpType(op);
        if(std::strcmp(type, "StatefulPartitionedCall") == 0)
        {
            return TF_Output{op, 0};
        }
    }
    pos = 0;
    while((op = TF_GraphNextOperation(graph, &pos)) != nullptr)
    {
        const char* type = TF_OperationOpType(op);
        if(std::strcmp(type, "Identity") == 0)
        {
            return TF_Output{op, 0};
        }
    }
    return none;
}

int main(int argc, char** argv) 
{ 
    if (argc < 3) 
    { 
        std::cerr << "Usage: " << argv[0] << " <test.csv> <saved_model_dir>" << std::endl;
        return 1;
    }
    const std::string test_path = argv[1];
    const std::string model_dir = argv[2];

    // Загружаем SavedModel через TF C API.
    TFStatus status;
    TF_Graph* graph = TF_NewGraph();
    TF_SessionOptions* sess_opts = TF_NewSessionOptions();

    const char* tags[] = {"serve"};
    TF_Buffer* meta = TF_NewBuffer();
    TF_Session* sess = TF_LoadSessionFromSavedModel(sess_opts, nullptr, model_dir.c_str(), tags, 1, graph, meta, status.s);
    TF_DeleteBuffer(meta);
    TF_DeleteSessionOptions(sess_opts);

    if(!status.ok() || sess == nullptr)
    {
        std::cerr << "Failed to load SavedModel from '" << model_dir << "': " << status.msg() << std::endl;
        if(graph)
        {
            TF_DeleteGraph(graph);
        }
        return 1;
    }

    // Ищем входной Placeholder и выходной узел в графе SavedModel
    TF_Output input = find_input_placeholder(graph);
    TF_Output output = find_output_op(graph);
    if(input.oper == nullptr || output.oper == nullptr)
    {
        std::cerr << "Failed to resolve input/output ops in graph" << std::endl;
        TF_DeleteSession(sess, status.s);
        TF_DeleteGraph(graph);
        return 1;
    }

    std::ifstream fin(test_path);
    if(!fin.is_open())
    {
        std::cerr << "Failed to open test data: " << test_path << std::endl;
        TF_DeleteSession(sess, status.s);
        TF_DeleteGraph(graph);
        return 1;
    }

    const int batch_size = 128;
    std::string line;
    std::vector<float> batch; batch.resize(batch_size * 28 * 28);
    std::vector<int> labels; labels.resize(batch_size);
    size_t in_batch = 0;
    size_t total = 0, correct = 0;

    // Пакетная обработка (batch) ускоряет инференс за счёт уменьшения числа вызовов TF_SessionRun
    auto flush_batch = [&](size_t cur_batch) 
    { 
        if(cur_batch == 0)
        {
            return;
        }
        std::vector<int64_t> dims = { static_cast<int64_t>(cur_batch), 28, 28, 1 };
        size_t bytes = cur_batch * 28 * 28 * sizeof(float);
        TF_Tensor* in_tensor = MakeTensor(TF_FLOAT, dims, batch.data(), bytes);
        if(in_tensor == nullptr)
        {
            std::cerr << "Failed to allocate input tensor" << std::endl;
            return;
        }

        TF_Output inputs[1] = { input };
        TF_Tensor* input_tensors[1] = { in_tensor };

        TF_Output outputs[1] = { output };
        TF_Tensor* output_tensors[1] = { nullptr };

        TF_SessionRun(sess,
                      nullptr,
                      inputs, input_tensors, 1,
                      outputs, output_tensors, 1,
                      nullptr, 0,
                      nullptr,
                      status.s);

        TF_DeleteTensor(in_tensor);
        if(status.ok() == false)
        {
            std::cerr << "SessionRun failed: " << status.msg() << std::endl;
            if(output_tensors[0])
            {
                TF_DeleteTensor(output_tensors[0]);
            }
            return;
        }

        TF_Tensor* out = output_tensors[0];
        const float* out_data = static_cast<const float*>(TF_TensorData(out));
        int64_t classes = (TF_NumDims(out) == 2) ? TF_Dim(out, 1) : 10;

        for(size_t i = 0; i < cur_batch; ++i)
        {
            int argmax = 0;
            float best = -std::numeric_limits<float>::infinity();
            size_t base = i * static_cast<size_t>(classes);
            for(int c = 0; c < static_cast<int>(classes); ++c) 
            { 
                float v = out_data[base + static_cast<size_t>(c)];
                if(v > best) 
                { 
                    best = v; argmax = c; 
                }
            }
            if(argmax == labels[i])
            {
                ++correct;
            }
            ++total;
        }

        TF_DeleteTensor(out);
    };

    std::vector<float> img(784);
    while(std::getline(fin, line)) 
    { 
        int label = -1;
        if(csv_parser::parseRow<float>(line, label, img, true) == false)
        {
            continue;
        }
        labels[in_batch] = label;
        std::memcpy(&batch[in_batch * 28 * 28], img.data(), 784 * sizeof(float));
        ++in_batch;
        if(in_batch == static_cast<size_t>(batch_size))
        {
            flush_batch(in_batch);
            in_batch = 0;
        }
    }
    if(in_batch)
    {
        flush_batch(in_batch);
    }

    if(total == 0)
    {
        std::cerr << "No valid rows in test data" << std::endl;
        TF_DeleteSession(sess, status.s);
        TF_DeleteGraph(graph);
        return 1;
    }

    double acc = static_cast<double>(correct) / static_cast<double>(total);
    std::cout << std::fixed << std::setprecision(3) << acc << std::endl;

    TF_DeleteSession(sess, status.s);
    TF_DeleteGraph(graph);
    return 0;
}
