#pragma once

#include <graphbench/csv_graph_reader.hpp>
#include <graphbench/benchmark_utils.hpp>
#include <graphbench/node_id_mapping.hpp>
#include <rocksdb/graph.h>
#include <string>
#include <map>
#include <chrono>
#include <iostream>

using namespace ROCKSDB_NAMESPACE;

namespace graphbench {

/**
 * Graph loader for Aster database.
 * Loads graph data from CSV files into Aster's RocksGraph.
 * Template-based to avoid circular dependencies.
 * Supports both structural and property graphs.
 */
template<typename ExecutorType>
class AsterGraphLoader {
public:
    /**
     * Load graph from dataset directory.
     * Reads nodes.csv and edges.csv and populates the Aster database.
     * Properties in CSV columns are automatically loaded if present.
     */
    static std::map<std::string, std::any> loadGraph(ExecutorType* executor,
                                                      const std::string& datasetPath,
                                                      bool loadProperties = true) {
        auto startTime = std::chrono::high_resolution_clock::now();

        if (executor->progressCallback_) {
            std::string msg = "Loading graph from " + datasetPath;
            if (loadProperties) {
                msg += " (with properties)";
            }
            executor->progressCallback_->sendLogMessage(msg, "INFO");
        }

        // Pre-allocate node ID mapping
        size_t nodeCount = NodeIdMapping<node_id_t>::count_nodes_from_csv(datasetPath);
        executor->nodeIdMapping_ = std::make_unique<NodeIdMapping<node_id_t>>(nodeCount, 0, executor->progressCallback_.get());

        int64_t loadedNodeCount = 0;
        int64_t edgeCount = 0;
        int64_t vertexPropCount = 0;
        int64_t edgePropCount = 0;

        // Load nodes and edges with properties
        CsvGraphReader::read(
            datasetPath,
            // Node callback
            [&](int64_t nodeId, const std::map<std::string, std::string>& properties) {
                node_id_t systemId = static_cast<node_id_t>(nodeId);
                executor->nodeIdMapping_->set(nodeId, systemId);

                Status s = executor->graph_->AddVertex(systemId);
                if (s.ok()) {
                    loadedNodeCount++;
                    if (systemId >= executor->nextVertexId_) {
                        executor->nextVertexId_ = systemId + 1;
                    }

                    // Load vertex properties if enabled and properties exist
                    if (loadProperties && !properties.empty()) {
                        for (const auto& [key, value] : properties) {
                            Property prop{key, value};
                            Status propStatus = executor->graph_->AddVertexProperty(systemId, prop);
                            if (propStatus.ok()) {
                                vertexPropCount++;
                            }
                        }
                    }
                } else {
                    std::cerr << "Failed to add vertex " << systemId << ": " << s.ToString() << std::endl;
                }
            },
            // Edge callback
            [&](int64_t srcId, int64_t dstId, const std::map<std::string, std::string>& properties) {
                node_id_t srcSystemId = executor->nodeIdMapping_->get(srcId, "src");
                node_id_t dstSystemId = executor->nodeIdMapping_->get(dstId, "dst");

                Status s = executor->graph_->AddEdge(srcSystemId, dstSystemId);
                if (s.ok()) {
                    edgeCount++;

                    // Load edge properties if enabled and properties exist
                    if (loadProperties && !properties.empty()) {
                        for (const auto& [key, value] : properties) {
                            Property prop{key, value};
                            Status propStatus = executor->graph_->AddEdgeProperty(srcSystemId, dstSystemId, prop);
                            if (propStatus.ok()) {
                                edgePropCount++;
                            }
                        }
                    }
                } else {
                    std::cerr << "Failed to add edge " << srcSystemId << " -> " << dstSystemId
                             << ": " << s.ToString() << std::endl;
                }
            }
        );

        auto endTime = std::chrono::high_resolution_clock::now();
        double durationSeconds = std::chrono::duration<double>(endTime - startTime).count();

        if (executor->progressCallback_) {
            std::string msg = "Graph loaded: " + std::to_string(loadedNodeCount) + " nodes, " +
                            std::to_string(edgeCount) + " edges";
            if (vertexPropCount > 0 || edgePropCount > 0) {
                msg += " (" + std::to_string(vertexPropCount) + " vertex props, " +
                       std::to_string(edgePropCount) + " edge props)";
            }
            msg += " in " + std::to_string(durationSeconds) + " seconds";
            executor->progressCallback_->sendLogMessage(msg, "INFO");
        }

        // Return metadata
        std::map<std::string, std::any> metadata;
        metadata["nodes"] = static_cast<int>(loadedNodeCount);
        metadata["edges"] = static_cast<int>(edgeCount);
        metadata["duration_seconds"] = durationSeconds;
        metadata["vertex_property_count"] = static_cast<int>(vertexPropCount);
        metadata["edge_property_count"] = static_cast<int>(edgePropCount);

        return metadata;
    }
};

} // namespace graphbench
