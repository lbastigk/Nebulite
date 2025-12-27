#include <Data/ByteTree.hpp>

namespace Nebulite::Data {

// ByteTree member implementations
template<typename StoreType>
StoreType& ByteTree<StoreType>::at(uint32_t const& id) {
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

template<typename StoreType>
StoreType& ByteTree<StoreType>::operator[](uint32_t const& id) {
    return at(id);
}

template<typename StoreType>
void ByteTree<StoreType>::cleanup() {
    rootLayer.cleanup();
}

// Layer idToIndex implementations
template<typename StoreType>
size_t ByteTree<StoreType>::Layer1::idToIndex(uint32_t const& id) const {
    return static_cast<size_t>(id & 0xFF);
}

template<typename StoreType>
size_t ByteTree<StoreType>::Layer2::idToIndex(uint32_t const& id) const {
    return static_cast<size_t>((id >> 8) & 0xFF);
}

template<typename StoreType>
size_t ByteTree<StoreType>::Layer3::idToIndex(uint32_t const& id) const {
    return static_cast<size_t>((id >> 16) & 0xFF);
}

template<typename StoreType>
size_t ByteTree<StoreType>::Layer4::idToIndex(uint32_t const& id) const {
    return static_cast<size_t>((id >> 24) & 0xFF);
}

} // namespace Nebulite::Data
