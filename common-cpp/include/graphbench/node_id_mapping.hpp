#pragma once

#include <vector>
#include <stdexcept>
#include <string>
#include <fstream>
#include <limits>
#include <memory>

namespace graphbench {

// Forward declaration
class ProgressCallback;

/**
 * Manages mapping from CSV node IDs (0-indexed sequential) to database-internal node IDs.
 * Uses std::vector for O(1) access with minimal memory overhead.
 */
template<typename T>
class NodeIdMapping {
public:
    /**
     * Create a NodeIdMapping with pre-allocated capacity.
     *
     * @param node_count Number of nodes to allocate space for
     * @param unset_value Sentinel value to indicate unset entries
     * @param progress_callback Callback for error reporting (optional)
     */
    NodeIdMapping(size_t node_count, const T& unset_value, ProgressCallback* progress_callback = nullptr)
        : unset_value_(unset_value), node_ids_(node_count, unset_value), progress_callback_(progress_callback) {}

    /**
     * Set the internal ID for a given node.
     *
     * @param node_id CSV node ID (0-indexed)
     * @param internal_id Database-internal node ID
     */
    void set(int64_t node_id, const T& internal_id) {
        size_t index = static_cast<size_t>(node_id);
        if (index >= node_ids_.size()) {
            std::string errorMsg = "Node ID " + std::to_string(node_id) +
                                   " exceeds expected count " + std::to_string(node_ids_.size());
            if (progress_callback_) {
                progress_callback_->sendErrorMessage(errorMsg, "DATA_ERROR");
            }
            throw std::runtime_error(errorMsg);
        }
        node_ids_[index] = internal_id;
    }

    /**
     * Get the internal ID for a given node, with validation.
     *
     * @param node_id CSV node ID (0-indexed)
     * @param context Context string for error messages (e.g., "src", "dst")
     * @return Database-internal node ID
     * @throws std::runtime_error if node ID is out of range or not yet loaded
     */
    T get(int64_t node_id, const std::string& context) const {
        size_t index = static_cast<size_t>(node_id);

        if (index >= node_ids_.size()) {
            std::string errorMsg = "Invalid " + context + " node ID " + std::to_string(node_id) +
                                   ": out of range (max=" + std::to_string(node_ids_.size() - 1) + ")";
            if (progress_callback_) {
                progress_callback_->sendErrorMessage(errorMsg, "DATA_ERROR");
            }
            throw std::runtime_error(errorMsg);
        }

        const T& internal_id = node_ids_[index];

        if (internal_id == unset_value_) {
            std::string errorMsg = "Invalid " + context + " node ID " + std::to_string(node_id) +
                                   ": node not loaded yet";
            if (progress_callback_) {
                progress_callback_->sendErrorMessage(errorMsg, "DATA_ERROR");
            }
            throw std::runtime_error(errorMsg);
        }

        return internal_id;
    }

    /**
     * Get internal ID without validation or error reporting (for parameter parsing).
     * Returns a default-constructed T if the node ID is invalid or not loaded.
     * This method should be used to check node existence without throwing exceptions.
     *
     * @param node_id CSV node ID (0-indexed)
     * @param found Output parameter set to true if valid, false otherwise
     * @return Database-internal node ID, or unset_value if invalid/not loaded
     */
    T get_or_default(int64_t node_id, bool* found = nullptr) const {
        size_t index = static_cast<size_t>(node_id);
        if (node_id < 0 || index >= node_ids_.size()) {
            if (found) *found = false;
            return unset_value_;
        }
        const T& internal_id = node_ids_[index];
        if (internal_id == unset_value_) {
            if (found) *found = false;
            return unset_value_;
        }
        if (found) *found = true;
        return internal_id;
    }

    /**
     * Get internal ID without validation (for performance-critical paths).
     */
    const T& get_unsafe(size_t index) const {
        return node_ids_[index];
    }

    /**
     * Get the number of nodes.
     */
    size_t size() const {
        return node_ids_.size();
    }

