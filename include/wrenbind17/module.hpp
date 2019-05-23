#pragma once

#include <sstream>
#include "foreign.hpp"

/**
 * @ingroup wrenbind17
 */
namespace wrenbind17 {
    /**
     * @ingroup wrenbind17
     */
    class ForeignModule {
    public:
        ForeignModule(std::string name, WrenVM* vm) : name(std::move(name)), vm(vm) {
        }
        ForeignModule(const ForeignModule& other) = delete;
        ForeignModule(ForeignModule&& other) noexcept : vm(nullptr) {
            swap(other);
        }
        ~ForeignModule() = default;
        ForeignModule& operator=(const ForeignModule& other) = delete;
        ForeignModule& operator=(ForeignModule&& other) noexcept {
            if (this != &other) {
                swap(other);
            }
            return *this;
        }
        void swap(ForeignModule& other) {
            std::swap(klasses, other.klasses);
            std::swap(vm, other.vm);
            std::swap(name, other.name);
        }

        template <typename T>
        ForeignKlassImpl<T>& klass(std::string name) {
            auto ptr = std::make_unique<ForeignKlassImpl<T>>(std::move(name));
            auto ret = ptr.get();
            addClassType(vm, this->name, ptr->getName(), typeid(T).hash_code());
            klasses.insert(std::make_pair(ptr->getName(), std::move(ptr)));
            return *ret;
        }

        std::string str() const {
            std::stringstream ss;
            for (const auto& pair : klasses) {
                pair.second->generate(ss);
            }
            return ss.str();
        }

        ForeignKlass& findKlass(const std::string& name) {
            auto it = klasses.find(name);
            if (it == klasses.end())
                throw NotFound();
            return *it->second;
        }

        const std::string& getName() const {
            return name;
        }

    private:
        std::string name;
        WrenVM* vm;
        std::unordered_map<std::string, std::unique_ptr<ForeignKlass>> klasses;
    };
} // namespace wrenbind17
