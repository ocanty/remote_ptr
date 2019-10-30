#pragma once
#ifndef REMOTE_PTR_CLASS_PROXY_HPP
#define REMOTE_PTR_CLASS_PROXY_HPP

#include <vector>
#include <utility>
#include "buffer_func.hpp"

/**
 * A class_proxy reads in class/struct data from a read_remote call and allows you to perform most if not
 * all operations that class supports had it been in your memory space
 * 
 * When the class_proxy destructs it will write_remote any changes you made
 */
template <typename T, read_buffer_func read_remote, write_buffer_func write_remote>
class class_proxy : public T {
private:
    using value_type = T;
    using instance_type = class_proxy<value_type, read_remote, write_remote>;


    std::uintptr_t m_address;
    std::vector<std::uint8_t> m_original;

public:
    class_proxy(std::uintptr_t address) 
        : value_type(), m_address(address) {
        std::cout << "class_proxy()" << std::endl;

        // Build a buffer that we can diff against later
        m_original.resize(sizeof(value_type));

        // Copy in remote data into the buffer
        read_remote(m_address, &m_original[0], m_original.size());

        // Copy buffer into our class so we can manipulate it
        std::memcpy(static_cast<value_type*>(this), (void*)&m_original[0], m_original.size());
    };

    ~class_proxy() {
        std::cout << "~class_proxy()" << std::endl;

        auto original_it        = m_original.begin();
        auto class_this         = static_cast<value_type*>(this);
        auto class_data_start   = reinterpret_cast<std::uint8_t*>(class_this);
        auto class_data_it      = reinterpret_cast<std::uint8_t*>(class_this);
        
        std::size_t diff_count = 0;

        // Diff two buffers and write changes
        while(original_it != m_original.end()) {
            auto offset = std::distance(m_original.begin(), original_it);
            
            // If this byte does not match original increment the sequence of changed bytes
            if(*original_it != *class_data_it) {
                diff_count++;
            }
            else if (diff_count > 0) { // If the byte does match and there was a previous sequence being tracked
                // The sequence has ended
                // Commit all previous changes
                write_remote(m_address + offset - diff_count, &class_data_start[offset - diff_count], diff_count);
                std::cout << "Committing " << diff_count << " bytes to " << m_address + offset - diff_count << std::endl;
                
                // Reset count
                diff_count = 0;
            }

            ++class_data_it;
            ++original_it;
        }
    };

    operator value_type&() {
        return static_cast<value_type&>(*this);
    };

    operator value_type&() const {
        return static_cast<const value_type&>(*this);
    }

    value_type* operator->() {
        return static_cast<value_type*>(this);
    };
    
#define OP(op) \
    template <typename ArgsR> \
    auto operator op (ArgsR&& value) -> decltype((*static_cast<value_type*>(this)) op ArgsR{}) {   \
        return value_type::operator op (std::forward<ArgsR>(value)); \
    } 

    OP(=);
    OP(+=);
    OP(-=);
    OP(*=);
    OP(/=);
    OP(%=);
    OP(&=);
    OP(|=);
    OP(^=);
    OP(<<=);
    OP(>>=);
#undef OP

    template <typename A = value_type>
    auto operator--(int) -> decltype(std::declval<A>().operator--(1)) {
        return value_type::operator--(1);
    }

    template <typename A = value_type>
    auto operator--() -> decltype(std::declval<A>().operator--()) {
        return value_type::operator--();
    }

    template <typename A = value_type>
    auto operator++(int) -> decltype(std::declval<A>().operator++(1)) {
        return value_type::operator++(1);
    }

    template <typename A = value_type>
    auto operator++() -> decltype(std::declval<A>().operator++()) {
        return value_type::operator++();
    }

    template <typename A = value_type>
    auto operator!() -> decltype(std::declval<A>().operator!()) const {
        return value_type::operator!();
    }

    template <typename A = value_type>
    auto operator-() -> decltype(std::declval<A>().operator-()) const {
        return value_type::operator-();
    }

    template <typename A = value_type>
    auto operator~() -> decltype(std::declval<A>().operator~()) const {
        return value_type::operator~();
    }

    template <typename A = value_type>
    auto operator+() -> decltype(std::declval<A>().operator+()) const {
        return value_type::operator+();
    }

#define OP(op) \
    template <typename ArgsL> \
    friend auto operator op (ArgsL&& l, instance_type&& r) -> decltype(std::declval<ArgsL>() op std::declval<value_type>()) const { \
        return std::forward<ArgsL>(l) op static_cast<value_type>(r); \
    } \
    template <typename ArgsR> \
    friend auto operator<<(instance_type&& l, ArgsR&& r) -> decltype(std::declval<value_type>() op std::declval<ArgsR>()) const { \
        return static_cast<value_type>(l) op std::forward<ArgsR>(r); \
    } 

    OP(==);
    OP(!=);
    OP(<);
    OP(>);
    OP(<=);
    OP(>=);
    // OP(<=>);
    OP(+);
    OP(-);
    OP(*);
    OP(/);
    OP(%);
    OP(&);
    OP(|);
    OP(^);
    OP(<<);
    OP(>>);
    OP(&&);
    OP(||);
#undef OP
};

#endif