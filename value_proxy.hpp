#pragma once
#ifndef REMOTE_PTR_VALUE_PROXY_HPP
#define REMOTE_PTR_VALUE_PROXY_HPP

#include <vector>
#include <utility>
#include "buffer_func.hpp"

/**
 * A class_proxy reads in a value from a read_remote call and allows you to perform most if not
 * all operations that value supports had it been in your memory space
 * 
 * When the value_proxy destructs it will write_remote any changes you made
 */
template <typename T, read_buffer_func read_remote, write_buffer_func write_remote>
class value_proxy {
    using value_type = T;
    using instance_type = value_proxy<value_type, read_remote, write_remote>;

private:
    std::uintptr_t m_address;
    std::vector<std::uint8_t> m_original;
    std::vector<std::uint8_t> m_future;

    constexpr value_type* value() {
        std::cout << "value()" << std::endl;
        for(int i = 0; i < m_original.size()/8; i++) {
            std::cout << "Read: " << ((int*)(m_future.data()))[i] << std::endl;
        }
        return reinterpret_cast<value_type*>(m_future.data());
    }

public:
    value_proxy(std::uintptr_t address) :
        m_address(address) {
        std::cout << "value_proxy()" << std::endl;

        m_original.resize(sizeof(value_type));
        read_remote(m_address, &m_original[0], m_original.size());
        m_future = m_original;
    };

    ~value_proxy() {
        //std::cout << "~value_proxy()" << std::endl;
        // Walk the two buffers we started and finished with and compare changes before and after creation of this class
        std::size_t diff_count = 0;

        auto original_it = m_original.begin();
        auto changed_it  = m_future.begin();

        while(original_it != m_original.end()) {
            auto offset = std::distance(m_original.begin(), original_it);

            // If this byte does not match increment the sequence of changed bytes
            if(*original_it != *changed_it) {
                diff_count++;
            }
            else if (diff_count > 0) { // If the byte does match and there was a previous sequence being tracked
                // The sequence has ended
                // Commit all previous changes
                write_remote(m_address + offset - diff_count, &m_future[offset - diff_count], diff_count);
                std::cout << "Committing " << diff_count << " bytes to " << m_address + offset - diff_count << std::endl;

                diff_count = 0;
            }

            ++changed_it;
            ++original_it;
        }

        std::cout << "~value_proxy()" << std::endl;
    };

    public:
    operator value_type&() {
        std::cout << "value_type()" << std::endl;
        return reinterpret_cast<value_type&>(*value());
    }

    typedef typename std::remove_pointer<value_type>::type value_type_deref;
    /**
     * Dereference the value remotely if T is a pointer type
     * @notes This reads the value stored at the memory address, and sets the address to it
     *        Disabled for non-poin8ter types
     * @return A value represented the dereferenced remote value
     */
    template <typename U = value_type, std::enable_if_t<std::is_pointer<U>::value, int> = 0>
    value_proxy<value_type_deref, read_remote, write_remote> operator*() {
        return (value_proxy<value_type_deref, read_remote, write_remote>(reinterpret_cast<std::uintptr_t>(*reinterpret_cast<value_type*>(m_future.data()))));
    }
};

#endif