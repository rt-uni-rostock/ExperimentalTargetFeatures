#pragma once


#include <cstdint>


/**
 * @brief Read the startup file.
 * @param[in] workVector The simulink work vector storing the pointer to the actual driver object.
 * @param[in] filename Absolute path to the startup file.
 * @param[in] strlenFilename The length of the filename.
 * @param[in] maxNumBytes Maximum number of bytes to read from the file.
 */
void ETFDriver_StartupFileInitialize(void** workVector, uint8_t* filename, uint32_t strlenFilename, uint32_t maxNumBytes);

/**
 * @brief Terminate the startup file.
 * @param[in] workVector The simulink work vector storing the pointer to the actual driver object.
 */
void ETFDriver_StartupFileTerminate(void* workVector);

/**
 * @brief Get binary data from the startup file.
 * @param[in] workVector The simulink work vector storing the pointer to the actual driver object.
 * @param[out] bytes Output array, where to store the binary data.
 * @param[out] length Output where to store the number of bytes that represent the actual binary data.
 * @param[in] maxNumBytes Maximum number of bytes that fit into the output array.
 */
void ETFDriver_StartupFileStep(void* workVector, uint8_t* bytes, uint32_t* length, uint32_t maxNumBytes);

