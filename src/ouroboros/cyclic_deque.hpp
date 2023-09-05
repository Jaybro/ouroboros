#pragma once

#include <cassert>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace ouroboros {

namespace internal {

template <typename Iterator_>
using value_type_t = typename std::iterator_traits<Iterator_>::value_type;

//! \brief Wrap \p index from the expected input range of
//! [start...finish+(finish-start)) between [start...finish).
template <typename Index_>
constexpr Index_ wrap_cycle(
    Index_ index, Index_ start, Index_ finish) noexcept {
  if (index >= finish) {
    return index - finish + start;
  } else {
    return index;
  }
}

//! \brief Increment \p index within the cyclic range [start...finish).
template <typename Index_>
constexpr Index_ inc_cycle(Index_ index, Index_ start, Index_ finish) noexcept {
  ++index;
  if (index == finish) {
    return start;
  } else {
    return index;
  }
}

//! \brief Decrement \p index within the cyclic range [start...finish).
template <typename Index_>
constexpr Index_ dec_cycle(Index_ index, Index_ start, Index_ finish) noexcept {
  if (index == start) {
    return finish - 1;
  } else {
    return index - 1;
  }
}

// Views have shallow const, meaning that the iterator returned by
// range.begin() const does not have to be a const_iterator.
// NOTE: This library does not currently provide a cyclic_deque_view.
template <typename Range_>
struct range_traits {
  using range = Range_ const;
  using maybe_const_reference = decltype(*std::declval<range>().begin())&;
  using maybe_const_iterator = decltype(std::declval<range>().begin());
};

struct size_from_container_tag {};

template <typename Container_>
class cyclic_deque_impl {
  using container = Container_;

 public:
  using size_type = typename container::size_type;
  using difference_type = typename container::difference_type;
  using value_type = typename container::value_type;
  using pointer = typename container::pointer;
  using const_pointer = typename container::const_pointer;
  using reference = typename container::reference;
  using const_reference = typename container::const_reference;

  using iterator = typename container::iterator;
  using const_iterator = typename container::const_iterator;

  using maybe_const_reference =
      typename range_traits<Container_>::maybe_const_reference;
  using maybe_const_iterator =
      typename range_traits<Container_>::maybe_const_iterator;

  constexpr cyclic_deque_impl() noexcept
      : buf(), deq_start(), deq_finish(), deq_size() {}

  constexpr cyclic_deque_impl(container b) noexcept
      : buf(std::move(b)),
        deq_start(buf.begin()),
        deq_finish(buf.begin()),
        deq_size() {}

  constexpr cyclic_deque_impl(container b, size_type n) noexcept
      : buf(std::move(b)),
        deq_start(buf.begin()),
        deq_finish(wrap_cycle(deq_start + n)),
        deq_size(n) {
    assert(n <= capacity());
  }

  constexpr cyclic_deque_impl(container b, size_from_container_tag) noexcept
      : buf(std::move(b)),
        deq_start(buf.begin()),
        deq_finish(buf.begin()),
        deq_size(capacity()) {}

  //! \brief Wrap \p index from range [buf.begin()...buf.begin()+2n) to range
  //! [buf.begin()...buf.begin()+n), where n equals buf.end()-buf.begin().
  constexpr iterator wrap_cycle(iterator index) noexcept {
    return internal::wrap_cycle(index, buf.begin(), buf.end());
  }

  //! \copydoc wrap_cycle(iterator)
  constexpr maybe_const_iterator wrap_cycle(
      maybe_const_iterator index) const noexcept {
    return internal::wrap_cycle(index, buf.begin(), buf.end());
  }

  //! \brief Convert an inner address [0...size) to an outer address that falls
  //! within the cyclic range [deq_start...deq_finish).
  template <typename Index_>
  constexpr iterator inner_to_outer(Index_ i) noexcept {
    return wrap_cycle(deq_start + i);
  }

  //! \copydoc inner_to_outer(Index_)
  template <typename Index_>
  constexpr maybe_const_iterator inner_to_outer(Index_ i) const noexcept {
    return wrap_cycle(deq_start + i);
  }

