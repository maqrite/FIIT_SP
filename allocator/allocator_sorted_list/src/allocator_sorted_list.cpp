#include "../include/allocator_sorted_list.h"
#include <not_implemented.h>

namespace {
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

inline void *&first_free_ref(void *t) {
  return *reinterpret_cast<void **>(reinterpret_cast<char *>(t) +
                                    sizeof(std::pmr::memory_resource *) +
                                    sizeof(allocator_with_fit_mode::fit_mode) +
                                    sizeof(size_t) + sizeof(std::mutex));
}

inline size_t &block_size_ref(void *block) {
  return *reinterpret_cast<size_t *>(block);
}
inline void *&next_free_ref(void *block) {
  return *reinterpret_cast<void **>(reinterpret_cast<char *>(block) +
                                    sizeof(size_t));
}

inline bool is_occupied(void *block) { return next_free_ref(block) == block; }

inline void set_occupied(void *block) { next_free_ref(block) = block; }

inline void set_free(void *block, void *next) { next_free_ref(block) = next; }

} // namespace

allocator_sorted_list::~allocator_sorted_list() {
  if (_trusted_memory) {
    std::pmr::memory_resource *parent = parent_alloc(_trusted_memory);
    size_t size = space_size_ref(_trusted_memory);

    mutex_ref(_trusted_memory).~mutex();
    parent->deallocate(_trusted_memory, size);
    _trusted_memory = nullptr;
  }
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list &&other) noexcept {
  _trusted_memory = other._trusted_memory;
  other._trusted_memory = nullptr;
}

allocator_sorted_list &
allocator_sorted_list::operator=(allocator_sorted_list &&other) noexcept {
  if (this != &other) {
    this->~allocator_sorted_list();
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
  }
  return *this;
}

allocator_sorted_list::allocator_sorted_list(
    size_t space_size, std::pmr::memory_resource *parent_allocator,
    allocator_with_fit_mode::fit_mode allocate_fit_mode) {
  if (!parent_allocator) {
    parent_allocator = std::pmr::get_default_resource();
  }

  if (space_size < allocator_metadata_size + block_metadata_size + 1) {
    throw std::logic_error("Not enough space for metadata");
  }

  _trusted_memory = parent_allocator->allocate(space_size);

  parent_alloc(_trusted_memory) = parent_allocator;
  fit_mode_ref(_trusted_memory) = allocate_fit_mode;
  space_size_ref(_trusted_memory) = space_size;

  new (&mutex_ref(_trusted_memory)) std::mutex();

  void *first_block =
      reinterpret_cast<char *>(_trusted_memory) + allocator_metadata_size;
  first_free_ref(_trusted_memory) = first_block;

  block_size_ref(first_block) =
      space_size - allocator_metadata_size - block_metadata_size;
  set_free(first_block, nullptr);
}

[[nodiscard]] void *allocator_sorted_list::do_allocate_sm(size_t size) {
  std::lock_guard<std::mutex> lock(mutex_ref(_trusted_memory));

  fit_mode mode = fit_mode_ref(_trusted_memory);
  void *best_block = nullptr;
  void *best_prev = nullptr;

  void *curr = first_free_ref(_trusted_memory);
  void *prev = nullptr;

  while (curr) {
    size_t bsize = block_size_ref(curr);
    if (bsize >= size) {
      if (mode == fit_mode::first_fit) {
        best_block = curr;
        best_prev = prev;
        break;
      } else if (mode == fit_mode::the_best_fit) {
        if (!best_block || bsize < block_size_ref(best_block)) {
          best_block = curr;
          best_prev = prev;
        }
      } else if (mode == fit_mode::the_worst_fit) {
        if (!best_block || bsize > block_size_ref(best_block)) {
          best_block = curr;
          best_prev = prev;
        }
      }
    }
    prev = curr;
    curr = next_free_ref(curr);
  }

  if (!best_block) {
    throw std::bad_alloc();
  }

  size_t bsize = block_size_ref(best_block);

  if (bsize >= size + block_metadata_size + 1) {
    void *new_free =
        reinterpret_cast<char *>(best_block) + block_metadata_size + size;
    block_size_ref(new_free) = bsize - size - block_metadata_size;
    set_free(new_free, next_free_ref(best_block));

    block_size_ref(best_block) = size;

    if (best_prev)
      set_free(best_prev, new_free);
    else
      first_free_ref(_trusted_memory) = new_free;
  } else {
    if (best_prev)
      set_free(best_prev, next_free_ref(best_block));
    else
      first_free_ref(_trusted_memory) = next_free_ref(best_block);
  }

  set_occupied(best_block);
  return reinterpret_cast<char *>(best_block) + block_metadata_size;
}

