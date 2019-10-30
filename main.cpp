
#include <vector>
#include <cstdint>
#include <cstring>
#include <iostream>

#include "remote_ptr.hpp"

std::vector<std::uint8_t> g_buffer;

void read_remote(std::uintptr_t address, std::uint8_t* buffer, std::size_t n) {
    if(address > (g_buffer.size()-1 + n)) return;
    std::memcpy(buffer, &g_buffer[address], n);
}

void write_remote(std::uintptr_t address, std::uint8_t* buffer, std::size_t n) {
    if(address > (g_buffer.size()-1 + n)) return;
    std::memcpy(&g_buffer[address], buffer, n);
}

template <typename value_type>
class test_object {
    public:
        test_object() {
            m_val = 0;
        }

        explicit test_object(value_type initial) 
            : m_val(initial) {

        };

        template <typename ArgR>
        test_object& operator+=(ArgR&& other) {
            std::cout << "triggered " << m_val << std::endl;
            m_val += std::forward<ArgR>(other);
            std::cout << "triggereda " << m_val << std::endl;
            return *this;
        };

        template <typename ArgR>
        test_object& operator/=(ArgR&& other) {
            m_val /= std::forward<ArgR>(other);
            return *this;
        };

        template <typename T>
        friend std::ostream& operator<<(std::ostream& os, const test_object<T>& obj) {
            return os << obj.get();
        }

        const value_type& get() const {
            return m_val;
        };

        bool operator!() const {
            return false;
        }

        value_type operator--(int) {
            auto preserve = m_val;
            m_val--;
            return preserve;
        }

        value_type operator--() {
            return --m_val;
        }

    //private:
        value_type m_val;

};

struct test {
    int a, b, c, d;
};

template <typename T>
using ptr = remote_ptr<T, read_remote, write_remote>;

int main(int argc, char** argv) {
    //using a = test_object<std::uint32_t>;
    g_buffer.resize(0x1000);

    *(std::uintptr_t*)&g_buffer[0] = 0x0008;
    *(std::uintptr_t*)&g_buffer[0x8] = 0xDEADBEEF;
    remote_ptr<std::uintptr_t*, read_remote, write_remote> ab(0);
    remote_ptr<std::uintptr_t, read_remote, write_remote> c(0);

    std::cout << *(*ab) << std::endl;

    std::cout << "here??" << std::endl;
    test_object<std::uint32_t> obj(5);
    write_remote(0, (std::uint8_t*)&obj, sizeof(test_object<std::uint32_t>));

    remote_ptr<test_object<std::uint32_t>, read_remote, write_remote> ac(0);
    std::cout <<"pre-> " << ac->get() << std::endl;

    (*ac) += 25;
    
    // (*ac) = 5;

    // (*ac)--;
    // --(*ac);

    std::cout << "assigned" << (*ac) << 5 << std::endl;
    //std::cout << (*ac).m_val << std::endl;
    //std::cout << ac->get() << std::endl;
    // std::cout <<"post-> " << ac->get() << std::endl;

    // write_remote(0, (std::uint8_t*)&obj, sizeof(test_object<std::uint32_t>));
    // std::cout << obj.get() << std::endl;

    return 0;
}
