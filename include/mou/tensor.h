#ifndef MOU_TENSOR_H
#define MOU_TENSOR_H

#include "lazy_eval.h"
#include <cassert>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <memory>
#include <numeric>
#include <ostream>
#include <utility>
#include <vector>

namespace mou {

namespace tensor {

template <typename DType>
class ShapeBase {
 public:
    template <typename... T>
    explicit ShapeBase(T ...args) {
        shape = {static_cast<DType>(args)...};
        len = (... * static_cast<DType>(args));
    }

    explicit ShapeBase() {
        shape = {};
        len = 0;
    }

    ShapeBase(std::initializer_list<DType> l) {
        shape = l;
        len = std::accumulate(shape.begin(), shape.end(), 1, std::multiplies<>());
    }

    DType operator [] (size_t i) const {
        return shape.at(i);
    }

    bool operator == (const ShapeBase& other) const {
        return shape == other.shape;
    }

    ShapeBase& operator = (std::initializer_list<DType> l) {
        shape = l;
        len = std::accumulate(shape.begin(), shape.end(), 1, std::multiplies<>());
        return *this;
    }

    inline DType Dims() const {
        return shape.size();
    }

    inline DType Size() const {
        return len;
    }

    inline DType Size(size_t i) const {
        return shape.at(i);
    }

    inline DType Size(size_t start, size_t end) const {
        DType sz = 1;
        for (size_t i = start; i < end; ++i) {
            sz *= shape.at(i);
        }
        return sz;
    }

    friend std::ostream& operator << (std::ostream& os, const ShapeBase& other) {
        os << '(';
        for (size_t i = 0; i < other.shape.size(); ++i) {
            os << other.shape.at(i) << ",)"[i==other.shape.size()-1];
        }
        return os;
    }

 private:
    std::vector<DType> shape;
    DType len;
};

using Shape = ShapeBase<size_t>;

template <typename DType, typename Allocator = std::allocator<DType> >
class Tensor : public expr::Exp<Tensor<DType> > {
 public:
    explicit Tensor(Shape shape) : shape(shape) {
        dptr = _M_allocate(shape.Size());
    }

    // TODO(Chenxia Han): Constructor with nested initializer_list
    Tensor(std::initializer_list<DType> l) {
        shape = {l.size()};
        dptr = _M_allocate_and_copy(l.begin(), l.end());
    }

    ~Tensor() {
        _M_deallocate(dptr, shape.Size());
    }

    Tensor(const Tensor& src) {
        shape = src.shape;
        dptr = _M_allocate_and_copy(src.dptr, src.dptr + src.shape.Size());
    }

    Tensor(Tensor&& src) noexcept {
        shape = std::move(src.shape);
        dptr = _M_allocate_on_move(src.dptr);
    }

    Tensor& operator = (const Tensor& src) {
        // TODO(Chenxia Han): Check shape in compile time
        if (shape == src.shape) {
            std::copy(src.dptr, src.dptr + src.shape.Size(), dptr);
        } else {
            _M_deallocate(dptr, shape.Size());
            shape = src.shape;
            dptr = _M_allocate_and_copy(src.dptr, src.dptr + src.shape.Size());
        }

        return *this;
    }

    Tensor& operator = (Tensor&& src) noexcept {
        _M_deallocate(dptr, shape.Size());
        shape = std::move(src.shape);
        dptr = _M_allocate_on_move(src.dptr);

        return *this;
    }

    Tensor& operator = (std::initializer_list<DType> l) {
        if (shape == Shape(l.size())) {
            std::copy(l.begin(), l.end(), dptr);
        } else {
            _M_deallocate(dptr, shape.Size());
            shape = {l.size()};
            dptr = _M_allocate_and_copy(l.begin(), l.end());
        }

        return *this;
    }

    DType operator [] (size_t i) const {
        return dptr[i];
    }

    template <typename EType>
    inline Tensor& operator = (const expr::Exp<EType> &src) {
        const EType &src_ = src.self();
        for (int i = 0; i < shape.Size(); ++i) {
            dptr[i] = src_.Eval(i);
        }
        return *this;
    }

    inline DType Eval(int i) const {
        return dptr[i];
    }

    inline Shape Shape_() const {
        return shape;
    }

    void Reshape(const Shape& src) {
        assert(shape.Size() == src.Size());
        shape = src;
    }

    template <typename... T>
    void Reshape(T ...args) {
        Shape src = Shape(args...);
        assert(shape.Size() == src.Size());
        shape = src;
    }

    template <typename T>
    void ReshapeLike(const Tensor<T>& other) {
        assert(shape.Size() == other.shape.Size());
        shape = other.shape;
    }

    friend std::ostream& operator << (std::ostream& os, const Tensor& other) {
        // TODO(Chenxia Han): Format output via dimensions
        os << '[';
        for (size_t i = 0; i < other.shape.Size(); ++i) {
            os << other[i] << ",]"[i==other.shape.Size()-1];
        }
        return os;
    }

 private:
    using pointer = typename std::allocator_traits<Allocator>::pointer;

    pointer dptr;
    Allocator alloc;
    Shape shape;

 public:
    using type = DType;

 private:
    pointer _M_allocate(size_t n) {
        using alloc_traits = std::allocator_traits<Allocator>;
        return n != 0 ? alloc_traits::allocate(alloc, n) : pointer();
    }

    void _M_deallocate(pointer p, size_t n) {
        using alloc_traits = std::allocator_traits<Allocator>;
        if (p) {
            alloc_traits::deallocate(alloc, p, n);
        }
    }

    template <typename ForwardIterator>
    pointer _M_allocate_and_copy(ForwardIterator first,
            ForwardIterator last) {
        auto n = last - first;
        pointer result = _M_allocate(n);
        try {
            std::uninitialized_copy(first, last, result);
            return result;
        } catch(...) {
            _M_deallocate(result, n);
            std::exception_ptr eptr = std::current_exception();
            std::rethrow_exception(eptr);
        }
    }

    template <typename ForwardIterator>
    pointer _M_allocate_on_move(ForwardIterator& first) {
        pointer result = first;
        first = nullptr;
        return result;
    }
};

}; // namespace tensor

}; // namespace mou

#endif // MOU_TENSOR_H
