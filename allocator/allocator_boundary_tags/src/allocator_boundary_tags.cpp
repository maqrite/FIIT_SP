#include "../include/allocator_boundary_tags.h"
#include <not_implemented.h>

#include <algorithm>
#include <cstring>
#include <mutex>
#include <new>

namespace {
constexpr size_t META_SIZE = sizeof(size_t) + sizeof(void *) + sizeof(void *) +
                             sizeof(void *); // Ровно 32 байта

inline std::pmr::memory_resource *&parent_alloc(void *t) {
  return *reinterpret_cast<std::pmr::memory_resource **>(t);
}
inline allocator_with_fit_mode::fit_mode &fit_mode_ref(void *t) {
  return *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(
      reinterpret_cast<char *>(t) + sizeof(std::pmr::memory_resource *));
}
inline size_t &space_size_ref(void *t) {
  return *reinterpret_cast<size_t *>(reinterpret_cast<char *>(t) +
                                     sizeof(std::pmr::memory_resource *) +
                                     sizeof(allocator_with_fit_mode::fit_mode));
}
inline std::mutex &mutex_ref(void *t) {
  return *reinterpret_cast<std::mutex *>(
      reinterpret_cast<char *>(t) + sizeof(std::pmr::memory_resource *) +
      sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t));
}

inline void *get_first_physical(void *t) {
  return reinterpret_cast<char *>(t) + sizeof(std::pmr::memory_resource *) +
         sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t) +
         sizeof(std::mutex) + sizeof(void *);
}

inline size_t &block_size_raw(void *block) {
  return *reinterpret_cast<size_t *>(block);
}
inline bool is_occupied(void *block) {
  return (block_size_raw(block) & 1) != 0;
}
inline size_t get_size(void *block) { return block_size_raw(block) & ~1ULL; }
inline void set_size_and_status(void *block, size_t size, bool occupied) {
  block_size_raw(block) = size | (occupied ? 1 : 0);
}

inline void *&block_parent_alloc(void *block) {
  return *reinterpret_cast<void **>(reinterpret_cast<char *>(block) +
                                    sizeof(size_t));
}
inline void *&prev_physical(void *block) {
  return *reinterpret_cast<void **>(reinterpret_cast<char *>(block) +
                                    sizeof(size_t) + sizeof(void *));
}
inline void *&next_physical(void *block) {
  return *reinterpret_cast<void **>(reinterpret_cast<char *>(block) +
                                    sizeof(size_t) + sizeof(void *) * 2);
}
} // namespace

allocator_boundary_tags::~allocator_boundary_tags() {
  if (_trusted_memory) {
    std::pmr::memory_resource *parent = parent_alloc(_trusted_memory);
    size_t s_size = space_size_ref(_trusted_memory);
    mutex_ref(_trusted_memory).~mutex();
    parent->deallocate(_trusted_memory, s_size + allocator_metadata_size);
    _trusted_memory = nullptr;
  }
}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags &&other) noexcept {
  _trusted_memory = other._trusted_memory;
  other._trusted_memory = nullptr;
}

allocator_boundary_tags &
allocator_boundary_tags::operator=(allocator_boundary_tags &&other) noexcept {
  if (this != &other) {
    this->~allocator_boundary_tags();
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
  }
  return *this;
}

allocator_boundary_tags::allocator_boundary_tags(
    size_t space_size, std::pmr::memory_resource *parent_allocator,
    allocator_with_fit_mode::fit_mode allocate_fit_mode) {
  if (!parent_allocator)
    parent_allocator = std::pmr::get_default_resource();

  size_t actual_alloc_size = space_size + allocator_metadata_size;
  _trusted_memory = parent_allocator->allocate(actual_alloc_size);

  parent_alloc(_trusted_memory) = parent_allocator;
  fit_mode_ref(_trusted_memory) = allocate_fit_mode;
  space_size_ref(_trusted_memory) = space_size;
  new (&mutex_ref(_trusted_memory)) std::mutex();

  void *first_block = get_first_physical(_trusted_memory);

  size_t initial_size = space_size;
  set_size_and_status(first_block, initial_size, false);
  block_parent_alloc(first_block) = this;
  prev_physical(first_block) = nullptr;
  next_physical(first_block) = nullptr;
}

