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

//============================================================================//
// FTM Calibration                                                            //
//============================================================================//

/**
 * @brief FTM Calibration utility for startup calibration
 * 
 * Usage:
 *   FTMCalibration calibrator(100.0f, 100);  // 100cm known distance, 100 samples
 *   
 *   // During calibration (in loop):
 *   if (calibrator.isCalibrating()) {
 *       calibrator.addSample(raw_distance);
 *   }
 *   
 *   // After calibration:
 *   float calibrated = calibrator.apply(raw_distance);
 */
class FTMCalibration {
private:
    float known_distance_cm;      // The actual distance during calibration
    uint32_t target_samples;      // Number of samples to collect
    uint32_t samples_collected;   // Current sample count
    float accumulated_sum;        // Sum of all samples
    float offset_cm;              // Calculated offset (measured - actual)
    bool calibration_complete;    // Flag indicating calibration is done

public:
    /**
     * @brief Construct a new FTM Calibration object
     * @param known_dist_cm The actual/true distance during calibration (default 100cm)
     * @param num_samples Number of samples to collect (default 100)
     */
    FTMCalibration(float known_dist_cm = 100.0f, uint32_t num_samples = 100)
        : known_distance_cm(known_dist_cm)
        , target_samples(num_samples)
        , samples_collected(0)
        , accumulated_sum(0.0f)
        , offset_cm(0.0f)
        , calibration_complete(false) {
    }

    /**
     * @brief Add a calibration sample
     * @param measured_distance_cm The raw FTM distance measurement
     * @return true if calibration just completed, false otherwise
     */
    bool addSample(float measured_distance_cm) {
        if (calibration_complete) {
            return false;
        }

        accumulated_sum += measured_distance_cm;
        samples_collected++;

        // Check if we have enough samples
        if (samples_collected >= target_samples) {
            float measured_avg = accumulated_sum / samples_collected;
            offset_cm = measured_avg - known_distance_cm;
            calibration_complete = true;
            return true;  // Calibration just finished
        }

        return false;
    }

    /**
     * @brief Check if calibration is still in progress
     * @return true if still collecting samples
     */
    bool isCalibrating() const {
        return !calibration_complete;
    }

    /**
     * @brief Check if calibration is complete
     * @return true if calibration finished
     */
    bool isCalibrated() const {
        return calibration_complete;
    }

    /**
     * @brief Get the current progress
     * @return Number of samples collected so far
     */
    uint32_t getSamplesCollected() const {
        return samples_collected;
    }

    /**
     * @brief Get the target number of samples
     * @return Total samples needed for calibration
     */
    uint32_t getTargetSamples() const {
        return target_samples;
    }

    /**
     * @brief Get progress as percentage
     * @return Progress 0-100
     */
    uint8_t getProgressPercent() const {
        return (uint8_t)((samples_collected * 100) / target_samples);
    }

    /**
     * @brief Get the calculated offset
     * @return Offset in cm (positive = measured too high, negative = measured too low)
     */
    float getOffset() const {
        return offset_cm;
    }

    /**
     * @brief Get the known/reference distance used for calibration
     * @return Known distance in cm
     */
    float getKnownDistance() const {
        return known_distance_cm;
    }

    /**
     * @brief Get the average measured distance during calibration
     * @return Average measured distance in cm
     */
    float getMeasuredAverage() const {
        if (samples_collected == 0) return 0.0f;
        return accumulated_sum / samples_collected;
    }

    /**
     * @brief Apply calibration offset to a raw measurement
     * @param raw_distance_cm The raw FTM measurement
     * @return Calibrated distance (raw - offset), clamped to >= 0
     */
    float apply(float raw_distance_cm) const {
        float calibrated = raw_distance_cm - offset_cm;
        return (calibrated < 0.0f) ? 0.0f : calibrated;
    }

    /**
     * @brief Manually set the offset (useful for restoring saved calibration)
     * @param offset New offset value in cm
     */
    void setOffset(float offset) {
        offset_cm = offset;
        calibration_complete = true;
    }

    /**
     * @brief Reset calibration to start fresh
     */
    void reset() {
        samples_collected = 0;
        accumulated_sum = 0.0f;
        offset_cm = 0.0f;
        calibration_complete = false;
    }
};

#endif // FTM_UTILS_H
