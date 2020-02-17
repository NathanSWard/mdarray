#pragma once

#include <memory>

template<class T>
struct default_container_policy {
    using element_type = T;
    using container_type = std::unique_ptr<T[]>;
    using pointer = T*;
    using const_pointer = T const*;
    using reference = T&;
    using const_reference = T const&;
    using offset_policy = default_container_policy<T>;

    container_type create(std::size_t const n) const { return std::make_unique<T[]>(n); }

    reference access(container_type const& p, std::ptrdiff_t const i) { return p[i]; }
    const_reference access(container_type const& p, std::ptrdiff_t const i) const { return p[i]; }

    pointer offset(pointer const p, std::ptrdiff_t const i) { return p + i; }
    const_pointer offset(const_pointer const p, std::ptrdiff_t const i) const { return p + i; }

    element_type* decay(pointer const p) { return p; }
    element_type const* decay(pointer const p) const { return p; }

    pointer data(container_type& c) { return c.get(); }
    const_pointer data(container_type const& c) const { return c.get(); }
};