[[nodiscard]] void *allocator_boundary_tags::do_allocate_sm(size_t size) {
  std::lock_guard<std::mutex> lock(mutex_ref(_trusted_memory));

  size_t total_required = size + META_SIZE;
  fit_mode mode = fit_mode_ref(_trusted_memory);
  void *best_block = nullptr;
  void *curr = get_first_physical(_trusted_memory);

  while (curr) {
    if (!is_occupied(curr)) {
      size_t bsize = get_size(curr);
      if (bsize >= total_required) {
        if (mode == fit_mode::first_fit) {
          best_block = curr;
          break;
        } else if (mode == fit_mode::the_best_fit) {
          if (!best_block || bsize < get_size(best_block))
            best_block = curr;
        } else if (mode == fit_mode::the_worst_fit) {
          if (!best_block || bsize > get_size(best_block))
            best_block = curr;
        }
      }
    }
    curr = next_physical(curr);
  }

  if (!best_block)
    throw std::bad_alloc();

  size_t bsize = get_size(best_block);

  if (bsize - total_required >= META_SIZE) {
    void *new_block = reinterpret_cast<char *>(best_block) + total_required;
    size_t new_size = bsize - total_required;

    set_size_and_status(new_block, new_size, false);
    block_parent_alloc(new_block) = this;

    next_physical(new_block) = next_physical(best_block);
    prev_physical(new_block) = best_block;

    if (next_physical(best_block)) {
      prev_physical(next_physical(best_block)) = new_block;
    }
    next_physical(best_block) = new_block;

    set_size_and_status(best_block, total_required, true);
  } else {
    set_size_and_status(best_block, bsize, true);
  }

  return reinterpret_cast<char *>(best_block) + META_SIZE;
}

void allocator_boundary_tags::do_deallocate_sm(void *at) {
  if (!at)
    return;
  std::lock_guard<std::mutex> lock(mutex_ref(_trusted_memory));

  void *block = reinterpret_cast<char *>(at) - META_SIZE;

  void *curr = get_first_physical(_trusted_memory);
  bool found = false;
  while (curr) {
    if (curr == block) {
      found = true;
      break;
    }
    curr = next_physical(curr);
  }
  if (!found || !is_occupied(block))
    throw std::logic_error("Invalid pointer");

  set_size_and_status(block, get_size(block), false);

  void *next = next_physical(block);
  if (next && !is_occupied(next)) {
    size_t new_size = get_size(block) + get_size(next);
    set_size_and_status(block, new_size, false);

    void *next_next = next_physical(next);
    next_physical(block) = next_next;
    if (next_next)
      prev_physical(next_next) = block;
  }

  void *prev = prev_physical(block);
  if (prev && !is_occupied(prev)) {
    size_t new_size = get_size(prev) + get_size(block);
    set_size_and_status(prev, new_size, false);

    void *next_next = next_physical(block);
    next_physical(prev) = next_next;
    if (next_next)
      prev_physical(next_next) = prev;
  }
}

inline void
allocator_boundary_tags::set_fit_mode(allocator_with_fit_mode::fit_mode mode) {
  std::lock_guard<std::mutex> lock(mutex_ref(_trusted_memory));
  fit_mode_ref(_trusted_memory) = mode;
}

std::vector<allocator_test_utils::block_info>
allocator_boundary_tags::get_blocks_info() const {
  std::lock_guard<std::mutex> lock(mutex_ref(_trusted_memory));
  return get_blocks_info_inner();
}

allocator_boundary_tags::boundary_iterator
allocator_boundary_tags::begin() const noexcept {
  return boundary_iterator(_trusted_memory);
}

allocator_boundary_tags::boundary_iterator
allocator_boundary_tags::end() const noexcept {
  return boundary_iterator();
}

std::vector<allocator_test_utils::block_info>
allocator_boundary_tags::get_blocks_info_inner() const {
  std::vector<allocator_test_utils::block_info> result;
  void *curr = get_first_physical(_trusted_memory);
  while (curr) {
    result.push_back({get_size(curr), is_occupied(curr)});
    curr = next_physical(curr);
  }
  return result;
}