  constexpr iterator inc_cycle(iterator index) noexcept {
    return internal::inc_cycle(index, buf.begin(), buf.end());
  }

  constexpr maybe_const_iterator inc_cycle(
      maybe_const_iterator index) const noexcept {
    return internal::inc_cycle(index, buf.begin(), buf.end());
  }

  constexpr iterator dec_cycle(iterator index) noexcept {
    return internal::dec_cycle(index, buf.begin(), buf.end());
  }

  constexpr maybe_const_iterator dec_cycle(
      maybe_const_iterator index) const noexcept {
    return internal::dec_cycle(index, buf.begin(), buf.end());
  }

 private:
  constexpr void throw_if_out_of_range(size_type i) const {
    if (i >= size()) {
      throw std::out_of_range(
          "cyclic_deque_impl::at: i (which is " + std::to_string(i) +
          ") >= this->size() (which is " + std::to_string(size()) + ")");
    }
  }

 public:
  //! \brief Return a reference to the specified element at \p i, with bounds
  //! checking.
  constexpr reference at(size_type i) {
    throw_if_out_of_range(i);
    return *inner_to_outer(i);
  }

  //! \copydoc at(size_type)
  constexpr maybe_const_reference at(size_type i) const {
    throw_if_out_of_range(i);
    return *inner_to_outer(i);
  }

  //! \brief Return a reference to an element using subscript access.
  //! \details Undefined behavior if the index is out of bounds.
  constexpr reference operator[](size_type i) noexcept {
    return *inner_to_outer(i);
  }

  //! \copydoc operator[](size_type)
  constexpr maybe_const_reference operator[](size_type i) const noexcept {
    return *inner_to_outer(i);
  }

  constexpr reference front() noexcept { return *deq_start; }

  constexpr maybe_const_reference front() const noexcept { return *deq_start; }

  constexpr reference back() noexcept { return *dec_cycle(deq_finish); }

  constexpr maybe_const_reference back() const noexcept {
    return *dec_cycle(deq_finish);
  }

 private:
  constexpr void set_value(value_type const& input, value_type& output) const {
    output = input;
  }

  constexpr void set_value(value_type&& input, value_type& output) const {
    output = std::move(input);
  }

 public:
  template <typename U_>
  constexpr void push_back(U_&& value) {
    assert(!full());
    // Strong exception safety: The internal state is only updated after calling
    // set_value, just in case it throws.
    set_value(std::forward<U_>(value), *deq_finish);
    // Increase the size by incrementing the deq_finish index.
    deq_finish = inc_cycle(deq_finish);
    ++deq_size;
  }

  //! \details This method only updates an index and a counter, making it unable
  //! to throw an exception. This is unlike pop_back() for an std::vector<>.
  constexpr void pop_back() noexcept {
    assert(!empty());
    // Decrease the size by decrementing the deq_finish index.
    deq_finish = dec_cycle(deq_finish);
    --deq_size;
  }

  template <typename U_>
  constexpr void push_front(U_&& value) {
    assert(!full());
    iterator dec_deq_start = dec_cycle(deq_start);
    // Strong exception safety: The internal state is only updated after calling
    // set_value, just in case it throws.
    set_value(std::forward<U_>(value), *dec_deq_start);
    // Increase the size by decrementing the deq_start index.
    deq_start = dec_deq_start;
    ++deq_size;
  }

  //! \see pop_back
  constexpr void pop_front() noexcept {
    assert(!empty());
    // Decrease the size by incrementing the deq_start index.
    deq_start = inc_cycle(deq_start);
    --deq_size;
  }

