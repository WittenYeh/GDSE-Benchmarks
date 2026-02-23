package com.graphbench.api;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.List;

/**
 * Manages mapping from CSV node IDs (0-indexed sequential) to database-internal node IDs.
 * Uses ArrayList for O(1) access with minimal memory overhead.
 */
public class NodeIdMapping<T> {
    private final List<T> nodeIdsList;
    private final T unsetValue;
    private final ProgressCallback progressCallback;

    /**
     * Create a NodeIdMapping with pre-allocated capacity.
     *
     * @param nodeCount Number of nodes to allocate space for
     * @param unsetValue Sentinel value to indicate unset entries (e.g., Long.MAX_VALUE, null)
     * @param progressCallback Callback for error reporting
     */
    public NodeIdMapping(int nodeCount, T unsetValue, ProgressCallback progressCallback) {
        this.unsetValue = unsetValue;
        this.progressCallback = progressCallback;
        this.nodeIdsList = new ArrayList<>(nodeCount);
        for (int i = 0; i < nodeCount; i++) {
            nodeIdsList.add(unsetValue);
        }
    }

    /**
     * Set the internal ID for a given node.
     *
     * @param nodeId CSV node ID (0-indexed)
     * @param internalId Database-internal node ID
     */
    public void set(long nodeId, T internalId) {
        int index = (int)nodeId;
        if (index >= nodeIdsList.size()) {
            String errorMsg = String.format("Node ID %d exceeds expected count %d", nodeId, nodeIdsList.size());
            progressCallback.sendErrorMessage(errorMsg, "DATA_ERROR");
            throw new RuntimeException(errorMsg);
        }
        nodeIdsList.set(index, internalId);
    }

    /**
     * Get the internal ID for a given node, with validation.
     *
     * @param nodeId CSV node ID (0-indexed)
     * @param context Context string for error messages (e.g., "src", "dst")
     * @return Database-internal node ID
     * @throws RuntimeException if node ID is out of range or not yet loaded
     */
    public T get(long nodeId, String context) {
        int index = (int)nodeId;

        if (index >= nodeIdsList.size()) {
            String errorMsg = String.format("Invalid %s node ID %d: out of range (max=%d)",
                    context, nodeId, nodeIdsList.size() - 1);
            progressCallback.sendErrorMessage(errorMsg, "DATA_ERROR");
            throw new RuntimeException(errorMsg);
        }

        T internalId = nodeIdsList.get(index);

        if (internalId == unsetValue || (unsetValue == null && internalId == null)) {
            String errorMsg = String.format("Invalid %s node ID %d: node not loaded yet", context, nodeId);
            progressCallback.sendErrorMessage(errorMsg, "DATA_ERROR");
            throw new RuntimeException(errorMsg);
        }

        return internalId;
    }

    /**
     * Get internal ID without validation or error reporting (for parameter parsing).
     * Returns null if the node ID is invalid or not loaded.
     * This method should be used by ParameterParser to check node existence without throwing exceptions.
     *
     * @param nodeId CSV node ID (0-indexed)
     * @return Database-internal node ID, or null if invalid/not loaded
     */
    public T getOrNull(long nodeId) {
        int index = (int)nodeId;
        if (index < 0 || index >= nodeIdsList.size()) {
            return null;
        }
        T internalId = nodeIdsList.get(index);
        if (internalId == unsetValue || (unsetValue == null && internalId == null)) {
            return null;
        }
        return internalId;
    }

    /**
     * Get internal ID without validation (for performance-critical paths).
     */
    public T getUnsafe(int index) {
        return nodeIdsList.get(index);
    }

    /**
     * Get the number of nodes.
     */
    public int size() {
        return nodeIdsList.size();
    }

    /**
     * Count nodes from nodes.csv file.
     *
     * @param datasetPath Path to dataset directory containing nodes.csv
     * @return Number of nodes (excluding header)
     */
    public static int countNodesFromCSV(String datasetPath) throws Exception {
        File nodesFile = new File(datasetPath, "nodes.csv");
        try (BufferedReader reader = new BufferedReader(new FileReader(nodesFile))) {
            reader.readLine(); // Skip header
            int count = 0;
            while (reader.readLine() != null) {
                count++;
            }
            return count;
        }
    }
}