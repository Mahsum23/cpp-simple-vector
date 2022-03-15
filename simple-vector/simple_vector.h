#pragma once

#include <initializer_list>
#include <stdexcept>
#include <utility>

#include "array_ptr.h"


class ReserveProxyObj
{
public:
    ReserveProxyObj() = delete;
    ReserveProxyObj(size_t capacity)
        : cap_(capacity)
    {}
	
    std::size_t GetCapacity()
    {
        return cap_;
    }
	
private:
    std::size_t cap_ = 0;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : data_(size), size_(size), capacity_(size)
    {
        for (size_t i = 0; i < size; ++i)
        {
            data_[i] = Type();
        }
    }

    explicit SimpleVector(ReserveProxyObj obj)
    {
        Reserve(obj.GetCapacity());
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : data_(size)
    {
        for (size_t i = 0; i < size; ++i)
        {
            data_[i] = value;
        }
        size_ = size;
        capacity_ = size;
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : data_(init.size())
    {
        for (auto it = init.begin(); it != init.end(); ++it)
        {
            data_[size_++] = *it;
        }
        capacity_ = size_;
    }

    SimpleVector(const SimpleVector& other)
        : data_(other.capacity_), size_(other.size_), capacity_(other.capacity_)
    {
        std::copy(other.begin(), other.end(), this->begin());
    }

    SimpleVector(SimpleVector&& other)
    {
        data_.swap(other.data_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

    SimpleVector& operator=(SimpleVector&& other)
    {
        if (this == &other)
            return *this;
        data_.swap(other.data_);
        other.data_.Release();
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
        return *this;
    }

    SimpleVector& operator=(const SimpleVector& other)
    {
        if (&other == this)
            return *this;
        SimpleVector tmp(other);
        swap(tmp);
        return *this;
    }

    void Reserve(std::size_t ncap)
    {
        if (ncap > capacity_)
        {
            capacity_ = ncap;
            SimpleVector<Type> tmp = SimpleVector(capacity_);
            std::move(this->begin(), this->end(), tmp.begin());
            data_.swap(tmp.data_);
        }
    }

    void swap(SimpleVector& other) noexcept
    {
        data_.swap(other.data_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
    }

    void PushBack(Type value)
    {
        if (capacity_ == 0)
        {
            this->Resize(1);
            data_[size_ - 1] = std::move(value);
            return;
        }
        if ((size_ + 1) <= capacity_)
        {
            data_[size_] = std::move(value);
            ++size_;
            return;
        }
        std::size_t prev_size = size_;
        Resize(capacity_ * 2);
        data_[prev_size] = std::move(value);
        size_ = prev_size + 1;
    }

    void PopBack() noexcept
    {
        if (size_ == 0)
            return;
        --size_;
    }

    Iterator Insert(ConstIterator pos, const Type& value)
    {
        if (capacity_ == 0)
        {
            PushBack(std::move(value));
            return begin();
        }
        auto index = pos - this->begin();
        if ((size_ + 1) > capacity_)
        {
            std::size_t prev_size = size_;
            this->Resize(capacity_ * 2);
            size_ = prev_size;
            std::move_backward(Iterator(this->begin() + index), this->end(), Iterator(this->end() + 1));
            data_[index] = std::move(value);
            ++size_;
        }
        else
        {
            std::copy_backward(Iterator(pos), this->end(), Iterator(this->end() + 1));
            data_[index] = std::move(value);
            ++size_;
        }
        return Iterator(this->begin() + index);
    }

    Iterator Insert(ConstIterator pos, Type&& value)
    {
        if (capacity_ == 0)
        {
            PushBack(std::move(value));
            return begin();
        }
        auto index = pos - this->begin();
        if ((size_ + 1) > capacity_)
        {
            std::size_t prev_size = size_;
            this->Resize(capacity_ * 2);
            size_ = prev_size;
            std::move_backward(Iterator(this->begin() + index), this->end(), Iterator(this->end() + 1));
            data_[index] = std::move(value);
            ++size_;
        }
        else
        {
            std::move_backward(Iterator(pos), this->end(), Iterator(this->end() + 1));
            data_[index] = std::move(value);
            ++size_;
        }
        return Iterator(this->begin() + index);
    }

    Iterator Erase(ConstIterator pos)
    {
        std::move(Iterator(pos + 1), this->end(), const_cast<Iterator>(pos));
        --size_;
        return const_cast<Iterator>(pos);
    }

    // Возвращает количество элементов в массиве
    std::size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    std::size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return (size_ == 0);
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_)
        {
            throw std::out_of_range("");
        }
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_)
        {
            throw std::out_of_range("");
        }
        return data_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_)
        {
            size_ = new_size;
            return;
        }
        if (new_size <= capacity_)
        {
            for (auto it = begin() + size_; it != begin() + new_size; ++it)
            {
                *it = std::move(Type());
            }
            //std::fill(this->begin() + size_, this->begin() + new_size, Type());
            size_ = new_size;
        }
        else
        {
            capacity_ = std::max(new_size, capacity_ * 2);
            SimpleVector<Type> tmp = SimpleVector(capacity_);
            std::move(this->begin(), this->end(), tmp.begin());
            data_.swap(tmp.data_);
            size_ = new_size;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return data_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return data_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return data_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return data_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return data_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return data_.Get() + size_;
    }
	
private:
    ArrayPtr<Type> data_;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return (lhs < rhs) || (lhs == rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return !(lhs < rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return (lhs >= rhs) && (lhs != rhs);
}

ReserveProxyObj Reserve(std::size_t ncap)
{
    return ReserveProxyObj(ncap);
}