  //! \brief Append a copy of the elements of range \p rg to the contents of the
  //! cyclic_deque. Undefined behavior if available() is not sufficient to
  //! accomodate the range.
  template <typename Range_>
  constexpr void append_range(Range_&& rg) {
    // Use std::ranges::end(), etc., with C++20 or higher.
    auto rg_size = std::distance(std::begin(rg), std::end(rg));
    assert(static_cast<size_type>(rg_size) <= available());
    // size1 can never be 0 because buf.end() is one past-the-last, always
    // resulting in a split with work.
    auto size1 = buf.end() - deq_finish;
    if (rg_size <= size1) {
      deq_finish = std::copy(rg.begin(), rg.end(), deq_finish);
    } else {
      auto split = std::next(rg.begin(), size1);
      std::copy(rg.begin(), split, deq_finish);
      deq_finish = std::copy(split, rg.end(), buf.begin());
    }
    deq_size += rg_size;
  }

  //! \brief Prepend a copy of the elements of range \p rg to the contents of
  //! the cyclic_deque. Undefined behavior if available() is not sufficient to
  //! accomodate the range.
  template <typename Range_>
  constexpr void prepend_range(Range_&& rg) {
    auto rg_size = std::distance(std::begin(rg), std::end(rg));
    assert(static_cast<size_type>(rg_size) <= available());
    // The cyclic_deque is empty when deq_start and buf.begin() are equal (it
    // can also be full, but that's treated as undefined behaviour). An empty
    // size would cause the copy to be split into a full size and zero size
    // copy. Because we don't want a 0 size copy, we move deq_start to buf.end()
    // when deq_start equals buf.begin() with the operation below. It results in
    // an unchanged address otherwise.
    iterator dec_deq_start = dec_cycle(deq_start) + 1;
    auto size2 = dec_deq_start - buf.begin();
    if (rg_size <= size2) {
      dec_deq_start -= rg_size;
      std::copy(rg.begin(), rg.end(), dec_deq_start);
    } else {
      auto split = std::prev(rg.end(), size2);
      std::copy(split, rg.end(), buf.begin());
      dec_deq_start = buf.end() - rg_size + size2;
      std::copy(rg.begin(), split, dec_deq_start);
    }
    deq_start = dec_deq_start;
    deq_size += rg_size;
  }

  constexpr size_type capacity() const noexcept {
    return static_cast<size_type>(buf.end() - buf.begin());
  }

  constexpr size_type size() const noexcept { return deq_size; }

  constexpr size_type available() const noexcept { return capacity() - size(); }

  constexpr bool empty() const noexcept { return deq_size == 0; }

  constexpr bool full() const noexcept { return deq_size == capacity(); }

  constexpr void clear() noexcept {
    deq_start = buf.begin();
    deq_finish = deq_start;
    deq_size = 0;
  }

  constexpr void resize(size_type n) noexcept {
    assert(n <= capacity());
    // 0 =< s <= capacity()
    auto s = static_cast<difference_type>(deq_size);
    // -capacity() =< d <= capacity()
    auto d = static_cast<difference_type>(n) - s;

    if (d < 0) {
      // Our wrapping function doesn't accept negative numbers so the negative
      // step is wrapped to a positive one.
      // deq_finish:
      //  [buf.begin()...buf.end())
      // d + capacity():
      //  [0...capacity())
      // Summing them keeps us within the accepted range of wrap_cycle.
      deq_finish =
          wrap_cycle(deq_finish + d + static_cast<difference_type>(capacity()));
    } else {
      deq_finish = wrap_cycle(deq_finish + d);
    }
    deq_size = static_cast<size_type>(s + d);
  }

  container buf;
  iterator deq_start;
  //! \brief One past-the-last element for the cycle. The value for deq_finish
  //! is cyclic. Meaning that when the array range is either empty or full,
  //! deq_start equals deq_finish. Or, when the cyclic_deque is not full, and
  //! deq_start equals 0, then deq_finish equals size.
  iterator deq_finish;
  size_type deq_size;
};

template <typename Data_, bool Const_>
struct cyclic_deque_data_traits {
  using pointer = typename Data_::pointer;
  using reference = typename Data_::reference;
};

template <typename Data_>
struct cyclic_deque_data_traits<Data_, true> {
  using pointer = typename Data_::const_pointer;
  using reference = typename Data_::const_reference;
};

template <typename Data_, bool Const_>
class cyclic_deque_iterator {
  using cyclic_data = Data_;
  using cyclic_data_traits = cyclic_deque_data_traits<Data_, Const_>;

