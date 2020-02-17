#pragma once

#include <utility>

static inline constexpr std::ptrdiff_t dynamic_extent = -1;

template<std::ptrdiff_t... Extents>
inline constexpr std::size_t count_dynamic_extents() {
    return (static_cast<std::size_t>(Extents == dynamic_extent) + ...);
}

template<std::size_t DynamicCount, std::ptrdiff_t... Extents>
class extents_base {
public:
    using index_type = std::ptrdiff_t;
private:
    static constexpr index_type static_size_ = ((Extents == dynamic_extent ? 1 : Extents) * ...); 

    template<std::size_t... Is>
    static constexpr index_type static_extent_impl(std::size_t const n, std::index_sequence<Is...>) noexcept {
        return ((n == Is ? Extents : 1) * ...);
    }

    template<std::size_t DynamicIndex>
    constexpr index_type extent_impl(std::size_t const n, std::integer_sequence<std::ptrdiff_t>, std::index_sequence<>) const noexcept {
        return 1;
    }

    template<std::size_t DynamicIndex, std::ptrdiff_t E, std::ptrdiff_t... Es, std::size_t I, std::size_t... Is>
    constexpr index_type extent_impl(std::size_t const n, std::integer_sequence<std::ptrdiff_t, E, Es...>, std::index_sequence<I, Is...>) const noexcept {
        if constexpr (E == dynamic_extent) {
            return n == I ? dynamic_storage_[DynamicIndex] 
                          : extent_impl<DynamicIndex + 1>(n, std::integer_sequence<std::ptrdiff_t, Es...>{}, std::index_sequence<Is...>{});
        }
        else {
            return n == I ? E 
                          : extent_impl<DynamicIndex>(n, std::integer_sequence<std::ptrdiff_t, Es...>{}, std::index_sequence<Is...>{});
        }
    }

public:
    ~extents_base() noexcept = default;
    constexpr extents_base() noexcept = default;
    constexpr extents_base(extents_base const&) = default;
    constexpr extents_base(extents_base&&) = default;
    constexpr extents_base& operator=(extents_base const&) = default;
    constexpr extents_base& operator=(extents_base&&) = default;

    template<class... IndexType>
    constexpr explicit extents_base(IndexType... DynamicExtents) noexcept 
        : dynamic_storage_{DynamicExtents...}
    {
        static_assert(std::conjunction_v<std::is_convertible<IndexType, index_type>...>, "");
        static_assert(DynamicCount == sizeof...(IndexType), "");
    }

    template<class IndexType>
    constexpr extents_base(std::array<IndexType, DynamicCount> const& other) noexcept {
        static_assert(std::is_convertible_v<IndexType,  index_type>, "");
        for (std::size_t i = 0; i < DynamicCount; ++i)
            dynamic_storage_[i] = other[i];
    }

    template<std::ptrdiff_t... OtherExtents>
    constexpr extents_base(extents_base<OtherExtents...> const& other) {
        *this = other;
    }

    template<std::ptrdiff_t... OtherExtents>
    constexpr extents_base& operator=(extents_base<OtherExtents...> const& other) {
        static_assert(sizeof...(Extents) == sizeof...(OtherExtents), "");
        static_assert(std::conjunction_v<std::conjunction<std::bool_constant<Extents != dynamic_extent>, 
                                         std::bool_constant<OtherExtents != dynamic_extent>,
                                         std::bool_constant<Extents == OtherExtents>>...>, "");
        std::size_t dynamic_index = 0;
        for (std::size_t i = 0; i < sizeof...(Extents); ++i) {
            if (static_extent(i) == dynamic_extent)
                dynamic_storage_[dynamic_index++] = other.extent(i);
        }
    }

    static constexpr std::size_t rank() noexcept { return sizeof...(Extents); }
    static constexpr std::size_t rank_dynamic() noexcept { return DynamicCount; }

    static constexpr index_type static_extent(std::size_t const n) noexcept {
        return static_extent_impl(n, std::make_index_sequence<sizeof...(Extents)>{});
    }

    constexpr index_type extent(std::size_t const n) const noexcept {
        return extent_impl<0>(n, std::integer_sequence<std::ptrdiff_t, Extents...>{}, std::make_index_sequence<sizeof...(Extents)>{});
    }

    constexpr index_type size() const noexcept { 
        index_type size = static_size_;
        for (auto const& s : dynamic_storage_)
            size *= s;
        return size;
    }

private:
    std::array<index_type, DynamicCount> dynamic_storage_ = {};
};

template<std::ptrdiff_t... Extents>
class extents_base<0, Extents...> {
    static_assert(((Extents != dynamic_extent) && ...), "");
public:
    using index_type = std::ptrdiff_t;
private:
    static constexpr index_type size_ = (Extents * ...);

    template<std::size_t... Is>
    static constexpr index_type static_extent_impl(std::size_t const n, std::index_sequence<Is...>) noexcept {
        return ((n == Is ? Extents : 1) * ...);
    }

public:
    ~extents_base() noexcept = default;
    constexpr extents_base() noexcept = default;
    constexpr extents_base(extents_base const&) = default;
    constexpr extents_base(extents_base&&) = default;
    extents_base& operator=(extents_base const&) = default;
    extents_base& operator=(extents_base&&) = default;

    template<class IndexType>
    constexpr extents_base(std::array<IndexType, 0> const& other) noexcept {
        static_assert(std::is_convertible_v<IndexType,  index_type>, "");
    }

    template<std::ptrdiff_t... OtherExtents>
    constexpr extents_base(extents_base<OtherExtents...> const& other) {
        *this = other;
    }

    template<std::ptrdiff_t... OtherExtents>
    constexpr extents_base& operator=(extents_base<OtherExtents...> const& other) {
        static_assert(sizeof...(Extents) == sizeof...(OtherExtents), "");
        static_assert(((Extents == OtherExtents) && ...), "");
        return *this;
    }

    static constexpr std::size_t rank() noexcept { return sizeof...(Extents); }
    static constexpr std::size_t rank_dynamic() noexcept { return 0; }

    static constexpr index_type static_extent(std::size_t const n) noexcept {
        return static_extent_impl(n, std::make_index_sequence<sizeof...(Extents)>{});
    }

    constexpr index_type extent(std::size_t const n) const noexcept {
        return static_extent(n);
    }

    constexpr index_type size() const noexcept { return size_; }
};

template<std::ptrdiff_t... Extents> 
class extents : public extents_base<count_dynamic_extents<Extents...>(), Extents...> {
    using base = extents_base<count_dynamic_extents<Extents...>(), Extents...>;
public:
    template<class... Args>
    extents(Args&&... args) : base(std::forward<Args>(args)...) {}
};

template<class Lhs, class Rhs, std::size_t... Is>
constexpr bool extents_equal(Lhs const& lhs, Rhs const& rhs, std::index_sequence<Is...>) noexcept {
    return ((lhs.extent(Is) == rhs.extent(Is)) && ...);
}

template<std::ptrdiff_t... Lhs, std::ptrdiff_t... Rhs>
constexpr bool operator==(extents<Lhs...> const& lhs, extents<Rhs...> const& rhs) noexcept {
    if constexpr (sizeof...(Lhs) != sizeof...(Rhs))
        return false;
    else
        return extents_equal(lhs, rhs, std::make_index_sequence<sizeof...(Lhs)>{});
}

template<std::ptrdiff_t... Lhs, std::ptrdiff_t... Rhs>
constexpr bool operator!=(extents<Lhs...> const& lhs, extents<Rhs...> const& rhs) noexcept {
    return !(lhs == rhs);
}