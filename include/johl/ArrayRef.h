#pragma once

namespace johl
{
  template<typename T>
  class ArrayRef
  {
  public:
    using iterator = T*;
    using const_iterator = const T*;

    ArrayRef()
      : m_array(nullptr)
      , m_size(0)
    {
    }

    ArrayRef(T* array, size_t size)
      : m_array(array)
      , m_size(size)
    {
    }

    ArrayRef(const ArrayRef&) = default;

    ArrayRef& operator=(const ArrayRef&) = default;

    ~ArrayRef() = default;

    T& operator[](size_t i)
    {
      return m_array[i];
    }

    const T& operator[](size_t i) const
    {
      return m_array[i];
    }

    size_t size() const
    {
      return m_size;
    }

    iterator begin()
    {
      return &m_array[0];
    }

    const_iterator begin() const
    {
      return &m_array[0];
    }

    iterator end()
    {
      return &m_array[m_size];
    }
    const_iterator end() const
    {
      return &m_array[m_size];
    }

  private:
    T* m_array;
    size_t m_size;
  };
}