 public:
  using size_type = typename cyclic_data::size_type;
  using difference_type = typename cyclic_data::difference_type;
  using value_type = typename cyclic_data::value_type;
  using pointer = typename cyclic_data_traits::pointer;
  using reference = typename cyclic_data_traits::reference;
  using iterator_category = std::random_access_iterator_tag;

  constexpr cyclic_deque_iterator() noexcept = default;

  constexpr cyclic_deque_iterator(
      cyclic_data* data, difference_type index) noexcept
      : data_(data), index_(index) {}

  constexpr reference operator*() const noexcept {
    return *data_->inner_to_outer(index_);
  }

  constexpr pointer operator->() const noexcept {
    return data_->inner_to_outer(index_);
  }

  //! \public Prefix increment.
  constexpr cyclic_deque_iterator& operator++() noexcept {
    ++index_;
    return *this;
  }

  //! \private Postfix increment.
  constexpr cyclic_deque_iterator operator++(int) noexcept {
    return ++cyclic_deque_iterator(*this);
  }

  //! \private Prefix decrement.
  constexpr cyclic_deque_iterator& operator--() noexcept {
    --index_;
    return *this;
  }

  //! \private Postfix decrement.
  constexpr cyclic_deque_iterator operator--(int) noexcept {
    return --cyclic_deque_iterator(*this);
  }

  constexpr cyclic_deque_iterator& operator+=(difference_type n) noexcept {
    index_ += n;
    return *this;
  }

  constexpr cyclic_deque_iterator& operator-=(difference_type n) noexcept {
    index_ -= n;
    return *this;
  }

  constexpr friend cyclic_deque_iterator operator+(
      cyclic_deque_iterator const& a, difference_type n) noexcept {
    return cyclic_deque_iterator(a) += n;
  }

  constexpr friend cyclic_deque_iterator operator+(
      difference_type n, cyclic_deque_iterator const& a) noexcept {
    return cyclic_deque_iterator(a) += n;
  }

  constexpr friend cyclic_deque_iterator operator-(
      cyclic_deque_iterator const& a, difference_type n) noexcept {
    return cyclic_deque_iterator(a) -= n;
  }

  constexpr friend cyclic_deque_iterator operator-(
      difference_type n, cyclic_deque_iterator const& a) noexcept {
    return cyclic_deque_iterator(a) -= n;
  }

  //! \brief The input is expected to be within the range of (-2n...2*n), where
  //! n equals buf.end()-buf.begin(). Undefined behavior otherwise.
  constexpr reference operator[](difference_type i) const noexcept {
    return *data_->inner_to_outer(index_ + i);
  }

  //! \brief iterator to const_iterator conversion.
  template <bool C_ = Const_, std::enable_if_t<!C_, int> = 0>
  constexpr operator cyclic_deque_iterator<cyclic_data const, !C_>()
      const noexcept {
    return {data_, index_};
  }

  constexpr difference_type const& base() const noexcept { return index_; }