allocator_boundary_tags::allocator_boundary_tags(
    const allocator_boundary_tags &other) {
  if (!other._trusted_memory) {
    _trusted_memory = nullptr;
    return;
  }

  std::lock_guard<std::mutex> lock(mutex_ref(other._trusted_memory));
  size_t s_size = space_size_ref(other._trusted_memory);
  std::pmr::memory_resource *parent = parent_alloc(other._trusted_memory);

  _trusted_memory = parent->allocate(s_size + allocator_metadata_size);
  std::memcpy(_trusted_memory, other._trusted_memory,
              s_size + allocator_metadata_size);

  new (&mutex_ref(_trusted_memory)) std::mutex();

  ptrdiff_t diff = reinterpret_cast<char *>(_trusted_memory) -
                   reinterpret_cast<char *>(other._trusted_memory);

  void *curr = get_first_physical(_trusted_memory);
  while (curr) {
    block_parent_alloc(curr) = this;
    if (prev_physical(curr))
      prev_physical(curr) =
          reinterpret_cast<char *>(prev_physical(curr)) + diff;
    if (next_physical(curr))
      next_physical(curr) =
          reinterpret_cast<char *>(next_physical(curr)) + diff;

    curr = next_physical(curr);
  }
}

allocator_boundary_tags &
allocator_boundary_tags::operator=(const allocator_boundary_tags &other) {
  if (this != &other) {
    this->~allocator_boundary_tags();
    allocator_boundary_tags temp(other);
    _trusted_memory = temp._trusted_memory;
    temp._trusted_memory = nullptr;
  }
  return *this;
}

bool allocator_boundary_tags::do_is_equal(
    const std::pmr::memory_resource &other) const noexcept {
  auto ptr = dynamic_cast<const allocator_boundary_tags *>(&other);
  return ptr && _trusted_memory == ptr->_trusted_memory;
}

bool allocator_boundary_tags::boundary_iterator::operator==(
    const allocator_boundary_tags::boundary_iterator &other) const noexcept {
  return _occupied_ptr == other._occupied_ptr;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(
    const allocator_boundary_tags::boundary_iterator &other) const noexcept {
  return !(*this == other);
}

allocator_boundary_tags::boundary_iterator &
allocator_boundary_tags::boundary_iterator::operator++() & noexcept {
  if (_occupied_ptr) {
    _occupied_ptr = next_physical(_occupied_ptr);
    if (_occupied_ptr)
      _occupied = is_occupied(_occupied_ptr);
  }
  return *this;
}

allocator_boundary_tags::boundary_iterator &
allocator_boundary_tags::boundary_iterator::operator--() & noexcept {
  if (_occupied_ptr) {
    _occupied_ptr = prev_physical(_occupied_ptr);
    if (_occupied_ptr)
      _occupied = is_occupied(_occupied_ptr);
  }
  return *this;
}

allocator_boundary_tags::boundary_iterator
allocator_boundary_tags::boundary_iterator::operator++(int n) {
  auto temp = *this;
  ++(*this);
  return temp;
}

allocator_boundary_tags::boundary_iterator
allocator_boundary_tags::boundary_iterator::operator--(int n) {
  auto temp = *this;
  --(*this);
  return temp;
}

size_t allocator_boundary_tags::boundary_iterator::size() const noexcept {
  return _occupied_ptr ? get_size(_occupied_ptr) : 0;
}

bool allocator_boundary_tags::boundary_iterator::occupied() const noexcept {
  return _occupied;
}

void *allocator_boundary_tags::boundary_iterator::operator*() const noexcept {
  return _occupied_ptr ? reinterpret_cast<char *>(_occupied_ptr) + META_SIZE
                       : nullptr;
}

allocator_boundary_tags::boundary_iterator::boundary_iterator()
    : _occupied_ptr(nullptr), _occupied(false), _trusted_memory(nullptr) {}

allocator_boundary_tags::boundary_iterator::boundary_iterator(void *trusted)
    : _occupied_ptr(nullptr), _occupied(false), _trusted_memory(trusted) {
  if (trusted) {
    _occupied_ptr = get_first_physical(trusted);
    if (_occupied_ptr)
      _occupied = is_occupied(_occupied_ptr);
  }
}

void *allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept {
  return _occupied_ptr;
}
