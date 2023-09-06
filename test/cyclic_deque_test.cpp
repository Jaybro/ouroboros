#include <gtest/gtest.h>

#include <ouroboros/cyclic_deque.hpp>

namespace {

template <typename T_>
void ExpectCapacityAndSize(
    ouroboros::cyclic_deque<T_> const& cdeque,
    std::size_t capacity,
    std::size_t size) {
  EXPECT_EQ(cdeque.capacity(), capacity);
  EXPECT_EQ(cdeque.size(), size);
  EXPECT_EQ(cdeque.available(), cdeque.capacity() - size);
  EXPECT_EQ(cdeque.empty(), size == 0);
  EXPECT_EQ(cdeque.full(), size == capacity);
}

}  // namespace

TEST(CyclicDequeTest, ConstructorDefault) {
  // Note that it will be both empty and full at the same time.
  ouroboros::cyclic_deque<std::size_t> cdeque;
  ExpectCapacityAndSize(cdeque, 0, 0);

  static_assert(
      std::is_nothrow_constructible_v<ouroboros::cyclic_deque<std::size_t>>);

  static_assert(std::is_nothrow_constructible_v<
                ouroboros::cyclic_deque<std::size_t>,
                std::allocator<std::size_t>>);
}

TEST(CyclicDequeTest, ConstructorCapacity) {
  std::size_t capacity = 10;
  ouroboros::cyclic_deque<std::size_t> cdeque(capacity);
  ExpectCapacityAndSize(cdeque, capacity, 0);
}

TEST(CyclicDequeTest, ConstructorCapacitySize) {
  std::size_t capacity = 10;
  std::size_t size = 4;
  ouroboros::cyclic_deque<std::size_t> cdeque(capacity, size);
  ExpectCapacityAndSize(cdeque, capacity, size);
}

TEST(CyclicDequeTest, ConstructorIterators) {
  std::vector<std::size_t> v(10, 42);
  ouroboros::cyclic_deque cdeque(v.begin(), v.end());
  ExpectCapacityAndSize(cdeque, v.size(), v.size());
}

TEST(CyclicDequeTest, ConstructorInitializerList) {
  ouroboros::cyclic_deque cdeque({42.0f, 42.0f, 42.0f, 42.0f});
  ExpectCapacityAndSize(cdeque, 4, 4);
}

TEST(CyclicDequeTest, FullEmptyClear) {
  std::size_t capacity = 8;
  ouroboros::cyclic_deque<std::size_t> cdeque(capacity, capacity);
  EXPECT_TRUE(cdeque.full());
  cdeque.clear();
  EXPECT_TRUE(cdeque.empty());
  EXPECT_EQ(cdeque.begin(), cdeque.end());
}

TEST(CyclicDequeTest, Resize) {
  std::size_t capacity = 10;
  // deq_start and deq_finish are equal in this test. deq_finish shouldn't
  // become < deq_start.
  {
    ouroboros::cyclic_deque<std::size_t> cdeque(capacity, capacity);
    std::size_t size_four = 4;
    cdeque.resize(size_four);
    ExpectCapacityAndSize(cdeque, capacity, size_four);
    EXPECT_EQ(&cdeque[size_four - 1], &cdeque.back());
  }

  // See the back() address rotates properly with an increment. This doesn't
  // happen if the deq_finish >= buffer.end(). Increments don't wrap once they
  // are out of the buffer range [buf.begin()...buf.end()).
  {
    ouroboros::cyclic_deque<std::size_t> cdeque(capacity);
    cdeque.resize(capacity);
    ExpectCapacityAndSize(cdeque, capacity, capacity);
    auto ptr_0 = &cdeque[0];
    cdeque.pop_front();
    cdeque.push_back(0);
    EXPECT_EQ(ptr_0, &cdeque.back());
  }
}

TEST(CyclicDequeTest, At) {
  ouroboros::cyclic_deque<std::size_t> cdeque(1, 1);
  EXPECT_EQ(cdeque.at(0), cdeque[0]);
  EXPECT_THROW(cdeque.at(1), std::out_of_range);
}

TEST(CyclicDequeTest, MaxSize) {
  ouroboros::cyclic_deque<std::size_t> cdeque(42);
  EXPECT_EQ(cdeque.max_size(), cdeque.capacity());
}

TEST(CyclicDequeTest, LiFoBack) {
  std::size_t capacity = 3;
  ouroboros::cyclic_deque<std::size_t> cdeque(capacity);

  for (std::size_t i = 0; i < cdeque.capacity(); ++i) {
    cdeque.push_back(i + 1);
    EXPECT_EQ(cdeque.back(), i + 1);
  }
  EXPECT_EQ(cdeque.size(), capacity);
  EXPECT_FALSE(cdeque.empty());
  EXPECT_TRUE(cdeque.full());
  EXPECT_EQ(cdeque.front(), 1);
  for (std::size_t i = 0; i < capacity; ++i) {
    EXPECT_EQ(cdeque[i], i + 1);
  }

  for (std::size_t i = 0; i < capacity; ++i) {
    cdeque.pop_back();
  }
  EXPECT_EQ(cdeque.size(), 0);
  EXPECT_TRUE(cdeque.empty());
  EXPECT_FALSE(cdeque.full());
}

