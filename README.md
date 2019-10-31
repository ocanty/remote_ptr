# remote_ptr
remote_ptr provides a smart pointer to access structs and classes in other address spaces (including other processes & files). Both applications must be relatively ABI compatible for this to work

To use it you need to define a read and write function, these will be called whenever reading/writing needs to be done. i.e. when you finish modifying a struct
An example of read/write functions found below are used to modify structs in another Windows process:
```
HANDLE get_handle() {
    static HANDLE hProcess = INVALID_HANDLE_VALUE;
    if(!hProcess) {
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, some_pid);
    }
    
    return hProcess;
}

void read_remote(std::uintptr_t address, std::uint8_t* buffer, std::size_t n) {
    if(!ReadProcessMemory(get_handle(), (LPCVOID)address, (LPVOID)buffer, n, NULL)) {
        throw new some_read_exception();
    }
}

void write_remote(std::uintptr_t address, std::uint8_t* buffer, std::size_t n) {
    if(!WriteProcessMemory(get_handle(), (LPCVOID)address, (LPVOID)buffer, n, NULL)) {
        throw new some_write_exception();
    }
}
```

After these are defined you can begin targeting structs at whatever addresses you please. The majority of operator overloads are implemented and can be used if the struct implements them.

```
#include <cmath>
#include <iostream>
#include "remote_ptr.hpp"

// Our remote object, you can do #pragma pack operations here if you wish
// and struct packing will be enforced
class SomeVector3f {
public:
    SomeVector3f(float a, float b, float c) :
        a(a), b(b), c(c) { };
        
    float magnitude() {
        return sqrt((a*a)+(b*b)+(c*c));
    };
    
    void half() {
        a /= 2; b /= 2; c /= 2;
    };
        
    float a, b, c;
};

// Alias the type for easier usage
template <typename T>
using ptr = remote_ptr<T, read_remote, write_remote>;

int main(int argc, char** argv) {
    // Place exception handlers within here for 
    // any exceptions you throw inside your read/write functions

    // Access a pointer at address 0x4000500
    ptr<SomeVector3f> vec(0x4000500);
    
    // Modifications are made
    vec->a = 5;
    vec->b = 6;
    
    *vec = { 0.5f, 2.5f, 3f };
    
    std::cout << vec->magnitude() << std::endl;
    
    vec->half();
    
    std::cout << vec->magnitude() << std::endl;
    
    // You can also access any primitive types
    ptr<int> some_int(0x41233);
    std::cout << *some_int << std::endl;
    
    (*some_int) *= 2;
    (*some_int) += 5;
    
    std::cout << *some_int << std::endl;
    
    // Pointer dereferencing is supported
    // (This means you could walk a linked list remotely!)
    ptr<int*> some_pointer(0x41233);
    
    // Dereference once to access remotely, dereference twice to dereference remote pointer
    std::cout << *(*some_pointer) << std::endl;
    
    return 0;
}
```

# How It Works

Whenever the pointer is dereferenced, a temporary is returned. In the constructor of the temporary, the value is read and written back in the destructor. All modifications are performed on the temporary

SFINAE is used to create operator overloads on class types so any variations in avaialble overloads are supported.








```
