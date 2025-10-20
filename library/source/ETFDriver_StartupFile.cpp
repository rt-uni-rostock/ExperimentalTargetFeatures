#include <ETFDriver_StartupFile.hpp>
#include <etf_startup_file.hpp>
#include <string>


void ETFDriver_StartupFileInitialize(void** workVector, uint8_t* filename, uint32_t strlenFilename, uint32_t maxNumBytes){
    etf::StartupFile* driver = new etf::StartupFile();
    std::string file((char*)filename, strlenFilename);
    driver->Initialize(file.c_str(), maxNumBytes);
    *workVector = reinterpret_cast<void*>(driver);
}

void ETFDriver_StartupFileTerminate(void* workVector){
    etf::StartupFile* driver = reinterpret_cast<etf::StartupFile*>(workVector);
    driver->Terminate();
    delete driver;
}

void ETFDriver_StartupFileStep(void* workVector, uint8_t* bytes, uint32_t* length, uint32_t maxNumBytes){
    etf::StartupFile* driver = reinterpret_cast<etf::StartupFile*>(workVector);
    driver->GetBytes(bytes, length, maxNumBytes);
}

