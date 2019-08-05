#pragma once

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <unordered_map>
#include <vector>
#include "module.hpp"
#include "variable.hpp"
#include "std.hpp"

namespace std {
    template <> struct hash<std::pair<size_t, size_t>> {
        inline size_t operator()(const std::pair<size_t, size_t>& v) const {
            const std::hash<size_t> hasher;
            return hasher(v.first) ^ hasher(v.second);
        }
    };
} // namespace std

/**
 * @ingroup wrenbind17
 */
namespace wrenbind17 {
    /**
     * @ingroup wrenbind17
     */
    typedef std::function<void(const char*)> PrintFn;

    /**
     * @ingroup wrenbind17
     */
    typedef std::function<std::string(const std::vector<std::string>& paths, const std::string& name)> LoadFileFn;

    /**
     * @ingroup wrenbind17
     */
    class VM {
    public:
        inline explicit VM(std::vector<std::string> paths = {"./"}, const size_t initHeap = 1024 * 1024,
                           const size_t minHeap = 1024 * 1024 * 10, const int heapGrowth = 50)
            : vm(nullptr), paths(std::move(paths)) {

            printFn = [](const char* text) -> void { std::cout << text; };
            loadFileFn = [](const std::vector<std::string>& paths, const std::string& name) -> std::string {
                for (const auto& path : paths) {
                    const auto test = path + "/" + std::string(name) + ".wren";

                    std::ifstream t(test);
                    if (!t)
                        continue;

                    std::string source((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
                    return source;
                }

                throw NotFound();
            };

            wrenInitConfiguration(&config);
            config.initialHeapSize = initHeap;
            config.minHeapSize = minHeap;
            config.heapGrowthPercent = heapGrowth;
            config.userData = this;
            config.reallocateFn = std::realloc;
            config.loadModuleFn = [](WrenVM* vm, const char* name) -> char* {
                auto& self = *reinterpret_cast<VM*>(wrenGetUserData(vm));

                const auto mod = self.modules.find(name);
                if (mod != self.modules.end()) {
                    auto source = mod->second.str();
                    auto buffer = new char[source.size() + 1];
                    std::memcpy(buffer, &source[0], source.size() + 1);
                    return buffer;
                }

                try {
                    auto source = self.loadFileFn(self.paths, std::string(name));
                    auto buffer = new char[source.size() + 1];
                    std::memcpy(buffer, &source[0], source.size() + 1);
                    return buffer;
                } catch (std::exception& e) {
                    (void)e;
                    return nullptr;
                }
            };
            config.bindForeignMethodFn = [](WrenVM* vm, const char* module, const char* className, const bool isStatic,
                                            const char* signature) -> WrenForeignMethodFn {
                auto& self = *reinterpret_cast<VM*>(wrenGetUserData(vm));
                try {
                    auto& found = self.modules.at(module);
                    auto& klass = found.findKlass(className);
                    return klass.findSignature(signature, isStatic);
                } catch (...) {
                    std::cerr << "Wren foreign method " << signature << " not found in C++" << std::endl;
                    std::abort();
                    return nullptr;
                }
            };
            config.bindForeignClassFn = [](WrenVM* vm, const char* module,
                                           const char* className) -> WrenForeignClassMethods {
                auto& self = *reinterpret_cast<VM*>(wrenGetUserData(vm));
                try {
                    auto& found = self.modules.at(module);
                    auto& klass = found.findKlass(className);
                    return klass.getAllocators();
                } catch (...) {
                    exceptionHandler(vm, std::current_exception());
                    return WrenForeignClassMethods{nullptr, nullptr};
                }
            };
            config.writeFn = [](WrenVM* vm, const char* text) {
                auto& self = *reinterpret_cast<VM*>(wrenGetUserData(vm));
                self.printFn(text);
            };
            config.errorFn = [](WrenVM* vm, WrenErrorType type, const char* module, const int line,
                                const char* message) {
                auto& self = *reinterpret_cast<VM*>(wrenGetUserData(vm));
                std::stringstream ss;
                switch (type) {
                    case WREN_ERROR_COMPILE:
                        ss << "Compile error: " << message << " at " << module << ":" << line << "\n";
                        break;
                    case WREN_ERROR_RUNTIME:
                        ss << "Runtime error: " << message << "\n";
                        break;
                    case WREN_ERROR_STACK_TRACE:
                        ss << "  at: " << module << ":" << line << "\n";
                        break;
                    default:
                        break;
                }
                self.lastError += ss.str();
            };

            vm = wrenNewVM(&config);
        }

        inline VM(const VM& other) = delete;

        inline VM(VM&& other) noexcept : vm(nullptr) {
            swap(other);
        }

        inline ~VM() {
            if (vm) {
                wrenFreeVM(vm);
            }
        }

        inline VM& operator=(const VM& other) = delete;

        inline VM& operator=(VM&& other) noexcept {
            if (this != &other) {
                swap(other);
            }
            return *this;
        }

        inline void swap(VM& other) noexcept {
            std::swap(vm, other.vm);
            std::swap(config, other.config);
            std::swap(paths, other.paths);
            std::swap(modules, other.modules);
            std::swap(classToModule, other.classToModule);
            std::swap(classToName, other.classToName);
        }

        inline void runFromSource(const std::string& name, const std::string& code) {
            const auto result = wrenInterpret(vm, name.c_str(), code.c_str());
            if (result != WREN_RESULT_SUCCESS) {
                getLastError();
            }
            return;
        }

        inline void runFromFile(const std::string& name, const std::string& path) {
            std::ifstream t(path);
            if (!t)
                throw Exception("Compile error: Failed to open source file");
            std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
            runFromSource(name, str);
        }

        inline void runFromModule(const std::string& name) {
            const auto source = loadFileFn(paths, name);
            runFromSource(name, source);
        }

        inline Variable find(const std::string& module, const std::string& name) {
            wrenEnsureSlots(vm, 1);
            wrenGetVariable(vm, module.c_str(), name.c_str(), 0);
            auto* handle = wrenGetSlotHandle(vm, 0);
            if (!handle)
                throw NotFound();
            return Variable(vm, std::make_shared<Handle>(vm, handle));
        }

        inline ForeignModule& module(const std::string& name) {
            auto it = modules.find(name);
            if (it == modules.end()) {
                it = modules.insert(std::make_pair(name, ForeignModule(name, vm))).first;
            }
            return it->second;
        }

        inline void addClassType(const std::string& module, const std::string& name, const size_t hash) {
            classToModule.insert(std::make_pair(hash, module));
            classToName.insert(std::make_pair(hash, name));
        }

        inline void getClassType(std::string& module, std::string& name, const size_t hash) {
            module = classToModule.at(hash);
            name = classToName.at(hash);
        }

        inline void addClassCast(std::shared_ptr<detail::ForeignPtrConvertor> convertor, const size_t hash,
                                 const size_t other) {
            classCasting.insert(std::make_pair(std::make_pair(hash, other), std::move(convertor)));
        }

        inline detail::ForeignPtrConvertor* getClassCast(const size_t hash, const size_t other) {
            return classCasting.at(std::pair(hash, other)).get();
        }

        inline void getLastError() {
            auto e = std::runtime_error(lastError);
            lastError.clear();
            throw e;
        }

        inline void setPrintFunc(const PrintFn& fn) {
            printFn = fn;
        }

        inline void setLoadFileFunc(const LoadFileFn& fn) {
            loadFileFn = fn;
        }

        inline WrenVM* getVm() const {
            return vm;
        }

        inline void gc() {
            wrenCollectGarbage(vm);
        }

    private:
        WrenVM* vm;
        WrenConfiguration config;
        std::vector<std::string> paths;
        std::unordered_map<std::string, ForeignModule> modules;
        std::unordered_map<size_t, std::string> classToModule;
        std::unordered_map<size_t, std::string> classToName;
        std::unordered_map<std::pair<size_t, size_t>, std::shared_ptr<detail::ForeignPtrConvertor>> classCasting;
        std::string lastError;
        PrintFn printFn;
        LoadFileFn loadFileFn;
    };

    inline void addClassType(WrenVM* vm, const std::string& module, const std::string& name, const size_t hash) {
        auto self = reinterpret_cast<VM*>(wrenGetUserData(vm));
        self->addClassType(module, name, hash);
    }
    inline void getClassType(WrenVM* vm, std::string& module, std::string& name, const size_t hash) {
        auto self = reinterpret_cast<VM*>(wrenGetUserData(vm));
        self->getClassType(module, name, hash);
    }
    inline void addClassCast(WrenVM* vm, std::shared_ptr<detail::ForeignPtrConvertor> convertor, const size_t hash,
                             const size_t other) {
        auto self = reinterpret_cast<VM*>(wrenGetUserData(vm));
        self->addClassCast(std::move(convertor), hash, other);
    }
    inline detail::ForeignPtrConvertor* getClassCast(WrenVM* vm, const size_t hash, const size_t other) {
        auto self = reinterpret_cast<VM*>(wrenGetUserData(vm));
        return self->getClassCast(hash, other);
    }
    inline void getLastError(WrenVM* vm) {
        auto self = reinterpret_cast<VM*>(wrenGetUserData(vm));
        self->getLastError();
    }
} // namespace wrenbind17
