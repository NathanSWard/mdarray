#pragma once

#include <tuple>
#include <type_traits>

#include "container_policy.hpp"
#include "extents.hpp"
#include "layout.hpp"

namespace detail {

template<std::ptrdiff_t... N>
using seq = std::integer_sequence<std::ptrdiff_t, N...>;

template<class Out, class In>
struct remove_extents_tail_impl;

template<std::ptrdiff_t... Es, std::ptrdiff_t Tail>
struct remove_extents_tail_impl<extents<Es...>, extents<Tail>> {
    using type = extents<Es...>;
};

template<std::ptrdiff_t... Es, std::ptrdiff_t Head, std::ptrdiff_t... Tail>
struct remove_extents_tail_impl<extents<Es...>, extents<Head, Tail...>> {
    using type = typename remove_extents_tail_impl<extents<Es..., Head>, extents<Tail...>>::type;
};

template<class E>
struct remove_extents_tail {
    using type = typename remove_extents_tail_impl<extents<>, E>::type;
};

template<class E>
using remove_extents_tail_t = typename remove_extents_tail<E>::type;

template<class E>
struct remove_extents_head;

template<template<std::ptrdiff_t...> class E, std::ptrdiff_t Head, std::ptrdiff_t... Tail>
struct remove_extents_head<E<Head, Tail...>> {
    using type = E<Tail...>;
};

template<class E>
using remove_extents_head_t = typename remove_extents_head<E>::type;

} // namespace detail

template<class, class, class, class>
class basic_mdarray;

template<class T, class Extents, class LayoutPolicy = layout_left, class ContainerPolicy = default_container_policy<T>>
class basic_mdarray_view : private LayoutPolicy::template mapping<Extents>, private ContainerPolicy {
public:
    using extents_type = Extents;
    using layout_type = LayoutPolicy;
    using mapping_type = typename layout_type::template mapping<extents_type>;
    using container_policy_type = ContainerPolicy;
    using element_type = typename container_policy_type::element_type;
    using value_type = std::remove_cv_t<element_type>;
    using index_type = std::ptrdiff_t;
    using difference_type = std::ptrdiff_t;
    using pointer = typename container_policy_type::pointer;
    using reference = typename container_policy_type::reference;
    using const_pointer = typename container_policy_type::const_pointer;
    using const_reference = typename container_policy_type::const_reference;
    using container_type = typename container_policy_type::container_type;

private:
    static constexpr auto rank_ = Extents::rank();
    static constexpr bool one_extent_ = (rank_ == 1);
    constexpr mapping_type& as_mt() noexcept { return static_cast<mapping_type&>(*this); }
    constexpr mapping_type const& as_mt() const noexcept { return static_cast<mapping_type const&>(*this); }
    constexpr container_policy_type& as_cpt() noexcept { return static_cast<container_policy_type&>(*this); }
    constexpr container_policy_type const& as_cpt() const noexcept { return static_cast<container_policy_type const&>(*this); }

public:
    template<class U, class E, class LP, class CP>
    constexpr basic_mdarray_view(basic_mdarray<U, E, LP, CP>& md)
        : mapping_type(md.mapping())
        , container_policy_type(md.container_policy())
        , ptr_(md.data())
    {}

    constexpr basic_mdarray_view(T* ptr) noexcept : ptr_(ptr) {}

    constexpr auto operator[](index_type const i) noexcept {
        if constexpr (one_extent_)
            return as_cpt().access(ptr_, i);
        else {
            using View = basic_mdarray_view<T, detail::remove_extents_tail_t<extents_type>, layout_type, container_policy_type>;
            return View(std::addressof(as_cpt().access(ptr_, as_mt()[i])));
        }
    }

    template<class... IndexTypes>
    constexpr reference operator()(IndexTypes... is) noexcept {
        static_assert(sizeof...(IndexTypes) == rank_, "");
        static_assert((std::is_convertible_v<IndexTypes, index_type> && ...), "");

        if constexpr (one_extent_)
            return as_cpt().access(ptr_, is...);
        else
            return as_cpt().access(ptr_, as_mt()(is...));
    }
    
    template<class... IndexTypes>
    constexpr const_reference operator()(IndexTypes... is) const noexcept {
        static_assert(sizeof...(IndexTypes) == rank_, "");
        static_assert((std::is_convertible_v<IndexTypes, index_type> && ...), "");

        if constexpr (one_extent_)
            return as_cpt().access(ptr_, is...);
        else
            return as_cpt().access(ptr_, as_mt()(is...));
    }

private:
    T* ptr_ = nullptr;
};