 private:
  cyclic_data* data_;
  //! \brief The value for index_ is expected to fall within the range
  //! [0...size) at all times. Undefined behavior otherwise.
  difference_type index_;
};

// Forward iterator
template <typename DataL_, bool ConstL_, typename DataR_, bool ConstR_>
constexpr bool operator==(
    cyclic_deque_iterator<DataL_, ConstL_> const& a,
    cyclic_deque_iterator<DataR_, ConstR_> const& b) noexcept {
  return a.base() == b.base();
}

template <typename DataL_, bool ConstL_, typename DataR_, bool ConstR_>
constexpr bool operator!=(
    cyclic_deque_iterator<DataL_, ConstL_> const& a,
    cyclic_deque_iterator<DataR_, ConstR_> const& b) noexcept {
  return a.base() != b.base();
}

// Random access iterator
template <typename DataL_, bool ConstL_, typename DataR_, bool ConstR_>
constexpr auto operator-(
    cyclic_deque_iterator<DataL_, ConstL_> const& a,
    cyclic_deque_iterator<DataR_, ConstR_> const& b) noexcept {
  return a.base() - b.base();
}

template <typename DataL_, bool ConstL_, typename DataR_, bool ConstR_>
constexpr bool operator>(
    cyclic_deque_iterator<DataL_, ConstL_> const& a,
    cyclic_deque_iterator<DataR_, ConstR_> const& b) noexcept {
  return a.base() > b.base();
}

template <typename DataL_, bool ConstL_, typename DataR_, bool ConstR_>
constexpr bool operator<(
    cyclic_deque_iterator<DataL_, ConstL_> const& a,
    cyclic_deque_iterator<DataR_, ConstR_> const& b) noexcept {
  return a.base() < b.base();
}

template <typename DataL_, bool ConstL_, typename DataR_, bool ConstR_>
constexpr bool operator>=(
    cyclic_deque_iterator<DataL_, ConstL_> const& a,
    cyclic_deque_iterator<DataR_, ConstR_> const& b) noexcept {
  return a.base() >= b.base();
}

template <typename DataL_, bool ConstL_, typename DataR_, bool ConstR_>
constexpr bool operator<=(
    cyclic_deque_iterator<DataL_, ConstL_> const& a,
    cyclic_deque_iterator<DataR_, ConstR_> const& b) noexcept {
  return a.base() <= b.base();
}

}  // namespace internal

template <typename T_, typename Allocator_ = std::allocator<T_>>
class cyclic_deque {
  static_assert(
      std::is_same_v<std::remove_cv_t<T_>, T_>,
      "ouroboros::cyclic_deque must have a non-const, non-volatile value_type");

  using container = std::vector<T_, Allocator_>;
  using cyclic_impl = internal::cyclic_deque_impl<container>;

 public:
  using size_type = typename container::size_type;
  using difference_type = typename container::difference_type;
  using value_type = typename container::value_type;
  using pointer = typename container::pointer;
  using const_pointer = typename container::const_pointer;
  using reference = typename container::reference;
  using const_reference = typename container::const_reference;

  using iterator = internal::cyclic_deque_iterator<cyclic_impl, false>;
  using const_iterator =
      internal::cyclic_deque_iterator<cyclic_impl const, true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  constexpr cyclic_deque() noexcept(noexcept(Allocator_())) = default;

  constexpr explicit cyclic_deque(Allocator_ const& a) noexcept
      : impl_(container(a)) {}

  constexpr explicit cyclic_deque(
      size_type c, Allocator_ const& a = Allocator_())
      : impl_(container(c, a)) {}

  constexpr cyclic_deque(
      size_type c, size_type s, Allocator_ const& a = Allocator_())
      : impl_(container(c, a), s) {}

  template <class InputIterator_>
  constexpr cyclic_deque(
      InputIterator_ f, InputIterator_ l, const Allocator_& a = Allocator_())
      : impl_(container(f, l, a), internal::size_from_container_tag()) {}

  constexpr cyclic_deque(
      std::initializer_list<T_> i, const Allocator_& a = Allocator_())
      : impl_(container(i, a), internal::size_from_container_tag()) {}

  //! \brief Return a reference to the specified element at \p i, with bounds
  //! checking.
  constexpr reference at(size_type i) { return impl_.at(i); }

  constexpr const_reference at(size_type i) const { return impl_.at(i); }

  //! \brief Return a reference to an element using subscript access.
  //! \details Undefined behavior if the index is out of bounds.
  constexpr reference operator[](size_type i) noexcept { return impl_[i]; }

  constexpr const_reference operator[](size_type i) const noexcept {
    return impl_[i];
  }

  //! \brief Return a reference to the first element of the cyclic_deque.
  //! \details Undefined behavior if the cyclic_deque is empty.
  constexpr reference front() noexcept { return impl_.front(); }

  constexpr const_reference front() const noexcept { return impl_.front(); }

  //! \brief Return a reference to the last element of the cyclic_deque.
  //! \details Undefined behavior if the cyclic_deque is empty.
  constexpr reference back() noexcept { return impl_.back(); }

