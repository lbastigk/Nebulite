#include <Data/ByteTree.hpp>

namespace Nebulite::Data {

// ByteTree member implementations
template<typename StoreType>
std::shared_ptr<StoreType> ByteTree<StoreType>::at(uint32_t const& id) {
    auto L3 = rootLayer.at(id);
    auto L2 = L3->at(id);
    auto L1 = L2->at(id);
    return L1->at(id);
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
