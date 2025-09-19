#pragma once


/* Include standard libraries */
#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <filesystem>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>


/* Default namespace for experimental target features */
namespace etf {


/* Internal namespace for implementation details */
namespace detail {


/**
 * @brief A ring buffer that writes samples to multiple files in a circular manner.
 * @details When the end of a file is reached, it swaps to the next file and wraps around to the beginning to overwrite old data.
 */
class MultiFileRingBuffer {
    public:
        /**
         * @brief Construct a new multi-file ring buffer object.
         */
        MultiFileRingBuffer(): sample_size(0), file_size(0), current_file(0), index(0) {}

        /**
         * @brief Destroy the multi-file ring buffer object.
         * @details Closes all open files.
         */
        ~MultiFileRingBuffer(){ Close(); }

        /**
         * @brief Open the multi-file ring buffer.
         * @param[in] folder The absolute path to a folder where to store the ring buffer files.
         * @param[in] sampleSize The size of each sample.
         * @param[in] numSamplesPerFile The number of samples per file.
         * @param[in] numFiles The number of files to store.
         * @return True if all files were opened successfully, false otherwise.
         */
        bool Open(const char* folder, size_t sampleSize, size_t numSamplesPerFile, size_t numFiles){
            if(!files.empty()){
                return false; // already open
            }
            sample_size = sampleSize ? sampleSize : 1;
            file_size = numSamplesPerFile ? (numSamplesPerFile * sample_size) : sample_size;
            current_file = 0;
            index = 0;
            numFiles = numFiles ? numFiles : 1;

            // create directory
            directory = std::filesystem::path(folder);
            if(!MakeDirectory(directory)){
                Close();
                return false;
            }

            // create files
            for(size_t k = 0; k < numFiles; ++k){
                std::filesystem::path filename = directory / ("buffer" + std::to_string(k) + ".dat");
                FILE* fp = fopen(filename.string().c_str(), "w");
                if(!fp){
                    Close();
                    return false;
                }
                files.push_back(fp);
            }
            return true;
        }

        /**
         * @brief Close the multi-file ring buffer.
         */
        void Close(void){
            if(!files.empty()){
                WriteJSONComplete();
            }
            sample_size = 0;
            file_size = 0;
            current_file = 0;
            index = 0;
            for(auto&& fp : files){
                fclose(fp);
            }
            files.clear();
            directory.clear();
        }

        /**
         * @brief Write a sample to the multi-file ring buffer.
         * @param[in] sampleData The sample data to write.
         */
        void Write(const void* sampleData){
            if(!files.empty()){
                fwrite(sampleData, 1, sample_size, files[current_file]);
                fflush(files[current_file]); // ensure data is written to disk
                index = (index + sample_size) % file_size;
                if(0 == index){
                    fseek(files[current_file], 0, SEEK_SET);
                    current_file = (current_file + 1) % files.size();
                }
            }
        }

        /**
         * @brief Check if the multi-file ring buffer is open.
         * @return True if open, false otherwise.
         */
        bool IsOpen(void) const { return !files.empty(); }

    private:
        size_t sample_size;                // Size of each sample.
        size_t file_size;                  // Total size of a file.
        size_t current_file;               // Index to the current file in use.
        size_t index;                      // Current write index of a file.
        std::vector<FILE*> files;          // File pointers of all open files.
        std::filesystem::path directory;   // Directory where files are stored.

        /**
         * @brief Make a directory if it does not exist.
         * @param[in] directory Path to the directory to be created.
         * @return True if success, false otherwise.
         */
        bool MakeDirectory(std::filesystem::path directory){
            try{
                std::filesystem::create_directories(directory);
            }
            catch(...){
                return false;
            }
            return true;
        }

        /**
         * @brief Write a JSON file indicating that the ring buffer is complete.
         * @details Creates a "complete.json" file in the directory of the ring buffer files.
         */
        void WriteJSONComplete(void){
            std::filesystem::path jsonFile = directory / "complete.json";
            FILE* fp = fopen(jsonFile.string().c_str(), "w");
            if(fp){
                fprintf(fp, "{\n");
                fprintf(fp, "    \"bytes_per_sample\": %zu,\n", sample_size);
                fprintf(fp, "    \"bytes_per_file\": %zu,\n", file_size);
                fprintf(fp, "    \"files_per_ringbuffer\": %zu,\n", files.size());
                fprintf(fp, "    \"writing_point\": {\n");
                fprintf(fp, "        \"file_index\": %zu,\n", current_file);
                fprintf(fp, "        \"byte_offset\": %zu\n", index);
                fprintf(fp, "    }\n");
                fprintf(fp, "}\n");
                fclose(fp);
            }
        }
};


/**
 * @brief Class representing a thread that can be notified to perform work.
 * @details This class encapsulates a thread that waits for notifications to execute a callback function.
 * It provides methods to start, stop, and notify the thread. The thread can be assigned a specific priority.
 */
class NotifyableThread {
    public:
        /**
         * @brief Construct a new notifyable thread.
         */
        NotifyableThread() : notified(false), terminate(false) {}

        /**
         * @brief Destroy the notifyable thread.
         * @details Stops the thread if it is still running.
         */
        ~NotifyableThread(){
            Stop();
        }

        /**
         * @brief Start or restart the notifyable thread.
         * @param[in] callback Callback function to be called inside the thread when it is notified.
         * @param[in] priority Thread priority to be set using pthread_setschedparam (SCHED_FIFO).
         */
        void Start(std::function<void(void)> callback, int priority){
            Stop();
            thread = std::thread(&NotifyableThread::WorkerThread, this, callback);
            struct sched_param param;
            param.sched_priority = priority;
            if(0 != pthread_setschedparam(thread.native_handle(), SCHED_FIFO, &param)){
                // ignore
            }
        }

        /**
         * @brief Stop the notifyable thread.
         */
        void Stop(void){
            terminate = true;
            Notify();
            if(thread.joinable()){
                thread.join();
            }
            terminate = false;
        }

        /**
         * @brief Notify the worker thread.
         */
        void Notify(void){
            std::unique_lock<std::mutex> lock(mtxNotify);
            notified = true;
            cvNotify.notify_one();
        }

    private:
        bool notified;                     // Flag for thread notification.
        std::atomic<bool> terminate;       // Flag for thread termination.
        std::thread thread;                // Internal worker thread.
        std::mutex mtxNotify;              // Mutex for thread notification.
        std::condition_variable cvNotify;  // Condition variable for thread notification.

        /**
         * @brief Worker thread function.
         */
        void WorkerThread(std::function<void(void)> callback){
            while(!terminate){
                {
                    std::unique_lock<std::mutex> lock(mtxNotify);
                    cvNotify.wait(lock, [this](){ return (notified || terminate); });
                    notified = false;
                }
                if(terminate){
                    break;
                }
                callback();
            }
        }
};


} // namespace detail


} // namespace etf