allocator_sorted_list::allocator_sorted_list(
    const allocator_sorted_list &other) {
  if (!other._trusted_memory) {
    _trusted_memory = nullptr;
    return;
  }

  std::lock_guard<std::mutex> lock(mutex_ref(other._trusted_memory));

  size_t size = space_size_ref(other._trusted_memory);
  std::pmr::memory_resource *parent = parent_alloc(other._trusted_memory);

  _trusted_memory = parent->allocate(size);
  std::memcpy(_trusted_memory, other._trusted_memory, size);

  new (&mutex_ref(_trusted_memory)) std::mutex();

  ptrdiff_t diff = reinterpret_cast<char *>(_trusted_memory) -
                   reinterpret_cast<char *>(other._trusted_memory);

  if (first_free_ref(_trusted_memory)) {
    first_free_ref(_trusted_memory) =
        reinterpret_cast<char *>(first_free_ref(_trusted_memory)) + diff;
  }

  void *curr =
      reinterpret_cast<char *>(_trusted_memory) + allocator_metadata_size;
  void *end = reinterpret_cast<char *>(_trusted_memory) + size;

  while (curr < end) {
    if (!is_occupied(curr) && next_free_ref(curr) != nullptr) {
      next_free_ref(curr) =
          reinterpret_cast<char *>(next_free_ref(curr)) + diff;
    } else if (is_occupied(curr)) {
      next_free_ref(curr) = curr;
    }
    curr = reinterpret_cast<char *>(curr) + block_metadata_size +
           block_size_ref(curr);
  }
}

allocator_sorted_list &
allocator_sorted_list::operator=(const allocator_sorted_list &other) {
  if (this != &other) {
    this->~allocator_sorted_list();
    allocator_sorted_list temp(other);
    _trusted_memory = temp._trusted_memory;
    temp._trusted_memory = nullptr;
  }
  return *this;
}

bool allocator_sorted_list::do_is_equal(
    const std::pmr::memory_resource &other) const noexcept {
  auto ptr = dynamic_cast<const allocator_sorted_list *>(&other);
  return ptr && _trusted_memory == ptr->_trusted_memory;
}

void allocator_sorted_list::do_deallocate_sm(void *at) {
  if (!at) {
    return;
  }

  std::lock_guard<std::mutex> lock(mutex_ref(_trusted_memory));
  void *block = reinterpret_cast<char *>(at) - block_metadata_size;

  void *end_ptr = reinterpret_cast<char *>(_trusted_memory) +
                  space_size_ref(_trusted_memory);
  if (block < _trusted_memory || block >= end_ptr || !is_occupied(block)) {
    throw std::logic_error(
        "Invalid pointer or double free in sorted_list allocator");
  }

  void *curr = first_free_ref(_trusted_memory);
  void *prev = nullptr;

  while (curr && curr < block) {
    prev = curr;
    curr = next_free_ref(curr);
  }

  set_free(block, curr);
  if (prev)
    set_free(prev, block);
  else
    first_free_ref(_trusted_memory) = block;

  if (curr) {
    void *block_end = reinterpret_cast<char *>(block) + block_metadata_size +
                      block_size_ref(block);
    if (block_end == curr) {
      block_size_ref(block) += block_metadata_size + block_size_ref(curr);
      set_free(block, next_free_ref(curr));
    }
  }

  if (prev) {
    void *prev_end = reinterpret_cast<char *>(prev) + block_metadata_size +
                     block_size_ref(prev);
    if (prev_end == block) {
      block_size_ref(prev) += block_metadata_size + block_size_ref(block);
      set_free(prev, next_free_ref(block));
    }
  }
}

inline void
allocator_sorted_list::set_fit_mode(allocator_with_fit_mode::fit_mode mode) {
  std::lock_guard<std::mutex> lock(mutex_ref(_trusted_memory));
  fit_mode_ref(_trusted_memory) = mode;
}

std::vector<allocator_test_utils::block_info>
allocator_sorted_list::get_blocks_info() const noexcept {
  std::lock_guard<std::mutex> lock(mutex_ref(_trusted_memory));
  return get_blocks_info_inner();
}

