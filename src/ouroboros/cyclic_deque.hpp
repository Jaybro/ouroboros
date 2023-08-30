#pragma once

#include <cassert>
#include <iterator>
#include <type_traits>

namespace ouroboros {

namespace internal {

template <typename Iterator_>
struct element_type {
  using type = std::remove_reference_t<
      typename std::iterator_traits<Iterator_>::reference>;
};

template <typename Iterator_>
using element_type_t = typename element_type<Iterator_>::type;

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

template <typename T_>
struct element_type_traits {
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using element_type = T_;
  using value_type = std::remove_cv_t<T_>;
  using pointer = T_*;
  using const_pointer = T_ const*;
  using reference = T_&;
  using const_reference = T_ const&;
};

template <typename T_>
struct cyclic_deque_data {
  using size_type = typename element_type_traits<T_>::size_type;
  using pointer = typename element_type_traits<T_>::pointer;

  constexpr cyclic_deque_data() noexcept
      : mem_start(), mem_finish(), deq_start(), deq_finish(), deq_size() {}

  template <typename ContiguousIterator_>
  constexpr cyclic_deque_data(
      ContiguousIterator_ ms, ContiguousIterator_ mf) noexcept
      : mem_start(&(*ms)),
        mem_finish(&(*mf)),
        deq_start(mem_start),
        deq_finish(mem_start),
        deq_size() {}

  template <typename ContiguousIterator_>
  constexpr cyclic_deque_data(
      ContiguousIterator_ ms, ContiguousIterator_ mf, size_type n) noexcept
      : mem_start(&(*ms)),
        mem_finish(&(*mf)),
        deq_start(mem_start),
        deq_finish(internal::wrap_cycle(deq_start + n, mem_start, mem_finish)),
        deq_size(n) {
    assert(n <= capacity());
  }

  constexpr size_type capacity() const noexcept {
    return static_cast<size_type>(mem_finish - mem_start);
  }

  constexpr size_type size() const noexcept { return deq_size; }

  constexpr bool empty() const noexcept { return deq_size == 0; }

  constexpr bool full() const noexcept { return deq_size == capacity(); }

  constexpr void clear() noexcept {
    deq_start = mem_start;
    deq_finish = deq_start;
    deq_size = 0;
  }

  constexpr pointer inc_cycle(pointer index) const noexcept {
    return internal::inc_cycle(index, mem_start, mem_finish);
  }

  constexpr pointer dec_cycle(pointer index) const noexcept {
    return internal::dec_cycle(index, mem_start, mem_finish);
  }

  //! \brief Decrease the size by incrementing the start index.
  constexpr void inc_start() noexcept {
    deq_start = inc_cycle(deq_start);
    --deq_size;
  }

  //! \brief Increase the size by decrementing the start index.
  constexpr void dec_start() noexcept {
    deq_start = dec_cycle(deq_start);
    ++deq_size;
  }

  //! \brief Increase the size by incrementing the finish index.
  constexpr void inc_finish() noexcept {
    deq_finish = inc_cycle(deq_finish);
    ++deq_size;
  }

  //! \brief Decrease the size by decrementing the finish index.
  constexpr void dec_finish() noexcept {
    deq_finish = dec_cycle(deq_finish);
    --deq_size;
  }

  //! \brief Wrap \p index from range [mem_start...mem_start+2n) to range
  //! [mem_start...mem_start+n), where n equals mem_finish-mem_start.
  constexpr pointer wrap_cycle(pointer index) const noexcept {
    return internal::wrap_cycle(index, mem_start, mem_finish);
  }

  //! \brief Convert an inner address [0...size) to an outer address that falls
  //! within the cyclic range [deq_start...deq_finish).
  template <typename Index_>
  constexpr pointer inner_to_outer(Index_ i) const noexcept {
    return wrap_cycle(deq_start + i);
  }

