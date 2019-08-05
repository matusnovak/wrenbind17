#pragma once

#include <vector>
#include <list>
#include "module.hpp"

namespace wrenbind17 {
    template<typename T>
    class StdVectorBindings {
    public:
        typedef typename std::vector<T>::iterator Iterator;
        typedef typename std::vector<T> Vector;

        static void setIndex(Vector& self, size_t index, T value) {
            self[index] = std::move(value);
        }

        static const T& getIndex(Vector& self, size_t index) {
            return self[index];
        }

        static void add(Vector& self, T value) {
            self.push_back(std::move(value));
        }

        static std::variant<bool, Iterator> iterate(Vector& self, std::variant<std::nullptr_t, Iterator> other) {
            if (other.index() == 1) {
                auto it = std::get<Iterator>(other);
                ++it;
                if (it != self.end()) {
                    return {it};
                }

                return {false};
            } else {
                return {self.begin()};
            }
        }

        static const T& iteratorValue(Vector& self, std::shared_ptr<Iterator> other) {
            auto& it = *other;
            return *it;
        }

        static size_t count(Vector& self) {
            return self.size();
        }

        static T removeAt(Vector& self, long index) {
            if (index == -1) {
                auto ret = std::move(self.back());
                self.pop_back();
                return std::move(ret);
            } else {
                if (index < 0) {
                    index = self.size() + index;
                }

                if (size_t(index) > self.size()) {
                    throw std::out_of_range("invalid index");
                } else if (index == self.size()) {
                    auto ret = std::move(self.back());
                    self.pop_back();
                    return std::move(ret);
                } else {
                    auto ret = std::move(self.at(index));
                    self.erase(self.begin() + index);
                    return std::move(ret);
                }
            }
        }

        static void insert(Vector& self, long index, T value) {
            if (index == -1) {
                self.push_back(std::move(value));
            } else {
                if (index < 0) {
                    index = self.size() + index;
                }

                if (size_t(index) > self.size()) {
                    throw std::out_of_range("invalid index");
                } else if (index == self.size()) {
                    self.push_back(std::move(value));
                } else {
                    auto it = self.begin() + index;
                    self.insert(it, std::move(value));
                }
            }
        }

        static bool contains(Vector& self, const T& value) {
            return std::find(self.begin(), self.end(), value) != self.end();
        }

        static T pop(Vector& self) {
            auto ret = std::move(self.back());
            self.pop_back();
            return std::move(ret);
        }

        static void clear(Vector& self) {
            self.clear();
        }

        static size_t size(Vector& self) {
            return self.size();
        }

        static void bind(ForeignModule& m, const std::string& name) {
            auto& iter = m.klass<Iterator>(name + "Iter");
            iter.ctor();

            auto& cls = m.klass<Vector>(name);
            cls.ctor();
            
            cls.template funcExt<&StdVectorBindings<T>::getIndex>(OPERATOR_GET_INDEX);
            cls.template funcExt<&StdVectorBindings<T>::setIndex>(OPERATOR_SET_INDEX);
            cls.template funcExt<&StdVectorBindings<T>::add>("add");
            cls.template funcExt<&StdVectorBindings<T>::iterate>("iterate");
            cls.template funcExt<&StdVectorBindings<T>::iteratorValue>("iteratorValue");
            cls.template funcExt<&StdVectorBindings<T>::removeAt>("removeAt");
            cls.template funcExt<&StdVectorBindings<T>::insert>("insert");
            cls.template funcExt<&StdVectorBindings<T>::contains>("contains");
            cls.template funcExt<&StdVectorBindings<T>::pop>("pop");
            cls.template funcExt<&StdVectorBindings<T>::clear>("clear");
            cls.template funcExt<&StdVectorBindings<T>::size>("size");
            cls.template propReadonlyExt<&StdVectorBindings<T>::count>("count");
        }
    };

    template<typename T>
    class StdListBindings {
    public:
        typedef typename std::list<T>::iterator Iterator;
        typedef typename std::list<T> List;

        static void setIndex(List& self, size_t index, T value) {
            auto it = self.begin();
            std::advance(it, index);
            *it = std::move(value);
        }

        static const T& getIndex(List& self, size_t index) {
            auto it = self.begin();
            std::advance(it, index);
            return *it;
        }

        static void add(List& self, T value) {
            self.push_back(std::move(value));
        }

        static std::variant<bool, Iterator> iterate(List& self, std::variant<std::nullptr_t, Iterator> other) {
            if (other.index() == 1) {
                auto it = std::get<Iterator>(other);
                ++it;
                if (it != self.end()) {
                    return {it};
                }

                return {false};
            } else {
                return {self.begin()};
            }
        }

        static const T& iteratorValue(List& self, std::shared_ptr<Iterator> other) {
            auto& it = *other;
            return *it;
        }

        static size_t count(List& self) {
            return self.size();
        }

        static T removeAt(List& self, long index) {
            if (index == -1) {
                auto ret = std::move(self.back());
                self.pop_back();
                return std::move(ret);
            } else {
                if (index < 0) {
                    index = self.size() + index;
                }

                if (size_t(index) > self.size()) {
                    throw std::out_of_range("invalid index");
                } else if (index == self.size()) {
                    auto ret = std::move(self.back());
                    self.pop_back();
                    return std::move(ret);
                } else {
                    auto it = self.begin();
                    std::advance(it, index);
                    auto ret = std::move(*it);
                    self.erase(it);
                    return std::move(ret);
                }
            }
        }

        static void insert(List& self, long index, T value) {
            if (index == -1) {
                self.push_back(std::move(value));
            } else {
                if (index < 0) {
                    index = self.size() + index;
                }

                if (size_t(index) > self.size()) {
                    throw std::out_of_range("invalid index");
                } else if (index == self.size()) {
                    self.push_back(std::move(value));
                } else {
                    auto it = self.begin();
                    std::advance(it, index);
                    self.insert(it, std::move(value));
                }
            }
        }

        static bool contains(List& self, const T& value) {
            return std::find(self.begin(), self.end(), value) != self.end();
        }

        static T pop(List& self) {
            auto ret = std::move(self.back());
            self.pop_back();
            return std::move(ret);
        }

        static void clear(List& self) {
            self.clear();
        }

        static size_t size(List& self) {
            return self.size();
        }

        static void bind(ForeignModule& m, const std::string& name) {
            auto& iter = m.klass<Iterator>(name + "Iter");
            iter.ctor();

            auto& cls = m.klass<List>(name);
            cls.ctor();
            
            cls.template funcExt<&StdListBindings<T>::getIndex>(OPERATOR_GET_INDEX);
            cls.template funcExt<&StdListBindings<T>::setIndex>(OPERATOR_SET_INDEX);
            cls.template funcExt<&StdListBindings<T>::add>("add");
            cls.template funcExt<&StdListBindings<T>::iterate>("iterate");
            cls.template funcExt<&StdListBindings<T>::iteratorValue>("iteratorValue");
            cls.template funcExt<&StdListBindings<T>::removeAt>("removeAt");
            cls.template funcExt<&StdListBindings<T>::insert>("insert");
            cls.template funcExt<&StdListBindings<T>::contains>("contains");
            cls.template funcExt<&StdListBindings<T>::pop>("pop");
            cls.template funcExt<&StdListBindings<T>::clear>("clear");
            cls.template funcExt<&StdListBindings<T>::size>("size");
            cls.template propReadonlyExt<&StdListBindings<T>::count>("count");
        }
    };
} // namespace wrenbind17
