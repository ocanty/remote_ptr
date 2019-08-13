#include <iostream>
#include <vector>


template <typename T>
concept bool OperatorPlusEqual() {
    return requires(T& a, T b) {
        { a += b } -> T&;
    };
};


template <typename value_type, typename Derived>
class proxy_var_base {
public:
    template <typename... Args>
    proxy_var_base(Args&&... val) :
        m_val(std::forward<Args>(val)...) {

    }

private:
    Derived& derived_this() {
        return static_cast<Derived&>(*this);
    }

public:
    template <OperatorPlusEqual T>
    Derived& operator+=(T other) {
        std::cout << "wut face" << std::endl;
        before_operation(m_val);
        m_val += other;
        after_operation(m_val);

        return derived_this();
    }

    


    operator value_type() {
        return m_val;
    };


    virtual void before_operation(value_type& val) { };
    virtual void after_operation(value_type& val) { };

private:
    value_type m_val;
};


class proxy_int : public proxy_var_base<int, proxy_int> {
    public:
        proxy_int(int a) :
            proxy_var_base<int,proxy_int>(a) {

        }

        virtual void before_operation(int& val) {
            std::cout << "pre operation " << val << std::endl;
        }

        virtual void after_operation(int& val) {
            std::cout << "after operation " << val << std::endl;
        }
};



int main(int argc, char** argv) {
    proxy_int test = 5;

    std::cout << "proxy var testing " << std::endl;

    test += 2.0f;

    // std::cout << test << std::endl;
    
    return 0;
}