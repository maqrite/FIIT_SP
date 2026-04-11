#ifndef PP_ALLOCATOR_HPP
#define PP_ALLOCATOR_HPP

#include <cstddef>

template <typename T> class pp_allocator {
public:
  using value_type = T;

  pp_allocator() noexcept = default;

  template <typename U>
  constexpr pp_allocator(const pp_allocator<U> &) noexcept {}

  template <typename U> struct rebind {
    using other = pp_allocator<U>;
  };

  [[nodiscard]] T *allocate(std::size_t n) {
    return static_cast<T *>(::operator new(n * sizeof(T)));
  }

  void deallocate(T *p, std::size_t n) noexcept { ::operator delete(p); }

  template <typename U>
  friend bool operator==(const pp_allocator &,
                         const pp_allocator<U> &) noexcept {
    return true;
  }
};

#endif
