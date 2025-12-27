/**
 * @file ByteTree.hpp
 * @brief Declaration of the ByteTree class for hierarchical tree structure based on bytes.
 */

#ifndef NEBULITE_DATA_BYTE_TREE_HPP
#define NEBULITE_DATA_BYTE_TREE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/Branch.hpp"

//------------------------------------------
namespace Nebulite::Data {
/**
 * @class ByteTree
 * @brief A hierarchical tree structure based on bytes.
 *        Each node can have up to 256 children, indexed by byte values (0-255).
 *        In total, the tree can address 2^32 unique IDs using 4 layers.
 * @tparam StoreType The type of elements to be stored in the tree nodes.
 */
template<typename StoreType>
class ByteTree {
public:
    StoreType& at(uint32_t const& id) {
        /**
         * @todo Check if the one-after-another access is faster
         *       perhaps the locking works better that way?
         */
        /*
        auto L4out = rootLayer.at(id);
        auto L3out = L4out.at(id);
        auto L2out = L3out.at(id);
        auto L1out = L2out.at(id);
        return L1out;
        //*/

        //               L4     L3     L2     L1
        return rootLayer.at(id).at(id).at(id).at(id);
    }

    StoreType& operator[](uint32_t const& id) {
        return at(id);
    }

    /**
     * @brief Probabilistic cleanup of inactive entries in the entire tree.
     */
    void cleanup() {
        rootLayer.cleanup();
    }

private:
    class Layer1 : public Branch<StoreType, uint32_t, 8> {
    public:
        size_t idToIndex(uint32_t const& id) const override {
            // Use last 8 bits from left side for index
            return (size_t(id & 0xFF));
        }
    };

    class Layer2 : public Branch<Layer1, uint32_t, 8> {
    public:
        size_t idToIndex(uint32_t const& id) const override {
            // Use third 8 bits from left side for index
            return (size_t((id >> 8) & 0xFF));
        }
    };

    class Layer3 : public Branch<Layer2, uint32_t, 8> {
    public:
        size_t idToIndex(uint32_t const& id) const override {
            // Use second 8 bits from left side for index
            return (size_t((id >> 16) & 0xFF));
        }
    };

    class Layer4 : public Branch<Layer3, uint32_t, 8> {
    public:
        size_t idToIndex(uint32_t const& id) const override {
            // Use first 8 bits from left side for index
            return (size_t((id >> 24) & 0xFF));
        }
    };

    Layer4 rootLayer;

};
} // namespace Nebulite::Data
