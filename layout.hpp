#pragma once

#include <utility>

namespace detail {

template<class Extents, bool IsLeft>
class mapping_base : public Extents {
    using index_type = typename Extents::index_type;

    template<std::size_t... Is, class... Indices>
    index_type op_helper(std::index_sequence<Is...>, Indices... is) const {
        return ((is * stride(Is)) + ...);
    }
public:
    constexpr mapping_base() noexcept : Extents() {}
    constexpr mapping_base(mapping_base const& other) noexcept : Extents(static_cast<Extents const&>(other)) {}
    constexpr mapping_base(mapping_base&& other) noexcept : Extents(std::move(static_cast<std::add_rvalue_reference_t<Extents>>(other))) {}
    constexpr mapping_base(Extents const& e) noexcept : Extents(e) {}
    template<class OtherExtents, bool B>
    constexpr mapping_base(mapping_base<OtherExtents, B> const& other) : Extents(static_cast<OtherExtents const&>(other)) {}

    mapping_base& operator=(mapping_base&&) noexcept = default;
    mapping_base& operator=(mapping_base const& other) noexcept = default;
    template<class OtherExtents, bool B>
    constexpr mapping_base& operator=(mapping_base<OtherExtents, B> const& other) {
        static_cast<Extents&>(*this) = static_cast<OtherExtents const&>(other);
        return *this;
    }

    Extents extents() const noexcept { return static_cast<Extents const&>(*this); }

    constexpr index_type required_span_size() const noexcept {
        return static_cast<Extents const&>(*this).size();
    }

    index_type operator[](index_type i) const {
        if constexpr (Extents::rank() == 0)
            return 0;
        else
            return i * stride(i);
    }

    template<class... Indices>
    index_type operator()(Indices... is) const {
        static_assert(sizeof...(Indices) == Extents::rank());
        static_assert((std::is_convertible_v<Indices, index_type> && ...), "");

        if constexpr (Extents::rank() == 0)
            return 0;
        else
            return op_helper(std::make_index_sequence<sizeof...(Indices)>{}, is...);
    }

    static constexpr bool is_always_unique() { return true; }
    static constexpr bool is_always_contiguous() { return true; }
    static constexpr bool is_always_strided() { return true; }

    constexpr bool is_unique() const { return true; }
    constexpr bool is_contiguous() const { return true; }
    constexpr bool is_strided() const { return true; }

    index_type stride(std::size_t const r) const noexcept {
        if constexpr (IsLeft) {
            index_type s = 1;
            for (std::size_t k = 0; k < r; ++k)
                s *= static_cast<Extents const&>(*this).extent(k);
            return s;
        }
        else { // IsRight
            index_type s = 1;
            for (std::size_t k = r + 1; k < static_cast<Extents const&>(*this).rank(); ++k)
                s *= static_cast<Extents const&>(*this).extent(r);
            return s;
        }
    }

    template<class OtherExtents, bool B>
    constexpr bool operator==(mapping_base<OtherExtents, B> const& other) const noexcept {
        return static_cast<Extents const&>(*this) == static_cast<OtherExtents const&>(other);
    }

    template<class OtherExtents, bool B>
    constexpr bool operator!=(mapping_base<OtherExtents, B> const& other) const noexcept {
        return !(*this == other);
    }
};

} // namespace detail

struct layout_left {
    template<class Extent>
    using mapping = detail::mapping_base<Extent, true>;
};

struct layout_right {
    template<class Extent>
    using mapping = detail::mapping_base<Extent, false>;
};