template<class T, class E, class LP, class CP>
basic_mdarray_view(basic_mdarray<T, E, LP, CP>) -> basic_mdarray_view<T, E, LP, CP>;

template<class T, std::ptrdiff_t... Extents>
using mdarray_view = basic_mdarray_view<T, extents<Extents...>>;

template<class T, class Extents, class LayoutPolicy = layout_left, class ContainerPolicy = default_container_policy<T>>
class basic_mdarray : private LayoutPolicy::template mapping<Extents>, private ContainerPolicy {
public:
    using extents_type = Extents;
    using layout_type = LayoutPolicy;
    using mapping_type = typename layout_type::template mapping<extents_type>;
    using container_policy_type = ContainerPolicy;
    using element_type = typename container_policy_type::element_type;
    using value_type = std::remove_cv_t<element_type>;
    using index_type = std::ptrdiff_t;
    using difference_type = std::ptrdiff_t;
    using pointer = typename container_policy_type::pointer;
    using reference = typename container_policy_type::reference;
    using const_pointer = typename container_policy_type::const_pointer;
    using const_reference = typename container_policy_type::const_reference;
    using container_type = typename container_policy_type::container_type;

    // view_type
    // const_view_type

private:
    static constexpr auto rank_ = Extents::rank();
    static constexpr bool one_extent_ = (rank_ == 1);
    constexpr mapping_type& as_mt() noexcept { return static_cast<mapping_type&>(*this); }
    constexpr mapping_type const& as_mt() const noexcept { return static_cast<mapping_type const&>(*this); }
    constexpr container_policy_type& as_cpt() noexcept { return static_cast<container_policy_type&>(*this); }
    constexpr container_policy_type const& as_cpt() const noexcept { return static_cast<container_policy_type const&>(*this); }
    
    template<class IndexType, std::size_t N, std::size_t... Is>
    constexpr reference op_paren_arr_helper(std::array<IndexType, N> const& indices, std::index_sequence<Is...>) {
        if constexpr (one_extent_)
            return as_cpt().access(c_, std::get<0>(indices));
        else
            return as_cpt().access(c_, as_mt()(std::get<Is>(indices)...));
    }

    template<class IndexType, std::size_t N, std::size_t... Is>
    constexpr const_reference op_paren_arr_helper(std::array<IndexType, N> const& indices, std::index_sequence<Is...>) const {
        if constexpr (one_extent_)
            return as_cpt().access(c_, std::get<0>(indices));
        else
            return as_cpt().access(c_, as_mt()(std::get<Is>(indices)...));
    }

public:
    constexpr basic_mdarray() noexcept : mapping_type(), container_policy_type(), c_(as_cpt().create(size())) {}
    constexpr basic_mdarray(basic_mdarray const&) noexcept = default;
    constexpr basic_mdarray(basic_mdarray&&) noexcept = default;

    template<class... IndexTypes>
    explicit constexpr basic_mdarray(IndexTypes... dynamic_extents) 
        : mapping_type(Extents(dynamic_extents...)) 
        , container_policy_type()
        , c_(as_cpt().create(size()))
    {
        static_assert((sizeof...(IndexTypes) == rank_dynamic()), "");
        static_assert((std::is_convertible_v<IndexTypes, index_type> && ...), "");
        static_assert(std::is_constructible_v<mapping_type, extents_type>, "");
        static_assert(std::is_default_constructible_v<container_policy_type>, "");
    }

    template<class... Args, class... IndexTypes>
    explicit constexpr basic_mdarray(std::in_place_t, std::tuple<Args...> args, IndexTypes... dynamic_extents) 
        : mapping_type(Extents(dynamic_extents...)) 
        , container_policy_type()
        , c_(std::apply([&](auto&&... args) { 
                return as_cpt().create(size(), std::forward<decltype(args)>(args)...);
            }, args))
    {
        static_assert((sizeof...(IndexTypes) == rank_dynamic()), "");
        static_assert((std::is_convertible_v<IndexTypes, index_type> && ...), "");
        static_assert(std::is_constructible_v<mapping_type, extents_type>, "");
        static_assert(std::is_default_constructible_v<container_policy_type>, "");
        static_assert(std::is_constructible_v<element_type, Args...>, "");
    }

    // TODO: add rvalue overload?
    explicit constexpr basic_mdarray(mapping_type const& m)
        : mapping_type(m)
        , container_policy_type()
        , c_(as_cpt().create(size()))
    {}

    // TODO: add rvalue overload?
    explicit constexpr basic_mdarray(mapping_type const& m, container_policy_type const& cp)
        : mapping_type(m)
        , container_policy_type(cp)
        , c_(cp.create(size()))
    {}

