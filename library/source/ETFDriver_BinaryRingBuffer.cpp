#include <ETFDriver_BinaryRingBuffer.hpp>
#include <etf_binary_ring_buffer.hpp>
#include <string>


void ETFDriver_BinaryRingBufferInitialize(void** workVector, uint8_t* folderName, uint32_t strlenFolderName, uint32_t sampleSize, uint32_t numSamplesPerFile, uint32_t numFiles, int32_t threadPriority){
    etf::BinaryRingBuffer* driver = new etf::BinaryRingBuffer();
    std::string folder((char*)folderName, strlenFolderName);
    driver->Initialize(folder.c_str(), sampleSize, numSamplesPerFile, numFiles, threadPriority);
    *workVector = reinterpret_cast<void*>(driver);
}

void ETFDriver_BinaryRingBufferTerminate(void* workVector){
    etf::BinaryRingBuffer* driver = reinterpret_cast<etf::BinaryRingBuffer*>(workVector);
    driver->Terminate();
    delete driver;
}

void ETFDriver_BinaryRingBufferStep(void* workVector, uint8_t* isOpen, uint32_t* numCachedSamples, uint8_t* sampleData, uint8_t startNewRingBuffer){
    etf::BinaryRingBuffer* driver = reinterpret_cast<etf::BinaryRingBuffer*>(workVector);
    *isOpen = static_cast<uint8_t>(driver->IsOpen());
    *numCachedSamples = driver->AddSample(sampleData, static_cast<bool>(startNewRingBuffer));
}

