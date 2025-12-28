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
    ByteTree() = default;

    std::shared_ptr<StoreType> at(uint32_t const& id);

    /**
     * @brief Probabilistic cleanup of inactive entries in the entire tree.
     */
    void cleanup();

    /**
     * @brief Applies the given operation to all active entries in the tree.
     */
    void apply() {
        rootLayer.apply();
    }

private:
    // Layers
    // Layer4 -> Layer3 -> Layer2 -> Layer1 -> StoreType
    // uin32_t split into 4 bytes for indexing each layer
    // Bits: [31-24] [23-16] [15-8] [7-0]
    //       Layer4  Layer3  Layer2  Layer1

    // TODO: For some reason we never reach Layer1 on apply calls...
    //       Say we have id 235, we Should emplace back:
    //       Layer4[0] -> Layer3[0] -> Layer2[0] -> Layer1[235] -> StoreType
    //       But Layer1 is never reached...

    // Lowest layer, directly storing StoreType
    class Layer1 : public Branch<StoreType, uint32_t, 8> {
    public:
        Layer1() = default;

        // Allow copy and move
        Layer1(Layer1 const&) = default;
        Layer1(Layer1&&) noexcept = default;
        Layer1& operator=(Layer1 const&) = default;
        Layer1& operator=(Layer1&&) noexcept = default;

        [[nodiscard]] size_t idToIndex(uint32_t const& id) const override;
    };

    // Second layer, storing Layer1
    class Layer2 : public Branch<Layer1, uint32_t, 8> {
    public:
        Layer2() = default;

        // Allow copy and move
        Layer2(Layer2 const&) = default;
        Layer2(Layer2&&) noexcept = default;
        Layer2& operator=(Layer2 const&) = default;
        Layer2& operator=(Layer2&&) noexcept = default;

        [[nodiscard]] size_t idToIndex(uint32_t const& id) const override;
    };

    // Third layer, storing Layer2
    class Layer3 : public Branch<Layer2, uint32_t, 8> {
    public:
        Layer3() = default;

        // Allow copy and move
        Layer3(Layer3 const&) = default;
        Layer3(Layer3&&) noexcept = default;
        Layer3& operator=(Layer3 const&) = default;
        Layer3& operator=(Layer3&&) noexcept = default;

        [[nodiscard]] size_t idToIndex(uint32_t const& id) const override;
    };

    // Top layer, storing Layer3
    class Layer4 : public Branch<Layer3, uint32_t, 8> {
    public:
        Layer4() = default;

        // Allow copy and move
        Layer4(Layer4 const&) = default;
        Layer4(Layer4&&) noexcept = default;
        Layer4& operator=(Layer4 const&) = default;
        Layer4& operator=(Layer4&&) noexcept = default;

        [[nodiscard]] size_t idToIndex(uint32_t const& id) const override;
    };

    Layer4 rootLayer;
};
} // namespace Nebulite::Data
#include "Data/ByteTree.tpp"
#endif // NEBULITE_DATA_BYTE_TREE_HPP