  pointer mem_start;
  pointer mem_finish;
  pointer deq_start;
  //! \brief One past-the-last element for the cycle. The value for deq_finish
  //! is cyclic. Meaning that when the array range is either empty or full,
  //! deq_start equals deq_finish. Or, when the cyclic_deque is not full, and
  //! deq_start equals 0, then deq_finish equals size.
  pointer deq_finish;
  size_type deq_size;
};

template <typename T_, bool Const_>
class cyclic_deque_iterator {
  // NOTE: The two input template arguments can be reduced to a single template
  // argument using std::basic_const_iterator from C++23.
  using element_traits =
      element_type_traits<std::conditional_t<Const_, T_ const, T_>>;
  using cyclic_data = cyclic_deque_data<T_>;

 public:
  using size_type = typename element_traits::size_type;
  using difference_type = typename element_traits::difference_type;
  using value_type = typename element_traits::value_type;
  using pointer = typename element_traits::pointer;
  using reference = typename element_traits::reference;
  using iterator_category = std::random_access_iterator_tag;

  constexpr cyclic_deque_iterator() noexcept = default;

  constexpr cyclic_deque_iterator(
      cyclic_data const* data, difference_type index) noexcept
      : data_(data), index_(index) {}

  reference operator*() const { return *data_->inner_to_outer(index_); }

  pointer operator->() const { return data_->inner_to_outer(index_); }

  friend bool operator==(
      cyclic_deque_iterator const& a, cyclic_deque_iterator const& b) {
    assert(a.data_ == b.data_);
    return a.index_ == b.index_;
  }

  friend bool operator!=(
      cyclic_deque_iterator const& a, cyclic_deque_iterator const& b) {
    assert(a.data_ == b.data_);
    return a.index_ != b.index_;
  }

  friend bool operator>(
      cyclic_deque_iterator const& a, cyclic_deque_iterator const& b) {
    assert(a.data_ == b.data_);
    return a.index_ > b.index_;
  }

  friend bool operator<(
      cyclic_deque_iterator const& a, cyclic_deque_iterator const& b) {
    assert(a.data_ == b.data_);
    return a.index_ < b.index_;
  }

  friend bool operator>=(
      cyclic_deque_iterator const& a, cyclic_deque_iterator const& b) {
    assert(a.data_ == b.data_);
    return a.index_ >= b.index_;
  }

  friend bool operator<=(
      cyclic_deque_iterator const& a, cyclic_deque_iterator const& b) {
    assert(a.data_ == b.data_);
    return a.index_ <= b.index_;
  }

  //! \public Prefix increment.
  cyclic_deque_iterator& operator++() {
    ++index_;
    return *this;
  }

  //! \private Postfix increment.
  cyclic_deque_iterator operator++(int) {
    return ++cyclic_deque_iterator(*this);
  }

  //! \private Prefix decrement.
  cyclic_deque_iterator& operator--() {
    --index_;
    return *this;
  }

  //! \private Postfix decrement.
  cyclic_deque_iterator operator--(int) {
    return --cyclic_deque_iterator(*this);
  }

  cyclic_deque_iterator& operator+=(difference_type n) {
    index_ += n;
    return *this;
  }

  cyclic_deque_iterator& operator-=(difference_type n) {
    index_ -= n;
    return *this;
  }

  friend cyclic_deque_iterator operator+(
      cyclic_deque_iterator const& a, difference_type n) {
    return cyclic_deque_iterator(a) += n;
  }

  friend cyclic_deque_iterator operator+(
      difference_type n, cyclic_deque_iterator const& a) {
    return cyclic_deque_iterator(a) += n;
  }

  friend cyclic_deque_iterator operator-(
      cyclic_deque_iterator const& a, difference_type n) {
    return cyclic_deque_iterator(a) -= n;
  }

  friend cyclic_deque_iterator operator-(
      difference_type n, cyclic_deque_iterator const& a) {
    return cyclic_deque_iterator(a) -= n;
  }

  friend difference_type operator-(
      cyclic_deque_iterator const& a, cyclic_deque_iterator const& b) {
    assert(a.data_ == b.data_);
    return a.index_ - b.index_;
  }

  //! \brief The input is expected to be within the range of (-2n...2*n), where
  //! n equals mem_finish-mem_start. Undefined behavior otherwise.
  reference operator[](difference_type i) const {
    return *data_->inner_to_outer(index_ + i);
  }

