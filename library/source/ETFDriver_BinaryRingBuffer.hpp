#pragma once


#include <cstdint>


/**
 * @brief Initialize the binary ring buffer.
 * @param[in] workVector The simulink work vector storing the pointer to the actual driver object.
 * @param[in] folderName The name of the folder to store the data files.
 * @param[in] strlenFolderName The length of the folder name.
 * @param[in] sampleSize The size of each sample in bytes.
 * @param[in] numSamplesPerFile The number of samples to store in each file.
 * @param[in] numFiles The number of files to create.
 * @param[in] threadPriority The priority of the internal worker thread.
 */
void ETFDriver_BinaryRingBufferInitialize(void** workVector, uint8_t* folderName, uint32_t strlenFolderName, uint32_t sampleSize, uint32_t numSamplesPerFile, uint32_t numFiles, int32_t threadPriority);

/**
 * @brief Terminate the binary ring buffer.
 * @param[in] workVector The simulink work vector storing the pointer to the actual driver object.
 * @details Stops the internal worker thread and closes the ring buffer.
 */
void ETFDriver_BinaryRingBufferTerminate(void* workVector);

/**
 * @brief Add a new sample to the binary ring buffer.
 * @param[in] workVector The simulink work vector storing the pointer to the actual driver object.
 * @param[out] isOpen Pointer to store the open status of the ring buffer.
 * @param[out] numCachedSamples Pointer to store the number of cached samples waiting to be written to disk.
 * @param[in] sampleData Pointer to the sample data to add. The size must be equal to the sample size specified during initialization.
 * @param[in] startNewRingBuffer Flag indicating whether to start a new ring buffer.
 */
void ETFDriver_BinaryRingBufferStep(void* workVector, uint8_t* isOpen, uint32_t* numCachedSamples, uint8_t* sampleData, uint8_t startNewRingBuffer);

