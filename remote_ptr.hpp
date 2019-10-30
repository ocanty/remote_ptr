#pragma once
#ifndef REMOTE_PTR_REMOTE_PTR_HPP
#define REMOTE_PTR_REMOTE_PTR_HPP

#include "buffer_func.hpp"
#include "class_proxy.hpp"
#include "value_proxy.hpp"

template <typename T, read_buffer_func read_remote, write_buffer_func write_remote>
class remote_ptr {
public:
    using value_type = T;
    using instance_type = remote_ptr<value_type, read_remote, write_remote>;

    explicit remote_ptr(std::uintptr_t address) :
        m_address(address) {    
    };

    template <typename U = value_type, std::enable_if_t<std::is_class<U>::value, int> = 0>
    class_proxy<U, read_remote, write_remote> operator->() {
        return class_proxy<U, read_remote, write_remote>(m_address);
    }

    template <typename U = value_type, std::enable_if_t<std::is_class<U>::value, int> = 0>
    class_proxy<U, read_remote, write_remote> operator*() {
        return class_proxy<U, read_remote, write_remote>(m_address);
    }

    template <typename U = value_type, std::enable_if_t<!std::is_class<U>::value, int> = 0>
    value_proxy<U, read_remote, write_remote> operator*() {
        return value_proxy<U, read_remote, write_remote>(m_address);
    }

    std::uintptr_t get() const {
        return m_address;
    }

private:
    std::uintptr_t m_address;
};

#endif