  //! \brief iterator to const_iterator conversion.
  template <bool C_ = Const_, std::enable_if_t<!C_, int> = 0>
  operator cyclic_deque_iterator<T_, !C_>() const {
    return {data_, index_};
  }

 private:
  cyclic_data const* data_;
  //! \brief The value for index_ is expected to fall within the range
  //! [0...size) at all times. Undefined behavior otherwise.
  difference_type index_;
};

}  // namespace internal

template <typename T_>
class cyclic_deque {
  using cyclic_data = internal::cyclic_deque_data<T_>;
  using element_traits = internal::element_type_traits<T_>;

 public:
  using size_type = typename element_traits::size_type;
  using difference_type = typename element_traits::difference_type;
  using value_type = typename element_traits::value_type;
  using element_type = typename element_traits::element_type;
  using pointer = typename element_traits::pointer;
  using const_pointer = typename element_traits::const_pointer;
  using reference = typename element_traits::reference;
  using const_reference = typename element_traits::const_reference;

  using iterator = internal::cyclic_deque_iterator<element_type, false>;
  using const_iterator = internal::cyclic_deque_iterator<element_type, true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  constexpr cyclic_deque() noexcept = default;

  template <typename ContiguousIterator_>
  constexpr cyclic_deque(
      ContiguousIterator_ mem_start, ContiguousIterator_ mem_finish) noexcept
      : impl_(mem_start, mem_finish) {}

  template <typename ContiguousIterator_>
  constexpr cyclic_deque(
      ContiguousIterator_ mem_start,
      ContiguousIterator_ mem_finish,
      size_type n) noexcept
      : impl_(mem_start, mem_finish, n) {}

  //! \brief Return a reference to an element using subscript access.
  //! \details Undefined behavior if the index is out of bounds.
  constexpr reference operator[](size_type i) const noexcept {
    return *impl_.inner_to_outer(i);
  }

  //! \brief Return a reference to the first element of the cyclic_deque.
  //! \details Undefined behavior if the cyclic_deque is empty.
  constexpr reference front() const noexcept { return head(); }

  //! \brief Return a reference to the last element of the cyclic_deque.
  //! \details Undefined behavior if the cyclic_deque is empty.
  constexpr reference back() const noexcept { return tail(); }

  //! \brief Add an element to the end of the cyclic_deque.
  //! \details Undefined behavior if the cyclic_deque is full.
  constexpr void push_back(value_type const& value) {
    assert(!full());
    impl_.inc_finish();
    back() = value;
  }

  //! \brief Add an element to the end of the cyclic_deque.
  //! \details Undefined behavior if the cyclic_deque is full.
  constexpr void push_back(value_type&& value) {
    assert(!full());
    impl_.inc_finish();
    back() = std::move(value);
  }

  //! \brief Remove last element.
  //! \details Undefined behavior if the cyclic_deque is empty.
  constexpr void pop_back() {
    assert(!empty());
    impl_.dec_finish();
  }

  //! \brief Add an element to the begin of the cyclic_deque.
  //! \details Undefined behavior if the cyclic_deque is full.
  constexpr void push_front(value_type const& value) {
    assert(!full());
    impl_.dec_start();
    front() = value;
  }

  //! \brief Add an element to the begin of the cyclic_deque.
  //! \details Undefined behavior if the cyclic_deque is full.
  constexpr void push_front(value_type&& value) {
    assert(!full());
    impl_.dec_start();
    front() = std::move(value);
  }

  //! \brief Remove first element.
  //! \details Undefined behavior if the cyclic_deque is empty.
  constexpr void pop_front() {
    assert(!empty());
    impl_.inc_start();
  }

  //! \brief Erase all elements.
  constexpr void clear() noexcept { impl_.clear(); }

