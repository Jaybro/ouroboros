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
}

TEST(CyclicDequeTest, ConstructorIterators) {
  std::vector<std::size_t> data(10);
  ouroboros::cyclic_deque cdeque(data.begin(), data.end());
  ExpectCapacityAndSize(cdeque, data.size(), 0);
}

TEST(CyclicDequeTest, ConstructorIteratorsSize) {
  std::size_t occupied = 4;
  std::vector<std::size_t> data(10);
  ouroboros::cyclic_deque cdeque(data.begin(), data.end(), occupied);
  ExpectCapacityAndSize(cdeque, data.size(), occupied);
}

TEST(CyclicDequeTest, ConstructorArray) {
  std::size_t data[10];
  ouroboros::cyclic_deque cdeque(data);
  ExpectCapacityAndSize(cdeque, std::size(data), 0);
}

TEST(CyclicDequeTest, ConstructorArraySize) {
  std::size_t occupied = 4;
  std::size_t data[10];
  ouroboros::cyclic_deque cdeque(data, occupied);
  ExpectCapacityAndSize(cdeque, std::size(data), occupied);
}

TEST(CyclicDequeTest, ConstructorStdArray) {
  std::array<std::size_t, 10> data;
  ouroboros::cyclic_deque cdeque(data);
  ExpectCapacityAndSize(cdeque, std::size(data), 0);
}

TEST(CyclicDequeTest, ConstructorStdArraySize) {
  std::size_t occupied = 4;
  std::array<std::size_t, 10> data;
  ouroboros::cyclic_deque cdeque(data, occupied);
  ExpectCapacityAndSize(cdeque, std::size(data), occupied);
}

TEST(CyclicDequeTest, FullEmptyClear) {
  std::size_t occupied = 8;
  std::vector<std::size_t> data(occupied);
  ouroboros::cyclic_deque cdeque(data.begin(), data.end(), occupied);
  EXPECT_TRUE(cdeque.full());
  cdeque.clear();
  EXPECT_TRUE(cdeque.empty());
  EXPECT_EQ(cdeque.begin(), cdeque.end());
}

TEST(CyclicDequeTest, At) {
  ouroboros::cyclic_deque<std::size_t> cdeque;
  EXPECT_THROW(cdeque.at(1), std::out_of_range);
}

TEST(CyclicDequeTest, LiFoBack) {
  std::vector<std::size_t> buffer(3);
  ouroboros::cyclic_deque cdeque(buffer.begin(), buffer.end());

  for (std::size_t i = 0; i < cdeque.capacity(); ++i) {
    cdeque.push_back(i + 1);
    EXPECT_EQ(cdeque.back(), i + 1);
  }
  EXPECT_EQ(cdeque.size(), buffer.size());
  EXPECT_FALSE(cdeque.empty());
  EXPECT_TRUE(cdeque.full());
  EXPECT_EQ(cdeque.front(), 1);
  for (std::size_t i = 0; i < buffer.size(); ++i) {
    EXPECT_EQ(cdeque[i], i + 1);
  }

  for (std::size_t i = 0; i < buffer.size(); ++i) {
    cdeque.pop_back();
  }
  EXPECT_EQ(cdeque.size(), 0);
  EXPECT_TRUE(cdeque.empty());
  EXPECT_FALSE(cdeque.full());
}

TEST(CyclicDequeTest, LiFoFront) {
  std::vector<std::size_t> buffer(3);
  ouroboros::cyclic_deque<std::size_t> cdeque(buffer.begin(), buffer.end());

  for (std::size_t i = 0; i < cdeque.capacity(); ++i) {
    cdeque.push_front(i + 1);
    EXPECT_EQ(cdeque.front(), i + 1);
  }
  EXPECT_EQ(cdeque.size(), buffer.size());
  EXPECT_FALSE(cdeque.empty());
  EXPECT_TRUE(cdeque.full());
  EXPECT_EQ(cdeque.back(), static_cast<float>(1));
  for (std::size_t i = 0; i < buffer.size(); ++i) {
    EXPECT_EQ(cdeque[buffer.size() - i - 1], i + 1);
  }

  for (std::size_t i = 0; i < buffer.size(); ++i) {
    cdeque.pop_front();
  }
  EXPECT_EQ(cdeque.size(), 0);
  EXPECT_TRUE(cdeque.empty());
  EXPECT_FALSE(cdeque.full());
}

TEST(CyclicDequeTest, FiFoBackInserter) {
  std::vector<std::size_t> buffer(3);
  ouroboros::cyclic_deque cdeque(buffer.begin(), buffer.end());

  for (std::size_t i = 0; i < cdeque.capacity(); ++i) {
    cdeque.push_back(i + 1);
  }
  cdeque.pop_front();
  cdeque.push_back(cdeque.capacity() + 1);
  for (std::size_t i = 0; i < buffer.size(); ++i) {
    EXPECT_EQ(cdeque[i], i + 2);
  }
  EXPECT_EQ(cdeque.size(), buffer.size());
  EXPECT_FALSE(cdeque.empty());
  EXPECT_TRUE(cdeque.full());
}

