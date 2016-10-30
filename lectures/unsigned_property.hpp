#include <cstddef>
#include <utility>
#include <memory>

// this needs to be in this order, but start reading at unsigned_property.
namespace detail {
inline namespace { // hide these symbols from the linker

struct unsigned_concept {
  virtual unsigned operator_subscript(size_t index) const = 0;
  virtual ~unsigned_concept() {}  // make sure destruction works properly
};

template <typename T>
struct unsigned_model : unsigned_concept {
  unsigned operator_subscript(size_t index) const override final {
    return value[index];
  }
  T value;

  // need value constructor to construct
  unsigned_model(T value) : value(std::move(value)) {}
};

}
} // detail

struct unsigned_property {
  unsigned operator[](size_t index) const {
    return handle->operator_subscript(index);
  }
  std::unique_ptr<detail::unsigned_concept> handle;

  // default construction, move construction, move assignment
  unsigned_property() = default; // enable default construction
  unsigned_property(unsigned_property&&) = default; // move construct
  unsigned_property& operator=(unsigned_property&&) = default; // move assign

  // the *magic* of making a new model - construction
  template <typename T>
  explicit unsigned_property(T value)
      : handle(std::make_unique<detail::unsigned_model<T>>(std::move(value))) {}
  // assignment
  template <typename T>
  unsigned_property& operator=(T value) {
    *this = unsigned_property(std::move(value));
    return *this;
  }
};

