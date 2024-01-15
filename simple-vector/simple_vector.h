#pragma once

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <initializer_list>
#include <iostream>
#include <utility>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity_to_reserve) :capacity_(capacity_to_reserve) {}
    size_t GetCapacity() {
        return capacity_;
    }
private:
    size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept :elements_(), size_(0), capacity_(0) {}

    SimpleVector(ReserveProxyObj reserve_obj) noexcept :elements_(reserve_obj.GetCapacity()), size_(0), capacity_(reserve_obj.GetCapacity()) {}

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) :elements_(size), size_(size), capacity_(size) {
        std::fill(begin(), end(), Type());
    }


    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) :elements_(size), size_(size), capacity_(size) {
        if (size == 0) {
            return;
        }
        std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) :elements_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector(const SimpleVector& other) :elements_(other.size_) {
        size_ = other.size_;
        capacity_ = other.capacity_;

        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector rhs_copy(rhs);
            swap(rhs_copy);
        }
        return *this;
    }

    SimpleVector(SimpleVector&& other) {
        swap(std::move(other));
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        if (this != &rhs) {
            SimpleVector rhs_copy(rhs);
            swap(std::move(rhs_copy));
        }
        return *this;
    }

    void PushBack(const Type& item) {
        Resize(size_ + 1);
        elements_[size_ - 1] = item;
    }

    void PushBack(Type&& item) {
        Resize(size_ + 1);
        elements_[size_ - 1] = std::move(item);
    }

    void PopBack() noexcept {
        assert(size_ != 0u);
        size_--;
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        int n = std::distance(cbegin(), pos);
        if (pos == end()) {
            PushBack(value);
            return &elements_[n];
        }

        Resize(size_ + 1);
        std::copy_backward(&elements_[n], &elements_[size_ - 1], &elements_[size_]);
        elements_[n] = value;
        return &elements_[n];
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        int n = std::distance(cbegin(), pos);
        if (pos == end()) {
            PushBack(std::move(value));
            return &elements_[n];
        }

        Resize(size_ + 1);
        std::copy_backward(std::make_move_iterator(&elements_[n]), std::make_move_iterator(&elements_[size_ - 1]), &elements_[size_]);
        elements_[n] = std::move(value);
        return &elements_[n];
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        Iterator p = begin() + std::distance(cbegin(), pos);
        std::copy(std::make_move_iterator(p + 1), std::make_move_iterator(end()), p);
        Resize(size_ - 1);
        return p;
    }

    void swap(SimpleVector& other) noexcept {
        if (this == &other) {
            return;
        }
        elements_.swap(other.elements_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void swap(SimpleVector&& other) noexcept {
        if (this == &other) {
            return;
        }
        elements_ = std::move(other.elements_);
        size_ = std::move(other.size_);
        capacity_ = std::move(other.capacity_);

        other.size_ = 0;
        other.capacity_ = 0;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        // Напишите тело самостоятельно
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return elements_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return elements_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("");
        }
        return elements_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("");
        }
        return elements_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size == size_) {
            return;
        }

        else if (new_size < size_) {
            size_ = new_size;
            return;
        }

        if (new_size <= capacity_) {
            for (size_t it = size_; it != new_size; it++) {
                elements_[it] = std::move(Type());
            }
            size_ = new_size;
            return;
        }

        size_t new_capacity = std::max(new_size, 2 * size_);
        Type* new_array = new Type[new_capacity];
        ArrayPtr<Type> tmp(new_array);

        for (size_t i = 0; i < size_; i++) {
            new_array[i] = std::move(elements_[i]);
        }
        for (size_t it = size_; it != new_size; it++) {
            new_array[it] = std::move(Type());
        }

        elements_.swap(tmp);
        size_ = new_size;
        capacity_ = new_capacity;

    }

    void Reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }

        Type* new_array = new Type[new_capacity];
        ArrayPtr<Type> tmp(new_array);
        for (size_t i = 0; i < size_; i++) {
            new_array[i] = elements_[i];
        }
        elements_.swap(tmp);
        capacity_ = new_capacity;
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return elements_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return elements_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return elements_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return elements_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return elements_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return elements_.Get() + size_;
    }
private:
    ArrayPtr<Type> elements_;
    size_t size_;
    size_t capacity_;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());;
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}


