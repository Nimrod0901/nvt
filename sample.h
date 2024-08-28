#pragma once

#include <tuple>
#include <type_traits>
#include <utility>
#include <iostream>
#include <vector>

namespace nvt {

template <typename T>
class Shape {
public:
    int Area() const {
        return static_cast<const T&>(*this).AreaImpl();
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

static_assert(index_of_v<int, int, double> == 0, "");
static_assert(index_of_v<double, int, double> == 1, "");
static_assert(index_of_v<float, int, double> == 2, ""); // equal to the type list length

} // namespace detail


template <typename... ShapeTs>
class ShapeManager {
public:
    struct Index {
        std::size_t type_index;
        std::size_t container_index;
    };

    template <typename Shape>
    // requires requires { detail::has_type_v<Shape, ShapeTs...>; }
    Index Add(Shape&& shape);

    void Dispatch(const Index&);

    void ForEach();
private:
    std::tuple<std::vector<ShapeTs>...> shapes;
};
//
template <typename... ShapeTs>
template <typename Shape>
// requires requires { detail::has_type_v<Shape, ShapeTs...>; }
typename ShapeManager<ShapeTs...>::Index ShapeManager<ShapeTs...>::Add(Shape&& shape) {
    Index index;
    constexpr std::size_t type_index = detail::index_of_v<std::remove_cv_t<Shape>, ShapeTs...>;
    index.type_index = type_index;
    index.container_index = std::get<type_index>(shapes).size();
    std::get<type_index>(shapes).push_back(shape);
    return index;
}

template <typename... ShapeTs>
void ShapeManager<ShapeTs...>::Dispatch(const Index& index) {
    detail::invoke_at(shapes, index.type_index, [container_index=index.container_index](auto& sub_shapes) {
        std::cout << sub_shapes.at(container_index).Area() << "\n";
    });
}

template <typename... ShapeTs>
void ShapeManager<ShapeTs...>::ForEach() {
    std::apply([](auto&&... sub_shapes) {
        ((std::for_each(sub_shapes.begin(), sub_shapes.end(), [](const auto& shape){ std::cout << shape.Area() << "\n";})), ...);
    }, shapes);
}

} // namespace nvt