TEST(CyclicDequeTest, LiFoFront) {
  std::size_t capacity = 3;
  ouroboros::cyclic_deque<std::size_t> cdeque(capacity);

  for (std::size_t i = 0; i < cdeque.capacity(); ++i) {
    cdeque.push_front(i + 1);
    EXPECT_EQ(cdeque.front(), i + 1);
  }
  EXPECT_EQ(cdeque.size(), capacity);
  EXPECT_FALSE(cdeque.empty());
  EXPECT_TRUE(cdeque.full());
  EXPECT_EQ(cdeque.back(), static_cast<float>(1));
  for (std::size_t i = 0; i < capacity; ++i) {
    EXPECT_EQ(cdeque[capacity - i - 1], i + 1);
  }

  for (std::size_t i = 0; i < capacity; ++i) {
    cdeque.pop_front();
  }
  EXPECT_EQ(cdeque.size(), 0);
  EXPECT_TRUE(cdeque.empty());
  EXPECT_FALSE(cdeque.full());
}

TEST(CyclicDequeTest, FiFoBackInserter) {
  std::size_t capacity = 3;
  ouroboros::cyclic_deque<std::size_t> cdeque(capacity);

  for (std::size_t i = 0; i < cdeque.capacity(); ++i) {
    cdeque.push_back(i + 1);
  }
  cdeque.pop_front();
  cdeque.push_back(cdeque.capacity() + 1);
  for (std::size_t i = 0; i < capacity; ++i) {
    EXPECT_EQ(cdeque[i], i + 2);
  }
  EXPECT_EQ(cdeque.size(), capacity);
  EXPECT_FALSE(cdeque.empty());
  EXPECT_TRUE(cdeque.full());
}

TEST(CyclicDequeTest, FiFoFrontInserter) {
  std::size_t capacity = 3;
  ouroboros::cyclic_deque<std::size_t> cdeque(capacity);

  for (std::size_t i = 0; i < cdeque.capacity(); ++i) {
    cdeque.push_front(i + 1);
  }
  cdeque.pop_back();
  cdeque.push_front(cdeque.capacity() + 1);
  for (std::size_t i = 0; i < capacity; ++i) {
    EXPECT_EQ(cdeque[capacity - i - 1], i + 2);
  }
  EXPECT_EQ(cdeque.size(), capacity);
  EXPECT_FALSE(cdeque.empty());
  EXPECT_TRUE(cdeque.full());
}

namespace {

template <typename T_>
void ExpectRandomAccessIterator(T_& cdeque) {
  // Forward iteration, reverse iteration.
  {
    auto fit = cdeque.begin();
    auto rit = cdeque.rbegin();
    for (std::size_t i = 0; i < cdeque.size(); ++i, ++fit, ++rit) {
      EXPECT_EQ(*fit, cdeque[i]);
      EXPECT_EQ(*rit, cdeque[cdeque.size() - i - 1]);
    }
    EXPECT_EQ(fit, cdeque.end());
    EXPECT_EQ(rit, cdeque.rend());
  }

  // Positive indexing.
  {
    auto it = cdeque.begin();
    for (std::size_t i = 0; i < cdeque.size(); ++i) {
      EXPECT_EQ(it[i], cdeque[i]);
      EXPECT_EQ(*(it + i), cdeque[i]);
      EXPECT_EQ(*(i + it), cdeque[i]);
    }
  }

  // Negative indexing.
  {
    auto it = --cdeque.end();
    for (std::size_t i = 0; i < cdeque.size(); ++i) {
      auto si = static_cast<std::ptrdiff_t>(i);
      EXPECT_EQ(it[-si], cdeque[cdeque.size() - i - 1]);
      EXPECT_EQ(*(it - i), cdeque[cdeque.size() - i - 1]);
      EXPECT_EQ(*(i - it), cdeque[cdeque.size() - i - 1]);
    }
  }

  // Various equalities.
  {
    EXPECT_EQ(
        cdeque.end() - cdeque.begin(),
        static_cast<std::ptrdiff_t>(cdeque.size()));
    EXPECT_TRUE(cdeque.end() > cdeque.begin());
    EXPECT_TRUE(cdeque.end() >= cdeque.begin());
    EXPECT_TRUE(cdeque.end() >= cdeque.end());
    EXPECT_TRUE(cdeque.end() <= cdeque.end());
    EXPECT_FALSE(cdeque.end() <= cdeque.begin());
    EXPECT_FALSE(cdeque.end() < cdeque.begin());
  }
}

}  // namespace

