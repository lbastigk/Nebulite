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
    StoreType& at(uint32_t const& id);

    StoreType& operator[](uint32_t const& id);

    /**
     * @brief Probabilistic cleanup of inactive entries in the entire tree.
     */
    void cleanup();

private:
    class Layer1 : public Branch<StoreType, uint32_t, 8> {
    public:
        [[nodiscard]] size_t idToIndex(uint32_t const& id) const override;
    };

    class Layer2 : public Branch<Layer1, uint32_t, 8> {
    public:
        [[nodiscard]] size_t idToIndex(uint32_t const& id) const override;
    };

    class Layer3 : public Branch<Layer2, uint32_t, 8> {
    public:
        [[nodiscard]] size_t idToIndex(uint32_t const& id) const override;
    };

    class Layer4 : public Branch<Layer3, uint32_t, 8> {
    public:
        [[nodiscard]] size_t idToIndex(uint32_t const& id) const override;
    };

    Layer4 rootLayer;
};
} // namespace Nebulite::Data
#include "Data/ByteTree.tpp"
#endif // NEBULITE_DATA_BYTE_TREE_HPP