TEST(CyclicDequeTest, FiFoFrontInserter) {
  std::vector<std::size_t> buffer(3);
  ouroboros::cyclic_deque cdeque(buffer.begin(), buffer.end());

  for (std::size_t i = 0; i < cdeque.capacity(); ++i) {
    cdeque.push_front(i + 1);
  }
  cdeque.pop_back();
  cdeque.push_front(cdeque.capacity() + 1);
  for (std::size_t i = 0; i < buffer.size(); ++i) {
    EXPECT_EQ(cdeque[buffer.size() - i - 1], i + 2);
  }
  EXPECT_EQ(cdeque.size(), buffer.size());
  EXPECT_FALSE(cdeque.empty());
  EXPECT_TRUE(cdeque.full());
}

TEST(CyclicDequeTest, Iterator) {
  std::vector<std::size_t> buffer(4);
  ouroboros::cyclic_deque cdeque(buffer.begin(), buffer.end());
  cdeque.push_back(39);
  cdeque.push_back(40);
  cdeque.push_back(41);
  cdeque.push_back(42);
  cdeque.pop_front();
  cdeque.pop_front();
  cdeque.push_back(43);
  cdeque.push_back(44);

  // The cyclic_deque is non-owning. Therefore, there is no practical difference
  // between a non-const and const cyclic_deque.
  auto const& const_cdeque = cdeque;

  // Forward iteration, reverse iteration.
  {
    auto fit = const_cdeque.begin();
    auto rit = const_cdeque.rbegin();
    for (std::size_t i = 0; i < const_cdeque.size(); ++i, ++fit, ++rit) {
      EXPECT_EQ(*fit, const_cdeque[i]);
      EXPECT_EQ(*rit, const_cdeque[const_cdeque.size() - i - 1]);
    }
    EXPECT_EQ(fit, const_cdeque.end());
    EXPECT_EQ(rit, const_cdeque.rend());
  }

  // Positive indexing.
  {
    auto it = const_cdeque.begin();
    for (std::size_t i = 0; i < const_cdeque.size(); ++i) {
      EXPECT_EQ(it[i], const_cdeque[i]);
      EXPECT_EQ(*(it + i), const_cdeque[i]);
      EXPECT_EQ(*(i + it), const_cdeque[i]);
    }
  }

  // Negative indexing.
  {
    auto it = --const_cdeque.end();
    for (std::size_t i = 0; i < const_cdeque.size(); ++i) {
      auto si = static_cast<std::ptrdiff_t>(i);
      EXPECT_EQ(it[-si], const_cdeque[const_cdeque.size() - i - 1]);
      EXPECT_EQ(*(it - i), const_cdeque[const_cdeque.size() - i - 1]);
      EXPECT_EQ(*(i - it), const_cdeque[const_cdeque.size() - i - 1]);
    }
  }

  // Various equalities.
  {
    EXPECT_EQ(
        const_cdeque.end() - const_cdeque.begin(),
        static_cast<std::ptrdiff_t>(const_cdeque.size()));
    EXPECT_TRUE(const_cdeque.end() > const_cdeque.begin());
    EXPECT_TRUE(const_cdeque.end() >= const_cdeque.begin());
    EXPECT_TRUE(const_cdeque.end() >= const_cdeque.end());
    EXPECT_TRUE(const_cdeque.end() <= const_cdeque.end());
    EXPECT_FALSE(const_cdeque.end() <= const_cdeque.begin());
    EXPECT_FALSE(const_cdeque.end() < const_cdeque.begin());
  }

  // Const iterator.
  {
    for (auto& e : cdeque) {
      e = 0;
    }

    // Conversion from non-const to const iterator.
    ouroboros::cyclic_deque<std::size_t>::const_iterator cit = cdeque.begin();
    for (; cit != cdeque.cend(); ++cit) {
      EXPECT_EQ(*cit, 0);
    }
  }
}

TEST(CyclicDequeTest, AppendRange) {
  std::vector<std::size_t> v(16, 42);
  std::vector<std::size_t> r{2, 3, 4, 5, 6, 7, 8, 9};

  ouroboros::cyclic_deque cdeque(v.begin(), v.end());
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
  std::vector<std::size_t> v(16, 42);
  std::vector<std::size_t> r{0, 1, 2, 3, 4, 5, 6, 7};

  ouroboros::cyclic_deque cdeque(v.begin(), v.end());
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
  std::vector<Evil> v(4);
  std::vector<Evil> r(2);

  std::size_t initial_size = 2;
  ouroboros::cyclic_deque cdeque(v.begin(), v.end(), initial_size);

  Evil singleton, tabs;
  EXPECT_THROW(cdeque.push_back(singleton), std::runtime_error);
  EXPECT_THROW(cdeque.push_back(std::move(singleton)), std::runtime_error);
  EXPECT_THROW(cdeque.push_front(tabs), std::runtime_error);
  EXPECT_THROW(cdeque.push_front(std::move(tabs)), std::runtime_error);
  EXPECT_THROW(cdeque.append_range(r), std::runtime_error);
  EXPECT_THROW(cdeque.prepend_range(r), std::runtime_error);
  EXPECT_EQ(cdeque.size(), initial_size);
  EXPECT_EQ(&cdeque[0], &v[0]);
  EXPECT_EQ(&cdeque[initial_size - 1], &v[initial_size - 1]);
}