    template<class ET, class Exts, class LP, class CP>
    constexpr basic_mdarray(basic_mdarray<ET, Exts, LP, CP> const& other)
        : mapping_type(other.as_mt())
        , container_policy_type(other.as_cpt())
        , c_(other.c_)
    {}

    template<class ET, class Exts, class LP, class CP>
    constexpr basic_mdarray(basic_mdarray<ET, Exts, LP, CP>&& other)
        : mapping_type(std::move(other.as_mt()))
        , container_policy_type(std::move(other.as_cpt()))
        , c_(std::move(other.c_))
    {}

    constexpr basic_mdarray& operator=(basic_mdarray const&) noexcept = default;
    constexpr basic_mdarray& operator=(basic_mdarray&&) noexcept = default;

    template<class ET, class Exts, class LP, class CP>
    constexpr basic_mdarray& operator=(basic_mdarray<ET, Exts, LP, CP> const& other) noexcept {
        as_mt() = other.as_mt();
        as_cpt() = other.as_cpt();
        c_ = other.c_;
        return *this;
    }

    template<class ET, class Exts, class LP, class CP>
    constexpr basic_mdarray& operator=(basic_mdarray<ET, Exts, LP, CP>&& other) noexcept {
        as_mt() = std::move(other.as_mt());
        as_cpt() = std::move(other.as_cpt());
        c_ = std::move(other.c_);
        return *this;
    }

    // operator[](index_type);
    // operator[](index_type) const;

    template<class... IndexTypes>
    constexpr reference operator()(IndexTypes... is) noexcept {
        static_assert(sizeof...(IndexTypes) == rank_, "");
        static_assert((std::is_convertible_v<IndexTypes, index_type> && ...), "");

        if constexpr (one_extent_)
            return as_cpt().access(c_, is...);
        else
            return as_cpt().access(c_, as_mt()(is...));
    }
    
    template<class... IndexTypes>
    constexpr const_reference operator()(IndexTypes... is) const noexcept {
        static_assert(sizeof...(IndexTypes) == rank_, "");
        static_assert((std::is_convertible_v<IndexTypes, index_type> && ...), "");

        if constexpr (one_extent_)
            return as_cpt().access(c_, is...);
        else
            return as_cpt().access(c_, as_mt()(is...));
    }

    template<class IndexType, std::size_t N>
    constexpr reference operator()(std::array<IndexType, N> const& indices) {
        static_assert(N == rank_, "");
        static_assert(std::is_convertible_v<IndexType, index_type>, "");
        return op_paren_arr_helper(indices, std::make_index_sequence<N>{});
    }

    template<class IndexType, std::size_t N>
    constexpr const_reference operator()(std::array<IndexType, N> const& indices) const {
        static_assert(N == rank_, "");
        static_assert(std::is_convertible_v<IndexType, index_type>, "");
        return op_paren_arr_helper(indices, std::make_index_sequence<N>{});
    }

    static constexpr std::size_t rank() noexcept { return Extents::rank(); }
    static constexpr std::size_t rank_dynamic() noexcept { return Extents::rank_dynamic(); }
    static constexpr index_type static_extent(std::size_t const i) noexcept { return Extents::static_extent(i); } 

    constexpr extents_type extents() const noexcept { return as_mt().extents(); }
    constexpr index_type extent(std::size_t const i) const noexcept { return extents().extent(i); }
    constexpr index_type size() const noexcept { return extents().size(); }
    constexpr index_type unique_size() const noexcept {
        return as_mt().is_unique() ? size() : 0; // FIX!!!!!!!!!!!!
    }

    // view_type view() noexcept;
    // const_view_type view() const noexcept;

    constexpr pointer data() noexcept { return as_cpt().data(c_); }
    constexpr const_pointer data() const noexcept { return as_cpt().data(c_); }
    constexpr container_policy_type container_policy() const noexcept { return as_cpt(); }

    static constexpr bool is_always_unique() noexcept { return mapping_type::is_always_unique(); }
    static constexpr bool is_always_contiguous() noexcept { return mapping_type::is_always_contiguous(); }
    static constexpr bool is_always_strided() noexcept { return mapping_type::is_always_strided(); }
    constexpr mapping_type mapping() const noexcept { return as_mt(); }
    constexpr bool is_unique() const noexcept { return as_mt().is_unique(); }
    constexpr bool is_contiguous() const noexcept { return as_mt().is_contiguous(); }
    constexpr bool is_strided() const noexcept { return as_mt().is_strided(); }
    constexpr index_type stride(std::size_t const r) const { return as_mt().stride(r); }

private:
    container_type c_{};
};

template<class T, std::ptrdiff_t... Extents>
using mdarray = basic_mdarray<T, extents<Extents...>>;