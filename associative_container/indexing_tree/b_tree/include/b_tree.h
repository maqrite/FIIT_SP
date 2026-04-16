#ifndef SYS_PROG_B_TREE_H
#define SYS_PROG_B_TREE_H

#include "../../../include/associative_container.h"
#include "not_implemented.h"
#include "pp_allocator.h"
#include <boost/container/static_vector.hpp>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <stack>
#include <utility>

template <typename tkey, typename tvalue,
          comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class B_tree final : private compare // EBCO
{
public:
  using tree_data_type = std::pair<tkey, tvalue>;
  using tree_data_type_const = std::pair<const tkey, tvalue>;
  using value_type = tree_data_type_const;

private:
  static constexpr const size_t minimum_keys_in_node = t - 1;
  static constexpr const size_t maximum_keys_in_node = 2 * t - 1;

  // region comparators declaration

  inline bool compare_keys(const tkey &lhs, const tkey &rhs) const;
  inline bool compare_pairs(const tree_data_type &lhs,
                            const tree_data_type &rhs) const;

  // endregion comparators declaration

  struct btree_node {
    boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1>
        _keys;
    boost::container::static_vector<btree_node *, maximum_keys_in_node + 2>
        _pointers;
    btree_node() noexcept;
  };

  pp_allocator<value_type> _allocator;
  btree_node *_root;
  size_t _size;

  pp_allocator<value_type> get_allocator() const noexcept;

  using node_allocator_type = typename std::allocator_traits<
      pp_allocator<value_type>>::template rebind_alloc<btree_node>;
  using node_traits = std::allocator_traits<node_allocator_type>;

  void destroy_node(btree_node *node) noexcept {
    if (!node)
      return;

    for (auto *child_ptr : node->_pointers) {
      destroy_node(child_ptr);
    }

    node_allocator_type node_alloc{_allocator};
    node_traits::destroy(node_alloc, node);
    node_traits::deallocate(node_alloc, node, 1);
  }

  btree_node *create_node();

  void insert_into_node(btree_node *node, tree_data_type &&data,
                        btree_node *right_child = nullptr);

  btree_node *split_node(btree_node *node, tree_data_type &median_data);

public:
  // region constructors declaration

  explicit B_tree(const compare &cmp = compare(),
                  pp_allocator<value_type> = pp_allocator<value_type>());

  explicit B_tree(pp_allocator<value_type> alloc,
                  const compare &comp = compare());

  template <input_iterator_for_pair<tkey, tvalue> iterator>
  explicit B_tree(iterator begin, iterator end, const compare &cmp = compare(),
                  pp_allocator<value_type> = pp_allocator<value_type>());

  B_tree(std::initializer_list<std::pair<tkey, tvalue>> data,
         const compare &cmp = compare(),
         pp_allocator<value_type> = pp_allocator<value_type>());

  // endregion constructors declaration

  // region five declaration

  B_tree(const B_tree &other);

  B_tree(B_tree &&other) noexcept;

  B_tree &operator=(const B_tree &other);

  B_tree &operator=(B_tree &&other) noexcept;

  ~B_tree() noexcept;

  // endregion five declaration

  // region iterators declaration

  class btree_iterator;
  class btree_reverse_iterator;
  class btree_const_iterator;
  class btree_const_reverse_iterator;

  class btree_iterator final {
    std::stack<std::pair<btree_node **, size_t>> _path;
    size_t _index;

  public:
    using value_type = tree_data_type_const;
    using reference = value_type &;
    using pointer = value_type *;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = ptrdiff_t;
    using self = btree_iterator;

    friend class B_tree;
    friend class btree_reverse_iterator;
    friend class btree_const_iterator;
    friend class btree_const_reverse_iterator;

    reference operator*() const noexcept;
    pointer operator->() const noexcept;

    self &operator++();
    self operator++(int);

    self &operator--();
    self operator--(int);

    bool operator==(const self &other) const noexcept;
    bool operator!=(const self &other) const noexcept;

    size_t depth() const noexcept;
    size_t current_node_keys_count() const noexcept;
    bool is_terminate_node() const noexcept;
    size_t index() const noexcept;

    explicit btree_iterator(
        const std::stack<std::pair<btree_node **, size_t>> &path =
            std::stack<std::pair<btree_node **, size_t>>(),
        size_t index = 0);
  };

  class btree_const_iterator final {
    std::stack<std::pair<btree_node *const *, size_t>> _path;
    size_t _index;

  public:
    using value_type = tree_data_type_const;
    using reference = const value_type &;
    using pointer = const value_type *;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = ptrdiff_t;
    using self = btree_const_iterator;

    friend class B_tree;
    friend class btree_reverse_iterator;
    friend class btree_iterator;
    friend class btree_const_reverse_iterator;

    btree_const_iterator(const btree_iterator &it) noexcept;

    reference operator*() const noexcept;
    pointer operator->() const noexcept;

    self &operator++();
    self operator++(int);

    self &operator--();
    self operator--(int);

    bool operator==(const self &other) const noexcept;
    bool operator!=(const self &other) const noexcept;

    size_t depth() const noexcept;
    size_t current_node_keys_count() const noexcept;
    bool is_terminate_node() const noexcept;
    size_t index() const noexcept;

    explicit btree_const_iterator(
        const std::stack<std::pair<btree_node *const *, size_t>> &path =
            std::stack<std::pair<btree_node *const *, size_t>>(),
        size_t index = 0);
  };

  class btree_reverse_iterator final {
    std::stack<std::pair<btree_node **, size_t>> _path;
    size_t _index;

  public:
    using value_type = tree_data_type_const;
    using reference = value_type &;
    using pointer = value_type *;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = ptrdiff_t;
    using self = btree_reverse_iterator;

    friend class B_tree;
    friend class btree_iterator;
    friend class btree_const_iterator;
    friend class btree_const_reverse_iterator;

    btree_reverse_iterator(const btree_iterator &it) noexcept;
    operator btree_iterator() const noexcept;

    reference operator*() const noexcept;
    pointer operator->() const noexcept;

    self &operator++();
    self operator++(int);

    self &operator--();
    self operator--(int);

    bool operator==(const self &other) const noexcept;
    bool operator!=(const self &other) const noexcept;

    size_t depth() const noexcept;
    size_t current_node_keys_count() const noexcept;
    bool is_terminate_node() const noexcept;
    size_t index() const noexcept;

    explicit btree_reverse_iterator(
        const std::stack<std::pair<btree_node **, size_t>> &path =
            std::stack<std::pair<btree_node **, size_t>>(),
        size_t index = 0);
  };

  class btree_const_reverse_iterator final {
    std::stack<std::pair<btree_node *const *, size_t>> _path;
    size_t _index;

  public:
    using value_type = tree_data_type_const;
    using reference = const value_type &;
    using pointer = const value_type *;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = ptrdiff_t;
    using self = btree_const_reverse_iterator;

    friend class B_tree;
    friend class btree_reverse_iterator;
    friend class btree_const_iterator;
    friend class btree_iterator;

    btree_const_reverse_iterator(const btree_reverse_iterator &it) noexcept;
    operator btree_const_iterator() const noexcept;

    reference operator*() const noexcept;
    pointer operator->() const noexcept;

    self &operator++();
    self operator++(int);

    self &operator--();
    self operator--(int);

    bool operator==(const self &other) const noexcept;
    bool operator!=(const self &other) const noexcept;

    size_t depth() const noexcept;
    size_t current_node_keys_count() const noexcept;
    bool is_terminate_node() const noexcept;
    size_t index() const noexcept;

    explicit btree_const_reverse_iterator(
        const std::stack<std::pair<btree_node *const *, size_t>> &path =
            std::stack<std::pair<btree_node *const *, size_t>>(),
        size_t index = 0);
  };

  friend class btree_iterator;
  friend class btree_const_iterator;
  friend class btree_reverse_iterator;
  friend class btree_const_reverse_iterator;

  // endregion iterators declaration

  // region element access declaration

  /*
   * Returns a reference to the mapped value of the element with specified key.
   * If no such element exists, an exception of type std::out_of_range is
   * thrown.
   */
  tvalue &at(const tkey &);
  const tvalue &at(const tkey &) const;

  /*
   * If key not exists, makes default initialization of value
   */
  tvalue &operator[](const tkey &key);
  tvalue &operator[](tkey &&key);

  // endregion element access declaration
  // region iterator begins declaration

  btree_iterator begin();
  btree_iterator end();

  btree_const_iterator begin() const;
  btree_const_iterator end() const;

  btree_const_iterator cbegin() const;
  btree_const_iterator cend() const;

  btree_reverse_iterator rbegin();
  btree_reverse_iterator rend();

  btree_const_reverse_iterator rbegin() const;
  btree_const_reverse_iterator rend() const;

  btree_const_reverse_iterator crbegin() const;
  btree_const_reverse_iterator crend() const;

  // endregion iterator begins declaration

  // region lookup declaration

  size_t size() const noexcept;
  bool empty() const noexcept;

  /*
   * Returns end() if not exist
   */

  btree_iterator find(const tkey &key);
  btree_const_iterator find(const tkey &key) const;

  btree_iterator lower_bound(const tkey &key);
  btree_const_iterator lower_bound(const tkey &key) const;

  btree_iterator upper_bound(const tkey &key);
  btree_const_iterator upper_bound(const tkey &key) const;

  bool contains(const tkey &key) const;

  // endregion lookup declaration

  // region modifiers declaration

  void clear() noexcept;

  /*
   * Does nothing if key exists, delegates to emplace.
   * Second return value is true, when inserted
   */
  std::pair<btree_iterator, bool> insert(const tree_data_type &data);
  std::pair<btree_iterator, bool> insert(tree_data_type &&data);

  template <typename... Args>
  std::pair<btree_iterator, bool> emplace(Args &&...args);

  /*
   * Updates value if key exists, delegates to emplace.
   */
  btree_iterator insert_or_assign(const tree_data_type &data);
  btree_iterator insert_or_assign(tree_data_type &&data);

  template <typename... Args> btree_iterator emplace_or_assign(Args &&...args);

  /*
   * Return iterator to node next ro removed or end() if key not exists
   */
  btree_iterator erase(btree_iterator pos);
  btree_iterator erase(btree_const_iterator pos);

  btree_iterator erase(btree_iterator beg, btree_iterator en);
  btree_iterator erase(btree_const_iterator beg, btree_const_iterator en);

  btree_iterator erase(const tkey &key);

  // endregion modifiers declaration
};

