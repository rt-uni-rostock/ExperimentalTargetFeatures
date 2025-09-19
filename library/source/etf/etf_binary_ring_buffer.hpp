/**
 * @file etf_binary_ring_buffer.hpp
 * @author Robert Damerius (damerius.mail@gmail.com)
 * @brief Experimental target feature for creating multi-file binary ring buffers where filesystem operations are handled by a separate worker thread.
 * @details Use the etf::BinaryRingBuffer class to create an asynchronous ring buffer that stores incoming samples in memory and writes them to disk in a separate worker thread.
 * @date 2025-09-18
 * @copyright Copyright (c) 2025 Robert Damerius
 */
#pragma once


/* Include standard libraries */
#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <filesystem>
#include <mutex>
#include <string>
#include <chrono>
#include <etf_detail.hpp>


/* Default namespace for experimental target features */
namespace etf {


/**
 * @brief Ring buffer for storing binary data samples in a non-blocking manner. All incoming samples are stored
 * in memory and written to disk by a worker thread. The ring buffer can be re-initialized at any time to start a new
 * ring buffer. All data files are stored in a specified folder, with each ring buffer instance creating a new subfolder
 * named according to the current UTC time.
 */
class BinaryRingBuffer {
    public:
        /**
         * @brief Construct a new binary ring buffer object.
         */
        BinaryRingBuffer(): sample_size(0), num_samples_per_file(0), num_files(0), ring_counter(0) {}

        /**
         * @brief Destroy the binary ring buffer object.
         * @details Stops the worker thread, closes the ring buffer, and clears all stored chunks.
         */
        ~BinaryRingBuffer(){ Terminate(); }

        /**
         * @brief Initialize the binary ring buffer.
         * @param[in] folder Path to the folder where data files will be stored. Each ring buffer instance will create a new subfolder named according to the current UTC time inside this folder.
         * @param[in] sampleSize Size of each sample in bytes.
         * @param[in] numSamplesPerFile Number of samples to store in each file.
         * @param[in] numFiles Number of files to use for the ring buffer.
         * @param[in] threadPriority Priority of the worker thread.
         */
        void Initialize(const char* folder, size_t sampleSize, size_t numSamplesPerFile, size_t numFiles, int threadPriority){
            data_folder = std::filesystem::path(folder);
            sample_size = sampleSize ? sampleSize : 1;
            num_samples_per_file = numSamplesPerFile ? numSamplesPerFile : 1;
            num_files = numFiles ? numFiles : 1;
            thread.Start(std::bind(&BinaryRingBuffer::CallbackNotify, this), threadPriority);
        }

        /**
         * @brief Terminate the binary ring buffer.
         * @details Stops the worker thread, closes the ring buffer, and clears all stored chunks.
         */
        void Terminate(void){
            // stop thread and write all remaining chunks
            thread.Stop();
            mtxChunks.lock();
            if(!chunks.empty()){ // prevents opening a new ring buffer if there are no chunks
                WriteChunks(chunks);
            }
            chunks.clear();
            mtxChunks.unlock();

            // close ring buffer
            ringBuffer.Close();

            // set parameters to default values (except ring counter)
            sample_size = 0;
            num_samples_per_file = 0;
            num_files = 0;
            data_folder.clear();
        }

        /**
         * @brief Add a new sample to the binary ring buffer.
         * @param[in] sampleData Pointer to the sample data to add. The size must be equal to the sample size specified during initialization.
         * @param[in] startNewRingBuffer Flag indicating whether to start a new ring buffer.
         * @return The number of cached samples waiting to be written to disk.
         */
        uint32_t AddSample(const void* sampleData, bool startNewRingBuffer){
            mtxChunks.lock();
            chunks.push_back({std::vector<uint8_t>((uint8_t*)sampleData, (uint8_t*)sampleData + sample_size), startNewRingBuffer});
            uint32_t numCachedChunks = static_cast<uint32_t>(chunks.size());
            mtxChunks.unlock();
            thread.Notify();
            return numCachedChunks;
        }

        /**
         * @brief Check if the ring buffer is currently open.
         * @return True if the ring buffer is open, false otherwise.
         * @note This is not thread-safe but it's fine for monitoring.
         */
        bool IsOpen(void) const { return ringBuffer.IsOpen(); }

    private:
        struct chunk_data {
            std::vector<uint8_t> data;            // Actual sample data of size @ref sample_size.
            bool start_new_ring_buffer;           // Flag indicating whether to start a new ring buffer before writing this sample.
        };

        size_t sample_size;                       // Size of each sample in the ring buffer.
        size_t num_samples_per_file;              // Number of samples per file in the ring buffer.
        size_t num_files;                         // Number of files in the ring buffer.
        size_t ring_counter;                      // Counter for the number of ring buffers created.
        std::filesystem::path data_folder;        // Data folder path where to store files for ring buffers.
        detail::MultiFileRingBuffer ringBuffer;   // Manages a multi-file ring buffer.
        detail::NotifyableThread thread;          // Worker thread that is notified when new samples are available.
        std::vector<chunk_data> chunks;           // Storage for chunks of sample data to be written to the ring buffer.
        std::mutex mtxChunks;                     // Mutex for protecting access to @ref chunks.

        /**
         * @brief Callback function executed inside the worker thread when notified.
         * @details Moves all available chunks to local storage of this thread, opens the ring buffer if not already open,
         * handles new ring buffer requests, and writes sample data to the ring buffer.
         */
        void CallbackNotify(void){
            // move all available chunks to local storage of this thread
            std::vector<chunk_data> localChunks;
            mtxChunks.lock();
            localChunks.swap(chunks);
            mtxChunks.unlock();

            // write chunk data to ring buffer
            WriteChunks(localChunks);
        }

        /**
         * @brief Generate a subdirectory name based on the current UTC time and ring counter.
         * @return The current subdirectory name of format "YYYYMMDD_HHMMSS_ringN".
         */
        std::string GenerateSubdirectoryName(void){
            auto timePoint = std::chrono::system_clock::now();
            std::time_t systemTime = std::chrono::system_clock::to_time_t(timePoint);
            std::tm* gmTime = std::gmtime(&systemTime);
            char cstr_utc[64];
            snprintf(cstr_utc, sizeof(cstr_utc), "%u%02u%02u_%02u%02u%02u", 1900 + gmTime->tm_year, 1 + gmTime->tm_mon, gmTime->tm_mday, gmTime->tm_hour, gmTime->tm_min, gmTime->tm_sec);
            return std::string(cstr_utc) + std::string("_") + std::string("ring") + std::to_string(ring_counter);
        }

        /**
         * @brief Write chunks of data to the ring buffer.
         * @param[in] dataChunks The chunks of data to write.
         * @details Opens the ring buffer if not already open, handles new ring buffer requests, and writes sample data to the ring buffer.
         */
        void WriteChunks(std::vector<chunk_data>& dataChunks){
            // open ring buffer if not already open
            if(!ringBuffer.IsOpen()){
                ring_counter++;
                std::filesystem::path directory = data_folder / GenerateSubdirectoryName();
                ringBuffer.Open(directory.string().c_str(), sample_size, num_samples_per_file, num_files);
            }

            // handle new ring buffer requests and write sample data
            for(auto&& chunk : dataChunks){
                if(chunk.start_new_ring_buffer){
                    ringBuffer.Close();
                    ring_counter++;
                    std::filesystem::path directory = data_folder / GenerateSubdirectoryName();
                    ringBuffer.Open(directory.string().c_str(), sample_size, num_samples_per_file, num_files);
                }
                ringBuffer.Write(&chunk.data[0]);
            }
        }
};


} // namespace etf

