// lesson 08 - type erasure for trooth

#include <iostream>
#include <fstream>
#include <utility>
#include <memory>
#include <functional>

void hello_world() {
  std::cout << "Hello world!\n";
}

struct output_to_file {
  std::unique_ptr<std::ofstream> out;
  void operator()() {
    (*out.get()) << "Hello world\n";
  }
};

struct nullary_function_concept {
  virtual void call() = 0;
  virtual ~nullary_function_concept() {}
};

template <typename F>
struct nullary_function_model : nullary_function_concept {
  nullary_function_model(F x) : f(std::move(x)) {}
  F f;
  void call() override {
    f();
  }
};

struct nullary_function {
  void operator()() {
    inner_function->call();
  }
  std::unique_ptr<nullary_function_concept> inner_function;
};

template <typename F>
auto make_nullary_function(F x) -> nullary_function {
  return {std::make_unique<nullary_function_model<F>>(std::move(x))};
}

template <typename ReturnType, typename... Args>
struct function_concept {
  virtual ReturnType call(Args... args) = 0;
  virtual std::unique_ptr<function_concept> clone() const = 0;
  virtual ~function_concept() {}
};

template <typename Callable, typename ReturnType, typename... Args>
struct function_model : function_concept<ReturnType, Args...> {
  function_model(Callable x) : f(std::move(x)) {}
  Callable f;
  std::unique_ptr<function_model> clone() const {
    return std::make_unique<function_model>(*this);
  }
  ReturnType call(Args... args) override {
    return f(args...);
  }
};

template <typename Signature>
struct function;

template <typename ReturnType, typename... ArgumentTypes>
struct function<ReturnType(ArgumentTypes...)> {
  template <typename T>
  function(T x)
      : f{std::make_unique<function_model<T, ReturnType, ArgumentTypes...>>(
            std::move(x))} {}

  ReturnType operator()(ArgumentTypes... xs) {
    return f->call(xs...);
  }
  std::unique_ptr<function_concept<ReturnType, ArgumentTypes...>> f;
};

template <typename Signature, typename T>
auto make_function(T x) -> function<Signature> {
  return {std::move(x)};
};

int main() {
  nullary_function x = make_nullary_function(hello_world);
  output_to_file fout{std::make_unique<std::ofstream>("foo")};
  nullary_function y = make_nullary_function(std::move(fout));
  // using fout for ANYTHING OTHER THAN DESTRUCTION AND ASSIGNMENT TO
  // IS NOT GUARANTEED TO WORK
  x();
  y();

  std::function<void()> f = hello_world;
  std::function<void()> f1 = std::move(x);
  std::function<void()> f2 = std::move(y);
  std::function<void()> f3 = f2;
  f();
}