template <
    std::input_iterator iterator,
    comparator<typename std::iterator_traits<iterator>::value_type::first_type>
        compare = std::less<
            typename std::iterator_traits<iterator>::value_type::first_type>,
    std::size_t t = 5, typename U>
B_tree(iterator begin, iterator end, const compare &cmp = compare(),
       pp_allocator<U> = pp_allocator<U>())
    -> B_tree<typename std::iterator_traits<iterator>::value_type::first_type,
              typename std::iterator_traits<iterator>::value_type::second_type,
              compare, t>;

template <typename tkey, typename tvalue,
          comparator<tkey> compare = std::less<tkey>, std::size_t t = 5,
          typename U>
B_tree(std::initializer_list<std::pair<tkey, tvalue>> data,
       const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>())
    -> B_tree<tkey, tvalue, compare, t>;

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::compare_pairs(
    const B_tree::tree_data_type &lhs,
    const B_tree::tree_data_type &rhs) const {
  return compare_keys(lhs.first, rhs.first);
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs,
                                                    const tkey &rhs) const {
  return compare::operator()(lhs, rhs);
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_node::btree_node() noexcept {}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
pp_allocator<typename B_tree<tkey, tvalue, compare, t>::value_type>
B_tree<tkey, tvalue, compare, t>::get_allocator() const noexcept {
  return _allocator;
}

// region constructors implementation

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(const compare &cmp,
                                         pp_allocator<value_type> alloc)
    : _allocator(alloc), _root(nullptr), _size(0) {
  *static_cast<compare *>(this) = cmp;
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(pp_allocator<value_type> alloc,
                                         const compare &comp)
    : _allocator(alloc), _root(nullptr), _size(0) {
  *static_cast<compare *>(this) = comp;
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
template <input_iterator_for_pair<tkey, tvalue> iterator>
B_tree<tkey, tvalue, compare, t>::B_tree(iterator begin, iterator end,
                                         const compare &cmp,
                                         pp_allocator<value_type> alloc) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "template<input_iterator_for_pair<tkey, tvalue> iterator>\n"
      "B_tree<tkey, tvalue, compare, t>::B_tree(\n"
      "iterator begin,\n"
      "iterator end,\n"
      "const compare& cmp,\n"
      "pp_allocator<value_type> alloc)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(
    std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp,
    pp_allocator<value_type> alloc) {
  throw not_implemented("template<typename tkey, typename tvalue, "
                        "comparator<tkey> compare, std::size_t t>\n"
                        "B_tree<tkey, tvalue, compare, t>::B_tree(\n"
                        "std::initializer_list<std::pair<tkey, tvalue>> data,\n"
                        "const compare& cmp,\n"
                        "pp_allocator<value_type> alloc)",
                        "your code should be here...");
}

// endregion constructors implementation

// region five implementation

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t>::~B_tree() noexcept {
  clear();
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(const B_tree &other) {
  throw not_implemented("template<typename tkey, typename tvalue, "
                        "comparator<tkey> compare, std::size_t t> B_tree<tkey, "
                        "tvalue, compare, t>::B_tree(const B_tree& other)",
                        "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t> &
B_tree<tkey, tvalue, compare, t>::operator=(const B_tree &other) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> B_tree<tkey, tvalue, compare, t>& B_tree<tkey, tvalue, "
      "compare, t>::operator=(const B_tree& other)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(B_tree &&other) noexcept {
  throw not_implemented("template<typename tkey, typename tvalue, "
                        "comparator<tkey> compare, std::size_t t> B_tree<tkey, "
                        "tvalue, compare, t>::B_tree(B_tree&& other) noexcept",
                        "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t> &
B_tree<tkey, tvalue, compare, t>::operator=(B_tree &&other) noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> B_tree<tkey, tvalue, compare, t>& B_tree<tkey, tvalue, "
      "compare, t>::operator=(B_tree&& other) noexcept",
      "your code should be here...");
}

// endregion five implementation

// region iterators implementation

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_iterator::btree_iterator(
    const std::stack<std::pair<btree_node **, size_t>> &path, size_t index)
    : _path(path), _index(index) {}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator*() const noexcept {
  return reinterpret_cast<reference>((*(_path.top().first))->_keys[_index]);
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator->() const noexcept {
  return &((*this).operator*());
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator &
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator++() {
  if (_path.empty()) {
    return *this;
  }

  btree_node *curr = *(_path.top().first);

  if (!curr->_pointers.empty()) {
    btree_node **child_ptr = &(curr->_pointers[_index + 1]);
    _path.push({child_ptr, 0});

    curr = *child_ptr;
    while (!curr->_pointers.empty()) {
      child_ptr = &(curr->_pointers[0]);
      _path.push({child_ptr, 0});
      curr = *child_ptr;
    }
    _index = 0;
  } else {
    _index++;
    while (!_path.empty() && _index >= (*(_path.top().first))->_keys.size()) {
      _index = _path.top().second;
      _path.pop();
    }
  }

  return *this;
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator++(int) {
  auto temp = *this;
  ++(*this);
  return temp;
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator &
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator--() {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> typename B_tree<tkey, tvalue, compare, "
      "t>::btree_iterator& B_tree<tkey, tvalue, compare, "
      "t>::btree_iterator::operator--()",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator--(int) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> typename B_tree<tkey, tvalue, compare, "
      "t>::btree_iterator B_tree<tkey, tvalue, compare, "
      "t>::btree_iterator::operator--(int)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::operator==(
    const self &other) const noexcept {
  if (_path.empty() || other._path.empty()) {
    return _path.empty() == other._path.empty();
  }
  return _path.top().first == other._path.top().first && _index == other._index;
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::operator!=(
    const self &other) const noexcept {
  return !(*this == other);
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
size_t
B_tree<tkey, tvalue, compare, t>::btree_iterator::depth() const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> size_t B_tree<tkey, tvalue, compare, "
      "t>::btree_iterator::depth() const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
size_t
B_tree<tkey, tvalue, compare, t>::btree_iterator::current_node_keys_count()
    const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> size_t B_tree<tkey, tvalue, compare, "
      "t>::btree_iterator::current_node_keys_count() const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::is_terminate_node()
    const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> bool B_tree<tkey, tvalue, compare, "
      "t>::btree_iterator::is_terminate_node() const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
size_t
B_tree<tkey, tvalue, compare, t>::btree_iterator::index() const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> size_t B_tree<tkey, tvalue, compare, "
      "t>::btree_iterator::index() const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::btree_const_iterator(
    const std::stack<std::pair<btree_node *const *, size_t>> &path,
    size_t index)
    : _path(path), _index(index) {}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::btree_const_iterator(
    const btree_iterator &it) noexcept {
  throw not_implemented("template<typename tkey, typename tvalue, "
                        "comparator<tkey> compare, std::size_t t>\n"
                        "B_tree<tkey, tvalue, compare, "
                        "t>::btree_const_iterator::btree_const_iterator(\n"
                        "const btree_iterator& it) noexcept",
                        "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator*()
    const noexcept {
  return reinterpret_cast<reference>((*(_path.top().first))->_keys[_index]);
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator->()
    const noexcept {
  return &((*this).operator*());
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator &
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator++() {
  if (_path.empty())
    return *this;

  const btree_node *curr = *(_path.top().first);

  if (!curr->_pointers.empty()) {
    btree_node *const *child_ptr = &(curr->_pointers[_index + 1]);
    _path.push({child_ptr, 0});
    curr = *child_ptr;
    while (!curr->_pointers.empty()) {
      child_ptr = &(curr->_pointers[0]);
      _path.push({child_ptr, 0});
      curr = *child_ptr;
    }
    _index = 0;
  } else {
    _index++;
    while (!_path.empty() && _index >= (*(_path.top().first))->_keys.size()) {
      _index = _path.top().second;
      _path.pop();
    }
  }
  return *this;
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator++(int) {
  auto temp = *this;
  ++(*this);
  return temp;
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator &
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--() {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator&\n"
      "B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--()",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--(int) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator\n"
      "B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--(int)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator==(
    const self &other) const noexcept {
  if (_path.empty() || other._path.empty()) {
    return _path.empty() == other._path.empty();
  }
  return _path.top().first == other._path.top().first && _index == other._index;
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator!=(
    const self &other) const noexcept {
  return !(*this == other);
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
size_t
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::depth() const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> size_t B_tree<tkey, tvalue, compare, "
      "t>::btree_const_iterator::depth() const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
size_t
B_tree<tkey, tvalue, compare,
       t>::btree_const_iterator::current_node_keys_count() const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> size_t B_tree<tkey, tvalue, compare, "
      "t>::btree_const_iterator::current_node_keys_count() const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::is_terminate_node()
    const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> bool B_tree<tkey, tvalue, compare, "
      "t>::btree_const_iterator::is_terminate_node() const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
size_t
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::index() const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> size_t B_tree<tkey, tvalue, compare, "
      "t>::btree_const_iterator::index() const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::
    btree_reverse_iterator(
        const std::stack<std::pair<btree_node **, size_t>> &path,
        size_t index) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "B_tree<tkey, tvalue, compare, "
      "t>::btree_reverse_iterator::btree_reverse_iterator(\n"
      "const std::stack<std::pair<btree_node**, size_t>>& path, size_t index)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::
    btree_reverse_iterator(const btree_iterator &it) noexcept {
  throw not_implemented("template<typename tkey, typename tvalue, "
                        "comparator<tkey> compare, std::size_t t>\n"
                        "B_tree<tkey, tvalue, compare, "
                        "t>::btree_reverse_iterator::btree_reverse_iterator(\n"
                        "const btree_iterator& it) noexcept",
                        "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator B_tree<
    tkey, tvalue, compare, t>::btree_iterator() const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> B_tree<tkey, tvalue, compare, "
      "t>::btree_reverse_iterator::operator btree_iterator() const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator*()
    const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "typename B_tree<tkey, tvalue, compare, "
      "t>::btree_reverse_iterator::reference\n"
      "B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator*() "
      "const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator->()
    const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "typename B_tree<tkey, tvalue, compare, "
      "t>::btree_reverse_iterator::pointer\n"
      "B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator->() "
      "const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator &
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator++() {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator&\n"
      "B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator++()",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator++(int) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator\n"
      "B_tree<tkey, tvalue, compare, "
      "t>::btree_reverse_iterator::operator++(int)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator &
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator--() {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator&\n"
      "B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator--()",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator--(int) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator\n"
      "B_tree<tkey, tvalue, compare, "
      "t>::btree_reverse_iterator::operator--(int)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator==(
    const self &other) const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> bool B_tree<tkey, tvalue, compare, "
      "t>::btree_reverse_iterator::operator==(const self& other) const "
      "noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator!=(
    const self &other) const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> bool B_tree<tkey, tvalue, compare, "
      "t>::btree_reverse_iterator::operator!=(const self& other) const "
      "noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::depth()
    const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> size_t B_tree<tkey, tvalue, compare, "
      "t>::btree_reverse_iterator::depth() const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
size_t
B_tree<tkey, tvalue, compare,
       t>::btree_reverse_iterator::current_node_keys_count() const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> size_t B_tree<tkey, tvalue, compare, "
      "t>::btree_reverse_iterator::current_node_keys_count() const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool B_tree<tkey, tvalue, compare,
            t>::btree_reverse_iterator::is_terminate_node() const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> bool B_tree<tkey, tvalue, compare, "
      "t>::btree_reverse_iterator::is_terminate_node() const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::index()
    const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> size_t B_tree<tkey, tvalue, compare, "
      "t>::btree_reverse_iterator::index() const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::
    btree_const_reverse_iterator(
        const std::stack<std::pair<btree_node *const *, size_t>> &path,
        size_t index) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "B_tree<tkey, tvalue, compare, "
      "t>::btree_const_reverse_iterator::btree_const_reverse_iterator(\n"
      "const std::stack<std::pair<const btree_node**, size_t>>& path, size_t "
      "index)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::
    btree_const_reverse_iterator(const btree_reverse_iterator &it) noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "B_tree<tkey, tvalue, compare, "
      "t>::btree_const_reverse_iterator::btree_const_reverse_iterator(\n"
      "const btree_reverse_iterator& it) noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator B_tree<
    tkey, tvalue, compare, t>::btree_const_iterator() const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> B_tree<tkey, tvalue, compare, "
      "t>::btree_const_reverse_iterator::operator btree_const_iterator() const "
      "noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare,
                t>::btree_const_reverse_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator*()
    const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "typename B_tree<tkey, tvalue, compare, "
      "t>::btree_const_reverse_iterator::reference\n"
      "B_tree<tkey, tvalue, compare, "
      "t>::btree_const_reverse_iterator::operator*() const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator->()
    const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "typename B_tree<tkey, tvalue, compare, "
      "t>::btree_const_reverse_iterator::pointer\n"
      "B_tree<tkey, tvalue, compare, "
      "t>::btree_const_reverse_iterator::operator->() const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator &
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator++() {
  throw not_implemented("template<typename tkey, typename tvalue, "
                        "comparator<tkey> compare, std::size_t t>\n"
                        "typename B_tree<tkey, tvalue, compare, "
                        "t>::btree_const_reverse_iterator&\n"
                        "B_tree<tkey, tvalue, compare, "
                        "t>::btree_const_reverse_iterator::operator++()",
                        "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator++(
    int) {
  throw not_implemented("template<typename tkey, typename tvalue, "
                        "comparator<tkey> compare, std::size_t t>\n"
                        "typename B_tree<tkey, tvalue, compare, "
                        "t>::btree_const_reverse_iterator\n"
                        "B_tree<tkey, tvalue, compare, "
                        "t>::btree_const_reverse_iterator::operator++(int)",
                        "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator &
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator--() {
  throw not_implemented("template<typename tkey, typename tvalue, "
                        "comparator<tkey> compare, std::size_t t>\n"
                        "typename B_tree<tkey, tvalue, compare, "
                        "t>::btree_const_reverse_iterator&\n"
                        "B_tree<tkey, tvalue, compare, "
                        "t>::btree_const_reverse_iterator::operator--()",
                        "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator--(
    int) {
  throw not_implemented("template<typename tkey, typename tvalue, "
                        "comparator<tkey> compare, std::size_t t>\n"
                        "typename B_tree<tkey, tvalue, compare, "
                        "t>::btree_const_reverse_iterator\n"
                        "B_tree<tkey, tvalue, compare, "
                        "t>::btree_const_reverse_iterator::operator--(int)",
                        "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator==(
    const self &other) const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> bool B_tree<tkey, tvalue, compare, "
      "t>::btree_const_reverse_iterator::operator==(const self& other) const "
      "noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator!=(
    const self &other) const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> bool B_tree<tkey, tvalue, compare, "
      "t>::btree_const_reverse_iterator::operator!=(const self& other) const "
      "noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::depth()
    const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> size_t B_tree<tkey, tvalue, compare, "
      "t>::btree_const_reverse_iterator::depth() const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
size_t B_tree<tkey, tvalue, compare,
              t>::btree_const_reverse_iterator::current_node_keys_count()
    const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> size_t B_tree<tkey, tvalue, compare, "
      "t>::btree_const_reverse_iterator::current_node_keys_count() const "
      "noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool B_tree<tkey, tvalue, compare,
            t>::btree_const_reverse_iterator::is_terminate_node()
    const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> bool B_tree<tkey, tvalue, compare, "
      "t>::btree_const_reverse_iterator::is_terminate_node() const noexcept",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::index()
    const noexcept {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> size_t B_tree<tkey, tvalue, compare, "
      "t>::btree_const_reverse_iterator::index() const noexcept",
      "your code should be here...");
}

// endregion iterators implementation

// region element access implementation

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
tvalue &B_tree<tkey, tvalue, compare, t>::at(const tkey &key) {
  throw not_implemented("template<typename tkey, typename tvalue, "
                        "comparator<tkey> compare, std::size_t t> tvalue& "
                        "B_tree<tkey, tvalue, compare, t>::at(const tkey& key)",
                        "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
const tvalue &B_tree<tkey, tvalue, compare, t>::at(const tkey &key) const {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> const tvalue& B_tree<tkey, tvalue, compare, t>::at(const "
      "tkey& key) const",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
tvalue &B_tree<tkey, tvalue, compare, t>::operator[](const tkey &key) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> tvalue& B_tree<tkey, tvalue, compare, "
      "t>::operator[](const tkey& key)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
tvalue &B_tree<tkey, tvalue, compare, t>::operator[](tkey &&key) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> tvalue& B_tree<tkey, tvalue, compare, "
      "t>::operator[](tkey&& key)",
      "your code should be here...");
}

// endregion element access implementation

// region iterator begins implementation

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::begin() {
  if (_root == nullptr)
    return end();

  std::stack<std::pair<btree_node **, size_t>> path;
  btree_node **curr = &_root;

  while (curr != nullptr && *curr != nullptr) {
    path.push({curr, 0});
    if ((*curr)->_pointers.empty())
      break;
    curr = &((*curr)->_pointers[0]);
  }

  return btree_iterator(path, 0);
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::end() {
  return btree_iterator();
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::begin() const {
  return cbegin();
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::end() const {
  return btree_iterator();
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::cbegin() const {
  if (_root == nullptr)
    return cend();

  std::stack<std::pair<btree_node *const *, size_t>> path;
  btree_node *const *curr = &_root;

  while (curr != nullptr && *curr != nullptr) {
    path.push({curr, 0});
    if ((*curr)->_pointers.empty())
      break;
    curr = &((*curr)->_pointers[0]);
  }

  return btree_const_iterator(path, 0);
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::cend() const {
  return btree_const_iterator();
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::rbegin() {
  return btree_iterator();
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::rend() {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> typename B_tree<tkey, tvalue, compare, "
      "t>::btree_reverse_iterator B_tree<tkey, tvalue, compare, t>::rend()",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::rbegin() const {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> typename B_tree<tkey, tvalue, compare, "
      "t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, "
      "t>::rbegin() const",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::rend() const {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> typename B_tree<tkey, tvalue, compare, "
      "t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, "
      "t>::rend() const",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::crbegin() const {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> typename B_tree<tkey, tvalue, compare, "
      "t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, "
      "t>::crbegin() const",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::crend() const {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> typename B_tree<tkey, tvalue, compare, "
      "t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, "
      "t>::crend() const",
      "your code should be here...");
}

// endregion iterator begins implementation

// region lookup implementation

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::size() const noexcept {
  return _size;
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::empty() const noexcept {
  return _size == 0;
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::find(const tkey &key) {
  std::stack<std::pair<btree_node **, size_t>> path;

  btree_node **current_ptr = &_root;

  while (*current_ptr != nullptr) {
    size_t i = 0;

    while (i < (*current_ptr)->_keys.size() &&
           compare_keys((*current_ptr)->_keys[i].first, key)) {
      ++i;
    }

    if (i < (*current_ptr)->_keys.size() &&
        !compare_keys((*current_ptr)->_keys[i].first, key) &&
        !compare_keys(key, (*current_ptr)->_keys[i].first)) {

      path.push({current_ptr, i});
      return btree_iterator(path, i);
    }

    if ((*current_ptr)->_pointers.empty()) {
      return end();
    }

    path.push({current_ptr, i});

    current_ptr = &((*current_ptr)->_pointers[i]);
  }

  return end();
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::find(const tkey &key) const {
  std::stack<std::pair<btree_node *const *, size_t>> path;
  btree_node *const *current_ptr = &_root;

  while (*current_ptr != nullptr) {
    size_t i = 0;
    while (i < (*current_ptr)->_keys.size() &&
           compare_keys((*current_ptr)->_keys[i].first, key)) {
      ++i;
    }

    if (i < (*current_ptr)->_keys.size() &&
        !compare_keys((*current_ptr)->_keys[i].first, key) &&
        !compare_keys(key, (*current_ptr)->_keys[i].first)) {
      path.push({current_ptr, i});
      return btree_const_iterator(path, i);
    }

    if ((*current_ptr)->_pointers.empty()) {
      return end();
    }

    path.push({current_ptr, i});
    current_ptr = &((*current_ptr)->_pointers[i]);
  }

  return end();
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::lower_bound(const tkey &key) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> typename B_tree<tkey, tvalue, compare, "
      "t>::btree_iterator B_tree<tkey, tvalue, compare, t>::lower_bound(const "
      "tkey& key)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::lower_bound(const tkey &key) const {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> typename B_tree<tkey, tvalue, compare, "
      "t>::btree_const_iterator B_tree<tkey, tvalue, compare, "
      "t>::lower_bound(const tkey& key) const",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::upper_bound(const tkey &key) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> typename B_tree<tkey, tvalue, compare, "
      "t>::btree_iterator B_tree<tkey, tvalue, compare, t>::upper_bound(const "
      "tkey& key)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::upper_bound(const tkey &key) const {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t> typename B_tree<tkey, tvalue, compare, "
      "t>::btree_const_iterator B_tree<tkey, tvalue, compare, "
      "t>::upper_bound(const tkey& key) const",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::contains(const tkey &key) const {
  btree_node *current = _root;

  while (current != nullptr) {

    size_t i = 0;

    while (i < current->_keys.size() &&
           compare_keys(current->_keys[i].first, key)) {
      ++i;
    }

    if (i < current->_keys.size() &&
        !compare_keys(current->_keys[i].first, key) &&
        !compare_keys(key, current->_keys[i].first)) {
      return true;
    }

    if (current->_pointers.empty()) {
      return false;
    }

    current = current->_pointers[i];
  }

  return false;
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_node *
B_tree<tkey, tvalue, compare, t>::create_node() {
  node_allocator_type node_alloc{_allocator};

  btree_node *node = node_traits::allocate(node_alloc, 1);

  node_traits::construct(node_alloc, node);

  return node;
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
void B_tree<tkey, tvalue, compare, t>::insert_into_node(
    btree_node *node, tree_data_type &&data, btree_node *right_child) {
  auto it = std::lower_bound(
      node->_keys.begin(), node->_keys.end(), data.first,
      [this](const tree_data_type &pair_in_vector, const tkey &key_to_insert) {
        return compare_keys(pair_in_vector.first, key_to_insert);
      });

  auto index = std::distance(node->_keys.begin(), it);

  node->_keys.insert(it, std::move(data));

  if (right_child != nullptr) {
    node->_pointers.insert(node->_pointers.begin() + index + 1, right_child);
  }
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_node *
B_tree<tkey, tvalue, compare, t>::split_node(btree_node *node,
                                             tree_data_type &median_data) {
  size_t mid_idx = t - 1;

  median_data = std::move(node->_keys[mid_idx]);

  btree_node *right_node = create_node();

  right_node->_keys.assign(
      std::make_move_iterator(node->_keys.begin() + mid_idx + 1),
      std::make_move_iterator(node->_keys.end()));

  if (!node->_pointers.empty()) {
    right_node->_pointers.assign(
        std::make_move_iterator(node->_pointers.begin() + mid_idx + 1),
        std::make_move_iterator(node->_pointers.end()));
    node->_pointers.erase(node->_pointers.begin() + mid_idx + 1,
                          node->_pointers.end());
  }

  node->_keys.erase(node->_keys.begin() + mid_idx, node->_keys.end());

  return right_node;
}
// endregion lookup implementation

// region modifiers implementation

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
void B_tree<tkey, tvalue, compare, t>::clear() noexcept {
  destroy_node(_root);
  _root = nullptr;
  _size = 0;
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::insert(const tree_data_type &data) {
  return insert(tree_data_type(data));
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::insert(tree_data_type &&data) {
  auto existing_it = find(data.first);
  if (existing_it != end()) {
    return {existing_it, false};
  }

  tkey key_to_return = data.first;

  if (_root == nullptr) {
    _root = create_node();
    _root->_keys.push_back(std::move(data));
    _size++;
    return {find(key_to_return), true};
  }

  std::stack<btree_node *> path;
  btree_node *curr = _root;

  while (curr != nullptr) {
    path.push(curr);
    if (curr->_pointers.empty())
      break;

    auto it =
        std::lower_bound(curr->_keys.begin(), curr->_keys.end(), data.first,
                         [this](const tree_data_type &p, const tkey &k) {
                           return compare_keys(p.first, k);
                         });
    size_t idx = std::distance(curr->_keys.begin(), it);
    curr = curr->_pointers[idx];
  }

  btree_node *node = path.top();
  insert_into_node(node, std::move(data));
  _size++;

  tree_data_type median;
  btree_node *right_child = nullptr;

  while (!path.empty() && node->_keys.size() > maximum_keys_in_node) {
    node = path.top();
    path.pop();

    right_child = split_node(node, median);

    if (path.empty()) {
      btree_node *new_root = create_node();
      new_root->_keys.push_back(std::move(median));
      new_root->_pointers.push_back(node);
      new_root->_pointers.push_back(right_child);
      _root = new_root;
    } else {
      btree_node *parent = path.top();
      insert_into_node(parent, std::move(median), right_child);
      node = parent;
    }
  }

  return {find(key_to_return), true};
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
template <typename... Args>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::emplace(Args &&...args) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "template<typename... Args>\n"
      "std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, "
      "bool>\n"
      "B_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type &data) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "typename B_tree<tkey, tvalue, compare, t>::btree_iterator\n"
      "B_tree<tkey, tvalue, compare, t>::insert_or_assign(const "
      "tree_data_type& data)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type &&data) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "typename B_tree<tkey, tvalue, compare, t>::btree_iterator\n"
      "B_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& "
      "data)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
template <typename... Args>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args &&...args) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "template<typename... Args>\n"
      "typename B_tree<tkey, tvalue, compare, t>::btree_iterator\n"
      "B_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_iterator pos) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "typename B_tree<tkey, tvalue, compare, t>::btree_iterator\n"
      "B_tree<tkey, tvalue, compare, t>::erase(btree_iterator pos)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator pos) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "typename B_tree<tkey, tvalue, compare, t>::btree_iterator\n"
      "B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator pos)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_iterator beg, btree_iterator en) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "typename B_tree<tkey, tvalue, compare, t>::btree_iterator\n"
      "B_tree<tkey, tvalue, compare, t>::erase(btree_iterator beg, "
      "btree_iterator en)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator beg,
                                        btree_const_iterator en) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "typename B_tree<tkey, tvalue, compare, t>::btree_iterator\n"
      "B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator beg, "
      "btree_const_iterator en)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(const tkey &key) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "typename B_tree<tkey, tvalue, compare, t>::btree_iterator\n"
      "B_tree<tkey, tvalue, compare, t>::erase(const tkey& key)",
      "your code should be here...");
}

// endregion modifiers implementation

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool compare_pairs(
    const typename B_tree<tkey, tvalue, compare, t>::tree_data_type &lhs,
    const typename B_tree<tkey, tvalue, compare, t>::tree_data_type &rhs) {
  throw not_implemented(
      "template<typename tkey, typename tvalue, comparator<tkey> compare, "
      "std::size_t t>\n"
      "bool compare_pairs(const typename B_tree<tkey, tvalue, compare, "
      "t>::tree_data_type &lhs,\n"
      "const typename B_tree<tkey, tvalue, compare, t>::tree_data_type &rhs)",
      "your code should be here...");
}

template <typename tkey, typename tvalue, comparator<tkey> compare,
          std::size_t t>
bool compare_keys(const tkey &lhs, const tkey &rhs) {
  throw not_implemented("template<typename tkey, typename tvalue, "
                        "comparator<tkey> compare, std::size_t >\n"
                        "bool compare_keys(const tkey &lhs, const tkey &rhs)",
                        "your code should be here...");
}

#endif
