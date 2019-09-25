
#include <iostream>
#include <cstdint>

/**
 * A wrapper class to proxy all read/write calls on a data type
 * 
 * Inherit this class and ovveride the read & write virtual functions
 * These will be used to read and write the value
 * Multithreading this given read/write is up to you
 * 
 * @param value_type Type of variable to proxy read/write, all operators will be overloaded to support these
 */ 
template <typename value_type, class Derived>
class proxy_var {
    public:
        /**
         * @param address The offset of the value that will be passed to your read function when it is asked to return the data
         */
        proxy_var() = default;
        virtual ~proxy_var() = default;

        /**
         * Override these functions for r/w
         */
        virtual const value_type& read() const = 0;
        virtual void write(const value_type&) const = 0;

        /**
         * A class that can construct a temporary via read()
         * All modifications are saved via write() when the temporary destructs
         */
        template <typename U = value_type, typename = std::enable_if_t<std::is_class<value_type>::value>>
        class rw_guard : public value_type {
        private:
            rw_guard(proxy_var<value_type, Derived>& exterior) :
                U(m_exterior.read()), m_exterior(exterior) {
            };

            virtual ~rw_guard() {
                m_exterior.write(static_cast<const U>(*this)); 
            };

            proxy_var<value_type, Derived>& m_exterior;
        };

        template <typename U = value_type, typename = std::enable_if_t<std::is_class<value_type>::value>>
        rw_guard<value_type> operator->() {
            return rw_guard(this);
        }

#define OP(op)                                                      \
    template <typename ArgsR>                                       \
    proxy_var<value_type, Derived>& operator op (ArgsR&& value) {   \
    value_type cur = this->read();                                  \
        cur op value;                                               \
        this->write(cur);                                           \
        return *this;                                               \
    }

    OP(+=);
    OP(-=);
    OP(*=);
    OP(/=);
    OP(|=);
    OP(^=);
    OP(&=);
    OP(%=);
    OP(<<=);
    OP(>>=);
#undef OP
};


template <typename T>
class remote_var : public proxy_var<T, remote_var<T>> {
    public:
        remote_var(std::uintptr_t offset) :
            m_offset(offset) {
            
        };

        const T& read() const {
            std::cout << "read called -> " << temp << std::endl;
            return temp;
        }

        void write(const T& value) const {
            std::cout << "write called -> " << temp << std::endl;
            temp = value;
        }

    private:
        std::uintptr_t m_offset;
        T temp;
};

template <typename value_type>
class test_object {
    public:
        test_object() = default;

        test_object(value_type initial) 
            : m_val(initial) {

        };

        template <typename ArgR>
        test_object& operator+=(ArgR&& other) {
            m_val += other;
            return *this;
        };

        template <typename ArgR>
        test_object& operator/=(ArgR&& other) {
            m_val /= other;
            return *this;
        };

        template <typename T>
        friend std::ostream& operator<<(std::ostream& os, const test_object obj);

        value_type get() {
            return m_val;

        };

    private:
        value_type m_val;

};

template <typename T>
std::ostream& operator<<(std::ostream& os, const test_object<T>& obj) {
    os << obj.get();
    return os;
}

int main(int argc, char** argv) {
    remote_var<test_object<std::uint8_t>> a(0x0);
    //test_object<std::uint8_t*> test((std::uint8_t*)5);

    a /= 5;

    return 0;
}
