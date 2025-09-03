#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <algorithm>
#include <mutex>
#include <stdexcept>

// 实现一致性哈希算法的类
class ConsistentHash {
public:
    // 指定每个节点的虚拟节点数和哈希函数
    ConsistentHash(size_t numReplicas = 1, std::function<size_t(const std::string&)> hashFunc = std::hash<std::string>())
        : numReplicas_(numReplicas), hashFunction_(hashFunc) {}

    // 添加节点，为物理节点生成多个虚拟节点
    void addNode(const std::string& node) {
        std::lock_guard<std::mutex> lock(mtx_); 
        for (size_t i = 0; i < numReplicas_; ++i) {
            std::string vnode = node + "_#" + std::to_string(i);
            size_t hash = hashFunction_(vnode);
            circle_[hash] = node;
            auto it = std::lower_bound(sortedHashes_.begin(), sortedHashes_.end(), hash);
            sortedHashes_.insert(it, hash);
        }
    }

    // 移除节点，
    void removeNode(const std::string& node) {
        std::lock_guard<std::mutex> lock(mtx_); 
        for (size_t i = 0; i < numReplicas_; ++i) {
            std::string vnode = node + "_#" + std::to_string(i);
            size_t hash = hashFunction_(vnode);
            circle_.erase(hash); 
            auto it = std::lower_bound(sortedHashes_.begin(), sortedHashes_.end(), hash);
            if (it != sortedHashes_.end() && *it == hash) {
                sortedHashes_.erase(it);
            }
        }
    }

    // 查找负责处理给定键的节点，返回物理节点名称
    std::string getNode(const std::string& key) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (circle_.empty()) {
            throw std::runtime_error("No nodes in consistent hash");
        }
        size_t hash = hashFunction_(key);
        auto it = std::upper_bound(sortedHashes_.begin(), sortedHashes_.end(), hash);
        if (it == sortedHashes_.end()) {
            it = sortedHashes_.begin();
        }
        return circle_[*it];
    }

private:
    size_t numReplicas_;                                        // 每个物理节点的虚拟节点数量
    std::function<size_t(const std::string&)> hashFunction_;    // 用户自定义或默认的哈希函数
    std::unordered_map<size_t, std::string> circle_;            // 哈希值到节点名称的映射
    std::vector<size_t> sortedHashes_;                          // 排序的哈希值列表，用于高效查找
    std::mutex mtx_;
};