TEST(CyclicDequeTest, Iterator) {
  std::size_t capacity = 4;
  ouroboros::cyclic_deque<std::size_t> cdeque(capacity);
  cdeque.push_back(39);
  cdeque.push_back(40);
  cdeque.push_back(41);
  cdeque.push_back(42);
  cdeque.pop_front();
  cdeque.pop_front();
  cdeque.push_back(43);
  cdeque.push_back(44);

  // Check both const and non-const iterators.
  auto const& const_cdeque = cdeque;
  ExpectRandomAccessIterator(cdeque);
  ExpectRandomAccessIterator(const_cdeque);

  // Conversion from non-const to const iterator.
  {
    for (auto& e : cdeque) {
      e = 0;
    }

    ouroboros::cyclic_deque<std::size_t>::const_iterator cit = cdeque.begin();
    for (; cit != cdeque.cend(); ++cit) {
      EXPECT_EQ(*cit, 0);
    }
  }
}

TEST(CyclicDequeTest, AppendRange) {
  std::vector<std::size_t> r{2, 3, 4, 5, 6, 7, 8, 9};

  ouroboros::cyclic_deque<std::size_t> cdeque(16);
  cdeque.push_back(0);
  cdeque.push_back(1);
  cdeque.append_range(r);
  EXPECT_EQ(cdeque.size(), r.size() + 2);
  for (std::size_t i = 0; i < cdeque.size(); ++i) {
    EXPECT_EQ(cdeque[i], i);
  }
  cdeque.pop_front();
  cdeque.pop_front();
  cdeque.pop_front();
  cdeque.pop_front();
  EXPECT_EQ(cdeque.size(), r.size() - 2);
  cdeque.append_range(r);
  for (std::size_t i = 0; i < 6; ++i) {
    EXPECT_EQ(cdeque[i], i + 4);
  }
  for (std::size_t i = 6; i < r.size() + 6; ++i) {
    EXPECT_EQ(cdeque[i], i - 4);
  }
  EXPECT_EQ(cdeque.size(), r.size() * 2 - 2);
}

TEST(CyclicDequeTest, PrependRange) {
  std::vector<std::size_t> r{0, 1, 2, 3, 4, 5, 6, 7};

  ouroboros::cyclic_deque<std::size_t> cdeque(16);
  cdeque.push_front(9);
  cdeque.push_front(8);
  cdeque.prepend_range(r);
  EXPECT_EQ(cdeque.size(), r.size() + 2);
  for (std::size_t i = 0; i < cdeque.size(); ++i) {
    EXPECT_EQ(cdeque[i], i);
  }
  cdeque.pop_back();
  cdeque.pop_back();
  cdeque.pop_back();
  cdeque.pop_back();
  EXPECT_EQ(cdeque.size(), r.size() - 2);
  cdeque.prepend_range(r);
  for (std::size_t i = 0; i < r.size(); ++i) {
    EXPECT_EQ(cdeque[i], i);
  }
  for (std::size_t i = r.size(); i < r.size() + 6; ++i) {
    EXPECT_EQ(cdeque[i], i - r.size());
  }
  EXPECT_EQ(cdeque.size(), r.size() * 2 - 2);
}

// This class may lead to "unreachable code" warnings. Likely because the lines
// of code that follow after a throw will never be reached.
struct Evil {
  Evil() = default;  // presage
  Evil(Evil const&) { throw std::runtime_error("malice"); }
  Evil(Evil&&) { throw std::runtime_error("malice"); }
  Evil& operator=(Evil const&) { throw std::runtime_error("malice"); }
  Evil& operator=(Evil&&) { throw std::runtime_error("malice"); }
  ~Evil() = default;  // serenity

  std::byte noise;
};

// An exception shouldn't change the state of the deque except for perhaps the
// contents of the available() part of the capacity(). The latter may happen
// when the copy itself is not strongly exception safe, overwriting, perhaps
// partially, an object.
TEST(CyclicDequeTest, StrongExceptionSafety) {
  std::vector<Evil> r(2);

  std::size_t initial_size = 2;
  ouroboros::cyclic_deque<Evil> cdeque(4, initial_size);
  auto ptr_0 = &cdeque[0];
  auto ptr_N = &cdeque[initial_size - 1];

  Evil singleton, tabs;
  EXPECT_THROW(cdeque.push_back(singleton), std::runtime_error);
  EXPECT_THROW(cdeque.push_back(std::move(singleton)), std::runtime_error);
  EXPECT_THROW(cdeque.push_front(tabs), std::runtime_error);
  EXPECT_THROW(cdeque.push_front(std::move(tabs)), std::runtime_error);
  EXPECT_THROW(cdeque.append_range(r), std::runtime_error);
  EXPECT_THROW(cdeque.prepend_range(r), std::runtime_error);
  EXPECT_EQ(cdeque.size(), initial_size);
  EXPECT_EQ(&cdeque[0], ptr_0);
  EXPECT_EQ(&cdeque[initial_size - 1], ptr_N);
}
