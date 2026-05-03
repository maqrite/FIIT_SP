#include "../include/allocator_global_heap.h"
#include <cstddef>
#include <memory_resource>
#include <stdexcept>

allocator_global_heap::allocator_global_heap() {}

[[nodiscard]] void *allocator_global_heap::do_allocate_sm(size_t size) {
  constexpr size_t MAGIC_COOKIE = 0xDEADBEEF;

  size_t total_size = size + 2 * size_t_size;
  void *ptr = ::operator new(total_size);

  size_t *meta = static_cast<size_t *>(ptr);
  meta[0] = MAGIC_COOKIE;
  meta[1] = size;

  return meta + 2;
}

void allocator_global_heap::do_deallocate_sm(void *at) {
  if (at == nullptr) {
    return;
  }

  constexpr size_t MAGIC_COOKIE = 0xDEADBEEF;

  size_t *meta = static_cast<size_t *>(at) - 2;

  if (meta[0] != MAGIC_COOKIE) {
    throw std::logic_error("Segmentation Fault pobezhden");
  }

  meta[0] = 0;

  ::operator delete(meta);
}

allocator_global_heap::~allocator_global_heap() {}

allocator_global_heap::allocator_global_heap(
    const allocator_global_heap &other) {}

allocator_global_heap &
allocator_global_heap::operator=(const allocator_global_heap &other) {
  return *this;
}

bool allocator_global_heap::do_is_equal(
    const std::pmr::memory_resource &other) const noexcept {
  return dynamic_cast<const allocator_global_heap *>(&other) != nullptr;
}

allocator_global_heap::allocator_global_heap(
    allocator_global_heap &&other) noexcept {}

allocator_global_heap &
allocator_global_heap::operator=(allocator_global_heap &&other) noexcept {
  return *this;
}
