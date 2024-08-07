#pragma once

#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace nvt {

template <typename T>
class Shape {
public:
    int Area() const {
        return static_cast<Shape&>(*this)->AreaImpl();
    }
};

class Circle : public Shape<Circle> {
public:
    Circle(int r): radius_{r} {}
    int AreaImpl() const {
        constexpr int pi = 3;
        return radius_ * radius_ * pi;
    }
private:
    int radius_;
};

class Rectangle : public Shape<Rectangle> {
public:
    Rectangle(int width, int length): width_{width}, length_{length} {}
    int AreaImpl() const {
        return width_ * length_;
    }
private:
    int width_;
    int length_;
};


namespace detail {

template <typename TPred, typename... Ts, std::size_t... Is>
inline void invoke_at_impl(std::tuple<Ts...>& tpl,
                           std::index_sequence<Is...>,
                           std::size_t idx,
                           TPred pred) {
    ((void)(Is == idx && (pred(std::get<Is>(tpl)), true)), ...);
}

template <typename TPred, typename... Ts>
inline void invoke_at(std::tuple<Ts...>& tpl, std::size_t idx, TPred pred) {
    if (idx >= sizeof...(Ts)) {
        throw std::out_of_range("Index out of range");
    }
    invoke_at_impl(tpl, std::make_index_sequence<sizeof...(Ts)>{}, idx, pred);
}

template <typename T, typename... Ts>
inline constexpr bool has_type_v = std::disjunction_v<std::is_same<T, Ts>...>;

// type to index
template <typename T, typename... Ts>
struct IndexOf;

template <typename T, typename... Ts>
constexpr std::size_t index_of_v = IndexOf<T, Ts...>::value;

template <typename T, typename... Ts>
struct IndexOf<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

template <typename T, typename U, typename... Ts>
struct IndexOf<T, U, Ts...> : std::integral_constant<std::size_t, index_of_v<T, Ts...> + 1> {};

template <typename T>
struct IndexOf<T> : std::integral_constant<std::size_t, 0> {};

static_assert(index_of_v<int, int, double> == 0);
static_assert(index_of_v<double, int, double> == 1);
static_assert(index_of_v<float, int, double> == 2); // equal to the type list length

} // namespace detail


// Sequence container
template <typename... ShapeTs>
class ShapeManagerV {
public:
    struct Index {
        std::size_t type_index;
        std::size_t container_index;
    };

    template <typename Shape>
    Index Add();

    void Dispatch(Index);
    void ForEach();
private:
    std::tuple<std::vector<ShapeTs>...> shapes;
};

} // namespace nvt
