//****************************************************************************//
// FTM Utilities - Helper functions for FTM Template                          //
// Société des Arts Technologiques (SAT)                                      //
// Input Devices and Music Interaction Laboratory (IDMIL)                     //
//****************************************************************************//

#ifndef FTM_UTILS_H
#define FTM_UTILS_H

#include <cstdint>
#include <cstdio>
#include <cstring>

//============================================================================//
// Time Formatting Utilities                                                  //
//============================================================================//

/**
 * @brief Format elapsed time in milliseconds to a human-readable string
 * @param elapsed_ms Time in milliseconds
 * @param buffer Output buffer for formatted string
 * @param buffer_size Size of the output buffer
 * @return Formatted string in format "MM:SS:mmm"
 */
inline void formatElapsedTime(unsigned long elapsed_ms, char* buffer, size_t buffer_size) {
    unsigned long minutes = elapsed_ms / 60000;
    unsigned long seconds = (elapsed_ms % 60000) / 1000;
    unsigned long milliseconds = elapsed_ms % 1000;
    snprintf(buffer, buffer_size, "%02lu:%02lu:%03lu", minutes, seconds, milliseconds);
}

//============================================================================//
// Moving Average Filter                                                      //
//============================================================================//

/**
 * @brief Simple moving average filter for smoothing FTM distance readings
 * 
 * Usage:
 *   MovingAverage filter(5);  // Window size of 5
 *   float smoothed = filter.addSample(raw_distance);
 * 
 * Window size trade-offs:
 *   - 3:  Low smoothing, high responsiveness
 *   - 5:  Medium smoothing, good balance (recommended)
 *   - 10: High smoothing, slower response
 */
class MovingAverage {
private:
    float* buffer;           // Circular buffer for samples
    uint16_t size;           // Window size
    uint16_t index;          // Current position in buffer
    uint16_t count;          // Number of samples collected (up to size)
    float sum;               // Running sum for efficient average calculation

public:
    /**
     * @brief Construct a new Moving Average filter
     * @param window_size Number of samples to average over
     */
    MovingAverage(uint16_t window_size = 5) 
        : size(window_size), index(0), count(0), sum(0.0f) {
        buffer = new float[size];
        for (uint16_t i = 0; i < size; i++) {
            buffer[i] = 0.0f;
        }
    }

    /**
     * @brief Destructor - free allocated buffer
     */
    ~MovingAverage() {
        delete[] buffer;
    }

    /**
     * @brief Add a new sample and return the updated average
     * @param value New sample value
     * @return Current moving average
     */
    float addSample(float value) {
        // Subtract the oldest value from sum
        sum -= buffer[index];
        // Store new value
        buffer[index] = value;
        // Add new value to sum
        sum += value;
        // Advance index (circular)
        index = (index + 1) % size;
        // Track count until buffer is full
        if (count < size) {
            count++;
        }
        // Return current average
        return getAverage();
    }

    /**
     * @brief Get the current moving average
     * @return Current average, or 0 if no samples
     */
    float getAverage() const {
        return (count > 0) ? (sum / count) : 0.0f;
    }

    /**
     * @brief Get the number of samples currently in the buffer
     * @return Sample count (0 to window_size)
     */
    uint16_t getSampleCount() const {
        return count;
    }

    /**
     * @brief Check if the buffer is full (enough samples for reliable average)
     * @return true if buffer is full
     */
    bool isReady() const {
        return count >= size;
    }

    /**
     * @brief Reset the filter, clearing all samples
     */
    void reset() {
        index = 0;
        count = 0;
        sum = 0.0f;
        for (uint16_t i = 0; i < size; i++) {
            buffer[i] = 0.0f;
        }
    }

    /**
     * @brief Get the last raw sample added
     * @return Last sample value
     */
    float getLastSample() const {
        if (count == 0) return 0.0f;
        // Index points to next position, so last sample is at index-1
        uint16_t lastIndex = (index == 0) ? (size - 1) : (index - 1);
        return buffer[lastIndex];
    }
};

#endif // FTM_UTILS_H
