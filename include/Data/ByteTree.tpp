#include <Data/ByteTree.hpp>

namespace Nebulite::Data {

// ByteTree member implementations
template<typename StoreType>
std::shared_ptr<StoreType> ByteTree<StoreType>::at(uint32_t const& id) {
    return rootLayer.at(id)->at(id)->at(id)->at(id);
}

template<typename StoreType>
void ByteTree<StoreType>::cleanup() {
    rootLayer.cleanup();
}

// Layer-dependent idToIndex implementations

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