std::vector<allocator_test_utils::block_info>
allocator_sorted_list::get_blocks_info_inner() const {
  std::vector<allocator_test_utils::block_info> result;
  void *curr =
      reinterpret_cast<char *>(_trusted_memory) + allocator_metadata_size;
  void *end = reinterpret_cast<char *>(_trusted_memory) +
              space_size_ref(_trusted_memory);

  while (curr < end) {
    result.push_back({block_size_ref(curr), is_occupied(curr)});
    curr = reinterpret_cast<char *>(curr) + block_metadata_size +
           block_size_ref(curr);
  }
  return result;
}

allocator_sorted_list::sorted_free_iterator
allocator_sorted_list::free_begin() const noexcept {
  return sorted_free_iterator(_trusted_memory);
}

allocator_sorted_list::sorted_free_iterator
allocator_sorted_list::free_end() const noexcept {
  return sorted_free_iterator();
}

allocator_sorted_list::sorted_iterator
allocator_sorted_list::begin() const noexcept {
  return sorted_iterator(_trusted_memory);
}

allocator_sorted_list::sorted_iterator
allocator_sorted_list::end() const noexcept {
  return sorted_iterator();
}

bool allocator_sorted_list::sorted_free_iterator::operator==(
    const allocator_sorted_list::sorted_free_iterator &other) const noexcept {
  return _free_ptr == other._free_ptr;
}

bool allocator_sorted_list::sorted_free_iterator::operator!=(
    const allocator_sorted_list::sorted_free_iterator &other) const noexcept {
  return !(*this == other);
}

allocator_sorted_list::sorted_free_iterator &
allocator_sorted_list::sorted_free_iterator::operator++() & noexcept {
  if (_free_ptr) {
    _free_ptr = next_free_ref(_free_ptr);
  }
  return *this;
}

allocator_sorted_list::sorted_free_iterator
allocator_sorted_list::sorted_free_iterator::operator++(int n) {
  auto temp = *this;
  ++(*this);
  return temp;
}

size_t allocator_sorted_list::sorted_free_iterator::size() const noexcept {
  return _free_ptr ? block_size_ref(_free_ptr) : 0;
}

void *allocator_sorted_list::sorted_free_iterator::operator*() const noexcept {
  return _free_ptr ? reinterpret_cast<char *>(_free_ptr) + block_metadata_size
                   : nullptr;
}

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator()
    : _free_ptr(nullptr) {}

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator(
    void *trusted) {
  _free_ptr = trusted ? first_free_ref(trusted) : nullptr;
}

bool allocator_sorted_list::sorted_iterator::operator==(
    const allocator_sorted_list::sorted_iterator &other) const noexcept {
  return _current_ptr == other._current_ptr;
}

bool allocator_sorted_list::sorted_iterator::operator!=(
    const allocator_sorted_list::sorted_iterator &other) const noexcept {
  return !(*this == other);
}

allocator_sorted_list::sorted_iterator &
allocator_sorted_list::sorted_iterator::operator++() & noexcept {
  if (_current_ptr) {
    _current_ptr = reinterpret_cast<char *>(_current_ptr) +
                   block_metadata_size + block_size_ref(_current_ptr);
    void *end = reinterpret_cast<char *>(_trusted_memory) +
                space_size_ref(_trusted_memory);
    if (_current_ptr >= end)
      _current_ptr = nullptr;
  }
  return *this;
}

allocator_sorted_list::sorted_iterator
allocator_sorted_list::sorted_iterator::operator++(int n) {
  auto temp = *this;
  ++(*this);
  return temp;
}

size_t allocator_sorted_list::sorted_iterator::size() const noexcept {
  return _current_ptr ? block_size_ref(_current_ptr) : 0;
}

void *allocator_sorted_list::sorted_iterator::operator*() const noexcept {
  return _current_ptr
             ? reinterpret_cast<char *>(_current_ptr) + block_metadata_size
             : nullptr;
}

allocator_sorted_list::sorted_iterator::sorted_iterator()
    : _free_ptr(nullptr), _current_ptr(nullptr), _trusted_memory(nullptr) {}

allocator_sorted_list::sorted_iterator::sorted_iterator(void *trusted)
    : _free_ptr(nullptr), _current_ptr(nullptr), _trusted_memory(trusted) {
  if (trusted) {
    _current_ptr = reinterpret_cast<char *>(trusted) + allocator_metadata_size;
    void *end = reinterpret_cast<char *>(trusted) + space_size_ref(trusted);
    if (_current_ptr >= end)
      _current_ptr = nullptr;
  } else {
    _current_ptr = nullptr;
  }
}

bool allocator_sorted_list::sorted_iterator::occupied() const noexcept {
  return _current_ptr ? is_occupied(_current_ptr) : false;
}
