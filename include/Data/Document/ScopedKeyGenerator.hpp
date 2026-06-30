#ifndef DATA_SCOPEDKEYGENERATOR_HPP
#define DATA_SCOPEDKEYGENERATOR_HPP

//------------------------------------------
// Includes

// Standard library
#include <stdexcept>
#include <string_view>

// Nebulite
#include "Data/Document/ScopedKey.hpp"

//------------------------------------------
namespace Nebulite::Data {

// Operation type discriminator
enum class OpType { Member, Index };

// Union-like struct to hold either operation
struct OpData {
    OpType type;
    std::string_view memberName;
    std::size_t indexValue;

    // constexpr default constructor so std::array<OpData, N> can be value-initialized
    constexpr OpData()
        : type(OpType::Member), indexValue(0) {}

    explicit constexpr OpData(std::string_view name) // NOLINT
        : type(OpType::Member), memberName(name), indexValue(0) {}

    explicit constexpr OpData(std::size_t index) // NOLINT
        : type(OpType::Index), indexValue(index) {}
};

/**
 * @brief Helper class to generate ScopedKeyViews using addMember and addIndex at compile time
 * @details Generates a function that itself returns a ScopedKey when using ScopedKeyGenerator::build() at runtime.
 *          This allows for chaining operations at compile time, utilizing its security features compared to manually defining a key.
 * @tparam MaxOps Maximum operations allowed
 * @todo Later on we might allow for true addMember and addIndex in ScopedKeyView using a fixed size buffer?
 */
template<std::size_t MaxOps = 16>
class ScopedKeyGenerator {
    std::string_view base_;
    std::array<OpData, MaxOps> ops_;
    std::size_t count_ = 0;

public:
    explicit constexpr ScopedKeyGenerator(std::string_view const base)
        : base_(base), ops_() {}

    constexpr ScopedKeyGenerator(std::string_view const base, const std::array<OpData, MaxOps>& ops, std::size_t const count)
        : base_(base), ops_(ops), count_(count) {}

    // Chain operations at compile-time
    constexpr ScopedKeyGenerator addMember(std::string_view const member) const {
        auto result = *this;
        if (result.count_ < MaxOps) {
            result.ops_[result.count_] = OpData(member);
            ++result.count_;
        }
        else {
            throw std::out_of_range("Exceeded maximum operations in ScopedKeyGenerator");
        }
        return result;
    }

    constexpr ScopedKeyGenerator addIndex(std::size_t const index) const {
        auto result = *this;
        if (result.count_ < MaxOps) {
            result.ops_[result.count_] = OpData(index);
            ++result.count_;
        }
        else {
            throw std::out_of_range("Exceeded maximum operations in ScopedKeyGenerator");
        }
        return result;
    }

    // Evaluate at runtime
    ScopedKey build() const {
        ScopedKey result(base_);
        for (std::size_t i = 0; i < count_; ++i) {
            if (const auto& op = ops_[i]; op.type == OpType::Member) {
                result = result.addMember(op.memberName);
            } else {
                result = result.addIndex(op.indexValue);
            }
        }
        return result;
    }
};

} // namespace Nebulite::Data
#endif // DATA_SCOPEDKEYGENERATOR_HPP
