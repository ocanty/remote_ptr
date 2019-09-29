
#include <cstdint>
#include <utility>
#include <vector>

using read_buffer_func = std::add_pointer<
        const std::vector<std::uint8_t>&(std::uintptr_t address)>::type;

using write_buffer_func = std::add_pointer<
        void(std::uintptr_t address,
                const std::vector<std::uint8_t>& buffer)>::type;

/**
 * Simple class for abstracting RPM/WPM to a smart pointer
 */
template <typename T, read_buffer_func read_remote, write_buffer_func write_remote>
class remote_ptr {
public:
    using value_type = T;
    using instance_type = remote_ptr<T, read_remote, write_remote>;

    // Constructor
    remote_ptr(std::uintptr_t address) :
        m_address(address) {
        m_buffer.resize(sizeof(value_type));
    };

    const value_type& read() {
        m_buffer.resize(sizeof(rw_class_proxy));
        read_remote(m_address, m_buffer);
        return *(reinterpret_cast<value_type*>(m_buffer.data()));
    }

    void write(const value_type& v) const {
        write_remote(m_address, m_buffer);
    }

    /**
     * A class that can construct a temporary 
     * Class 
     */
    template <typename U = value_type, typename = std::enable_if_t<std::is_class<value_type>::value>>
    class rw_class_proxy : public U {
    public:
        rw_class_proxy(instance_type& exterior) :
            U(*(exterior.read())), m_exterior(exterior) {

        }
        virtual ~rw_class_proxy() {
            m_exterior->write(*static_cast<U*>(this))
        };

        U* operator->() {
            return static_cast<U*>(this);
        }

    private:
        instance_type& m_exterior;
    };

    template <typename U = value_type, typename = std::enable_if_t<std::is_fundamental<value_type>::value>>
    class rw_value_proxy : public U {
    public:
        rw_value_proxy(instance_type& exterior) :
            U(*(exterior.read())), m_exterior(exterior) {
        }
        virtual ~rw_class_proxy() {
            m_exterior->write(*static_cast<U*>(this))
        };

        U* operator->() {
            return static_cast<U*>(this);
        }

    private:
        instance_type& m_exterior;
    };



    template <typename U = value_type, typename = std::enable_if_t<std::is_class<value_type>::value>>
    rw_class_proxy<value_type> operator->() {
        return rw_class_proxy<value_type>(this);
    }

    template <typename U = value_type, typename = std::enable_if_t<std::is_class<value_type>::value>>
    rw_class_proxy<value_type> operator*() {
        return rw_class_proxy<value_type>(this);
    }

    template <typename U = value_type, typename = std::enable_if_t<std::is_fundamental<value_type>::value>>
    rw_value_proxy<value_type>& operator*() {
        static rw_value_proxy<value_type> val(this);
        return val;
    }

private:
    std::vector<std::uint8_t> m_buffer;
    std::uintptr_t m_address;

}