  constexpr const_reference back() const noexcept { return impl_.back(); }

  //! \brief Add an element to the end of the cyclic_deque.
  //! \details Undefined behavior if the cyclic_deque is full.
  constexpr void push_back(value_type const& value) { impl_.push_back(value); }

  //! \brief Add an element to the end of the cyclic_deque.
  //! \details Undefined behavior if the cyclic_deque is full.
  constexpr void push_back(value_type&& value) {
    impl_.push_back(std::move(value));
  }

  //! \brief Remove last element.
  //! \details Undefined behavior if the cyclic_deque is empty.
  constexpr void pop_back() noexcept { impl_.pop_back(); }

  //! \brief Add an element to the begin of the cyclic_deque.
  //! \details Undefined behavior if the cyclic_deque is full.
  constexpr void push_front(value_type const& value) {
    impl_.push_front(value);
  }

  //! \brief Add an element to the begin of the cyclic_deque.
  //! \details Undefined behavior if the cyclic_deque is full.
  constexpr void push_front(value_type&& value) {
    impl_.push_front(std::move(value));
  }

  //! \brief Remove first element.
  //! \details Undefined behavior if the cyclic_deque is empty.
  constexpr void pop_front() noexcept { impl_.pop_front(); }

  //! \brief Append a copy of the elements of range \p rg to the contents of the
  //! cyclic_deque. Undefined behavior if available() is not sufficient to
  //! accomodate the range.
  template <typename Range_>
  constexpr void append_range(Range_&& rg) {
    impl_.append_range(std::forward<Range_>(rg));
  }

  //! \brief Prepend a copy of the elements of range \p rg to the contents of
  //! the cyclic_deque. Undefined behavior if available() is not sufficient to
  //! accomodate the range.
  template <typename Range_>
  constexpr void prepend_range(Range_&& rg) {
    impl_.prepend_range(std::forward<Range_>(rg));
  }

  //! \brief Erase all elements.
  constexpr void clear() noexcept { impl_.clear(); }

  //! \brief Change the number of stored elements.
  constexpr void resize(size_type n) noexcept { impl_.resize(n); }

  //! \brief Return the maximum number of elements the cyclic_deque can hold.
  constexpr size_type capacity() const noexcept { return impl_.capacity(); }

  //! \brief Return the number of elements in the cyclic_deque.
  constexpr size_type size() const noexcept { return impl_.size(); }

  //! \brief Return the number of elements that can be inserted before the
  //! cyclic_deque is full. I.e., the unoccupied capacity, capapcity() - size().
  constexpr size_type available() const noexcept { return impl_.available(); }

  constexpr bool empty() const noexcept { return impl_.empty(); }

  constexpr bool full() const noexcept { return impl_.full(); }

  constexpr iterator begin() noexcept {
    return iterator(&impl_, difference_type(0));
  }

  constexpr const_iterator cbegin() const noexcept {
    return const_iterator(&impl_, difference_type(0));
  }

  constexpr const_iterator begin() const noexcept { return cbegin(); }

  constexpr iterator end() noexcept {
    return iterator(&impl_, static_cast<difference_type>(size()));
  }

  constexpr const_iterator cend() const noexcept {
    return const_iterator(&impl_, static_cast<difference_type>(size()));
  }

  constexpr const_iterator end() const noexcept { return cend(); }

  constexpr reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }

  constexpr const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(cend());
  }

  constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }

  constexpr reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }

  constexpr const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(cbegin());
  }

  constexpr const_reverse_iterator rend() const noexcept { return crend(); }

 private:
  cyclic_impl impl_;
};

template <
    typename InputIterator_,
    typename Allocator_ = std::allocator<
        typename std::iterator_traits<InputIterator_>::value_type>>
cyclic_deque(InputIterator_, InputIterator_, Allocator_ = Allocator_())
    -> cyclic_deque<
        typename std::iterator_traits<InputIterator_>::value_type,
        Allocator_>;

}  // namespace ouroboros
