/**
 * @file etf_startup_file.hpp
 * @author Robert Damerius (damerius.mail@gmail.com)
 * @brief Experimental target feature for reading a file from a disk at startup and output the binary content during execution.
 * @date 2025-10-20
 * @copyright Copyright (c) 2025 Robert Damerius
 */
#pragma once


/* Include standard libraries */
#include <cstdint>
#include <fstream>
#include <vector>


/* Default namespace for experimental target features */
namespace etf {


/**
 * @brief Startup file for reading a file during the initialization step of the real-time application and output the binary
 * data during execution.
 */
class StartupFile {
    public:
        /**
         * @brief Construct a new startup file object.
         */
        StartupFile(){}

        /**
         * @brief Destroy the startup file object.
         */
        ~StartupFile(){ Terminate(); }

        /**
         * @brief Initialize the startup file.
         * @param[in] filename Absolute path to the startup file.
         * @param[in] maxNumBytes Maximum number of bytes to read from the file.
         * @details Reads at most maxNumBytes from filename and stores them in a buffer. Use @ref GetBytes to obtain the binary data.
         */
        void Initialize(const char* filename, uint32_t maxNumBytes){
            buffer.clear();
            std::ifstream file(filename, std::ios::binary);
            if(file){
                buffer.resize(maxNumBytes);
                file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
                buffer.resize(file.gcount());
                file.close();
            }
        }

        /**
         * @brief Terminate the startup file.
         */
        void Terminate(void){
            buffer.clear();
        }

        /**
         * @brief Get bytes from the buffer.
         * @param[out] bytes Output array, where to store the binary data.
         * @param[out] length Output where to store the number of bytes that represent the actual binary data.
         * @param[in] maxNumBytes Maximum number of bytes that fit into the output array.
         */
        void GetBytes(uint8_t* bytes, uint32_t* length, uint32_t maxNumBytes){
            if(bytes && length){
                for(*length = 0; (*length < maxNumBytes) && (*length < static_cast<uint32_t>(buffer.size())); ++*length){
                    bytes[*length] = buffer[*length];
                }
            }
        }

    private:
        std::vector<uint8_t> buffer;
};


} // namespace etf