  //! \brief Append a copy of the elements of range \p rg to the contents of the
  //! cyclic_deque. Undefined behavior if available() is not sufficient to
  //! accomodate the range.
  template <typename Range_>
  constexpr void append_range(Range_&& rg) {
    // Use std::ranges::end(), etc., with C++20 or higher.
    auto rg_size = std::distance(std::begin(rg), std::end(rg));
    assert(static_cast<size_type>(rg_size) <= available());
    // size1 can never be 0 because mem_finish is one past-the-last, always
    // resulting in a split with work.
    auto size1 = impl_.mem_finish - impl_.deq_finish;
    if (rg_size <= size1) {
      impl_.deq_finish = std::copy(rg.begin(), rg.end(), impl_.deq_finish);
    } else {
      auto split = std::next(rg.begin(), size1);
      std::copy(rg.begin(), split, impl_.deq_finish);
      impl_.deq_finish = std::copy(split, rg.end(), impl_.mem_start);
    }
    impl_.deq_size += rg_size;
  }

  //! \brief Prepend a copy of the elements of range \p rg to the contents of
  //! the cyclic_deque. Undefined behavior if available() is not sufficient to
  //! accomodate the range.
  template <typename Range_>
  constexpr void prepend_range(Range_&& rg) {
    auto rg_size = std::distance(std::begin(rg), std::end(rg));
    assert(static_cast<size_type>(rg_size) <= available());
    // The cyclic_deque is empty when deq_start and mem_start are equal (it can
    // also be full, but that's treated as undefined behaviour). Because we
    // don't want a 0 size copy, we move deq_start to mem_finish when deq_start
    // equals mem_start with the operation below. It results in a no-op
    // otherwise.
    impl_.deq_start = impl_.dec_cycle(impl_.deq_start) + 1;
    auto size2 = impl_.deq_start - impl_.mem_start;
    if (rg_size <= size2) {
      impl_.deq_start -= rg_size;
      std::copy(rg.begin(), rg.end(), impl_.deq_start);
    } else {
      auto split = std::prev(rg.end(), size2);
      std::copy(split, rg.end(), impl_.mem_start);
      impl_.deq_start = impl_.mem_finish - rg_size + size2;
      std::copy(rg.begin(), split, impl_.deq_start);
    }
    impl_.deq_size += rg_size;
  }

  //! \brief Return the maximum number of elements the cyclic_deque can hold.
  constexpr size_type capacity() const noexcept { return impl_.capacity(); }

  //! \brief Return the number of elements in the cyclic_deque.
  constexpr size_type size() const noexcept { return impl_.size(); }

  //! \brief Return the number of elements that can be inserted before the
  //! cyclic_deque is full. I.e., the unoccupied capacity, capapcity() - size().
  constexpr size_type available() const noexcept { return capacity() - size(); }

  constexpr bool empty() const noexcept { return impl_.empty(); }

  constexpr bool full() const noexcept { return impl_.full(); }

  constexpr iterator begin() const noexcept {
    return iterator(&impl_, difference_type(0));
  }

  constexpr const_iterator cbegin() const noexcept {
    return const_iterator(&impl_, difference_type(0));
  }

  constexpr iterator end() const noexcept {
    return iterator(&impl_, static_cast<difference_type>(size()));
  }

  constexpr const_iterator cend() const noexcept {
    return const_iterator(&impl_, static_cast<difference_type>(size()));
  }

  constexpr reverse_iterator rbegin() const noexcept {
    return reverse_iterator(end());
  }

  constexpr const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(cend());
  }

  constexpr reverse_iterator rend() const noexcept {
    return reverse_iterator(begin());
  }

  constexpr const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(cbegin());
  }

 private:
  constexpr reference head() const noexcept { return *impl_.deq_start; }

  constexpr reference tail() const noexcept {
    return *impl_.dec_cycle(impl_.deq_finish);
  }

  cyclic_data impl_;
};

template <class ContiguousIterator_>
cyclic_deque(ContiguousIterator_ mem_start, ContiguousIterator_ mem_finish)
    -> cyclic_deque<internal::element_type_t<ContiguousIterator_>>;

template <class ContiguousIterator_>
cyclic_deque(
    ContiguousIterator_ mem_start,
    ContiguousIterator_ mem_finish,
    typename internal::element_type_traits<
        internal::element_type_t<ContiguousIterator_>>::size_type n)
    -> cyclic_deque<internal::element_type_t<ContiguousIterator_>>;

}  // namespace ouroboros