    /**
     * Count nodes from nodes.csv file.
     *
     * @param dataset_path Path to dataset directory containing nodes.csv
     * @return Number of nodes (excluding header)
     */
    static size_t count_nodes_from_csv(const std::string& dataset_path) {
        std::string nodes_file = dataset_path + "/nodes.csv";
        std::ifstream file(nodes_file);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open nodes.csv: " + nodes_file);
        }

        std::string line;
        std::getline(file, line); // Skip header

        size_t count = 0;
        while (std::getline(file, line)) {
            count++;
        }

        return count;
    }

private:
    T unset_value_;
    std::vector<T> node_ids_;
    ProgressCallback* progress_callback_;
};

// Specialization for pointer types (use nullptr as unset value)
template<typename T>
class NodeIdMapping<T*> {
public:
    NodeIdMapping(size_t node_count, T* unset_value = nullptr, ProgressCallback* progress_callback = nullptr)
        : node_ids_(node_count, unset_value), progress_callback_(progress_callback) {}

    void set(int64_t node_id, T* internal_id) {
        size_t index = static_cast<size_t>(node_id);
        if (index >= node_ids_.size()) {
            std::string errorMsg = "Node ID " + std::to_string(node_id) +
                                   " exceeds expected count " + std::to_string(node_ids_.size());
            if (progress_callback_) {
                progress_callback_->sendErrorMessage(errorMsg, "DATA_ERROR");
            }
            throw std::runtime_error(errorMsg);
        }
        node_ids_[index] = internal_id;
    }

    T* get(int64_t node_id, const std::string& context) const {
        size_t index = static_cast<size_t>(node_id);

        if (index >= node_ids_.size()) {
            std::string errorMsg = "Invalid " + context + " node ID " + std::to_string(node_id) +
                                   ": out of range (max=" + std::to_string(node_ids_.size() - 1) + ")";
            if (progress_callback_) {
                progress_callback_->sendErrorMessage(errorMsg, "DATA_ERROR");
            }
            throw std::runtime_error(errorMsg);
        }

        T* internal_id = node_ids_[index];

        if (internal_id == nullptr) {
            std::string errorMsg = "Invalid " + context + " node ID " + std::to_string(node_id) +
                                   ": node not loaded yet";
            if (progress_callback_) {
                progress_callback_->sendErrorMessage(errorMsg, "DATA_ERROR");
            }
            throw std::runtime_error(errorMsg);
        }

        return internal_id;
    }

    /**
     * Get internal ID without validation or error reporting (for parameter parsing).
     * Returns nullptr if the node ID is invalid or not loaded.
     *
     * @param node_id CSV node ID (0-indexed)
     * @param found Output parameter set to true if valid, false otherwise
     * @return Database-internal node ID, or nullptr if invalid/not loaded
     */
    T* get_or_default(int64_t node_id, bool* found = nullptr) const {
        size_t index = static_cast<size_t>(node_id);
        if (node_id < 0 || index >= node_ids_.size()) {
            if (found) *found = false;
            return nullptr;
        }
        T* internal_id = node_ids_[index];
        if (internal_id == nullptr) {
            if (found) *found = false;
            return nullptr;
        }
        if (found) *found = true;
        return internal_id;
    }

    T* get_unsafe(size_t index) const {
        return node_ids_[index];
    }

    size_t size() const {
        return node_ids_.size();
    }

    static size_t count_nodes_from_csv(const std::string& dataset_path) {
        std::string nodes_file = dataset_path + "/nodes.csv";
        std::ifstream file(nodes_file);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open nodes.csv: " + nodes_file);
        }

        std::string line;
        std::getline(file, line); // Skip header

        size_t count = 0;
        while (std::getline(file, line)) {
            count++;
        }

        return count;
    }

private:
    std::vector<T*> node_ids_;
    ProgressCallback* progress_callback_;
};

} // namespace graphbench