
#include <iostream>
#include <cstdint>
#include <vector>

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
    virtual const value_type& read() = 0;
    virtual void write(const value_type&) = 0;

    /**
     * A class that can construct a temporary via read()
     * All modifications are saved via write() when the temporary destructs
     */
    template <typename U = value_type, typename = std::enable_if_t<std::is_class<value_type>::value>>
    class rw_guard : public value_type {
    public:
        rw_guard(proxy_var<value_type, Derived>* exterior) :
            U(exterior->read()), m_exterior(exterior) {
        };

        virtual ~rw_guard() {
            m_exterior->write(static_cast<const U>(*this)); 
        };

        value_type* operator->() {
            return static_cast<U*>(this);
        }

        proxy_var<value_type, Derived>* m_exterior = nullptr;
    };

    template <typename U = value_type, typename = std::enable_if_t<std::is_class<value_type>::value>>
    rw_guard<value_type> operator->() {
        return rw_guard<value_type>(this);
    }


private:
    Derived* ref() {
        return static_cast<Derived*>(this);
    }
public:

    operator value_type() {
        return read();
    }

#define OP(op)                                                      \
    template <typename ArgsR>                                       \
    Derived& operator op (ArgsR&& value) {   \
        value_type cur = this->read();                              \
        cur op value;                                               \
        this->write(cur);                                           \
        return *ref();                                              \
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

#define OP(op)                                              \
    bool operator op (const Derived& other) {   \
        return (this->read() op other.read());              \
    }                                                       \
    template <typename ArgsR>                               \
    bool operator op (ArgsR&& other) {                      \
        return (this->read() op other);                     \
    }

    OP(&&);
    OP(||);
    OP(<=);
    OP(>=);
    OP(==);
    OP(!=);
    OP(<);
    OP(>);
#undef OP

    //typedef typename std::remove_pointer<value_type>::type value_type_deref;

    // template <class U = value_type, typename = std::enable_if_t<std::is_pointer<U>::value>
    
    template <typename U = value_type>
    auto operator!() -> decltype(std::declval<U>().operator!()) {
        return !(this->read());
    }

    template <typename U = value_type>
    auto operator--(int) -> decltype(std::declval<U>().operator--(1)) {
        auto val = read();
        auto ret = val--;
        write(val);
        return ret;
    }

    template <typename U = value_type>
    auto operator--() -> decltype(std::declval<U>().operator--(), std::declval<Derived&>()) {
        auto val = read();
        --val;
        write(val);
        return *ref();
    }


    template <typename U = value_type>
    auto operator++(int) -> decltype(std::declval<U>().operator++(1)) {
        auto val = read();
        auto ret = val++;
        write(val);
        return ret;
    }

    template <typename U = value_type>
    auto operator++() -> decltype(std::declval<U>().operator++(), std::declval<Derived&>()) {
        auto val = read();
        ++val;
        write(val);
        return *ref();
    }
};


template <typename T>
class remote_var : public proxy_var<T, remote_var<T>> {
    public:
        remote_var(std::uintptr_t offset) :
            m_offset(offset) {
            
        };

        const T& read() {
            std::cout << "read called -> " << temp << std::endl;
            return temp;
        }

        void write(const T& value) {
            temp = value;
            std::cout << "write called -> " << temp << std::endl;
        }

    private:
        std::uintptr_t m_offset;
        T temp;
};

template <typename value_type>
class test_object {
    public:
        test_object() {
            m_val = 0;
        }

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

    private:
        value_type m_val;

};

template <typename T>
std::ostream& operator<<(std::ostream& os, const test_object<T>& obj) {
    os << (std::uint32_t)obj.get();
    return os;
}

int main(int argc, char** argv) {
    remote_var<test_object<std::uint8_t>> a(0x0);
    //test_object<std::uint8_t*> test((std::uint8_t*)5);

    a += 5;
    auto b = a--;
    --a;
    std::cout << a->get() << std::endl;

    return 0;
}
