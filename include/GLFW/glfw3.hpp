#ifndef _glfw3_hpp_
#define _glfw3_hpp_

#ifndef GLFW_HPP_NAMESPACE
#define GLFW_HPP_NAMESPACE glfw
#endif

#ifdef GLFW_HPP_NO_STD_STRING
#define GLFW_HPP_STRING char const *
#else
#define GLFW_HPP_STRING std::string
#endif

#include <GLFW/glfw3.h>

#include <functional>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

namespace GLFW_HPP_NAMESPACE {

    namespace _private {
#ifdef GLFW_HPP_NO_STD_STRING
        inline GLFW_HPP_STRING makeString_(char const *str) { return str; }
        inline char const *cString_(GLFW_HPP_STRING const &str) { return str; }
#else
        inline GLFW_HPP_STRING makeString_(char const *str) { return std::string(str); }
        inline char const *cString_(GLFW_HPP_STRING const &str) { return str.c_str(); }
#endif
    }

    using ErrorFun = GLFWerrorfun;

    struct Size {
        int width;
        int height;
    };

    enum class ErrorCode {
        noError = GLFW_NO_ERROR,
        notInitialized = GLFW_NOT_INITIALIZED,
        noCurrentContext = GLFW_NO_CURRENT_CONTEXT,
        invalidEnum = GLFW_INVALID_ENUM,
        invalidValue = GLFW_INVALID_VALUE,
        outOfMemory = GLFW_OUT_OF_MEMORY,
        apiUnavailable = GLFW_API_UNAVAILABLE,
        versionUnavailable = GLFW_VERSION_UNAVAILABLE,
        platformError = GLFW_PLATFORM_ERROR,
        formatUnavailable = GLFW_FORMAT_UNAVAILABLE,
        noWindowContext = GLFW_NO_WINDOW_CONTEXT,
        cursorUnavailable = GLFW_CURSOR_UNAVAILABLE,
        featureUnavailable = GLFW_FEATURE_UNAVAILABLE,
        featureUnimplemented = GLFW_FEATURE_UNIMPLEMENTED,
    };

}

namespace std {
    template <>
    struct is_error_code_enum<GLFW_HPP_NAMESPACE::ErrorCode> : public true_type {};
}

namespace GLFW_HPP_NAMESPACE {

    class ErrorCategory : public std::error_category {
    public:
        const char *name() const _NOEXCEPT override {
            return nullptr;
        }

        std::string message(int __ev) const override {
            return std::string();
        }
    };

    inline std::error_code make_error_code(ErrorCode code) {
        return {static_cast<int>(code), ErrorCategory()};
    }

    class Error : public std::system_error {
    public:
        ErrorCode const code;

    private:
        Error(ErrorCode const code, GLFW_HPP_STRING const &description) :
                std::system_error(std::error_code{code}, description),
                code{code} {}

        static std::optional<Error> get_() {
            char const *description;
            auto code = static_cast<ErrorCode const>(glfwGetError(&description));

            if (code == ErrorCode::noError) {
                return std::optional<Error>{};
            }

            return Error(code, description ? _private::makeString_(description) : "");
        }

        static void getAndThrow_() {
            auto const err = get_();
            if (err) {
                throw err.value();
            }
        }

        friend class GLFW;
        friend class Window;
    };

    class Window {
    public:
        Window(Size const &size, GLFW_HPP_STRING const &title) :
                size_{size},
                title_{title},
                handle_{glfwCreateWindow(size.width, size.height, _private::cString_(title), nullptr, nullptr)} {
            Error::getAndThrow_();
        }

        void makeContextCurrent() const {
            glfwMakeContextCurrent(this->handle_);
        }

        [[nodiscard]] bool shouldClose() const {
            return glfwWindowShouldClose(this->handle_);
        }

        void swapBuffers() const {
            glfwSwapBuffers(this->handle_);
        }

        GLFWwindow *handle_;
    private:

        Size size_;
        GLFW_HPP_STRING title_;
    };

    class GLFW {
    public:
        GLFW() {
            if (!glfwInit()) {
                Error::getAndThrow_();
            }
        }

        explicit GLFW(ErrorFun errorCallback) : GLFW() {
            this->errorCallback = errorCallback;
            glfwSetErrorCallback(this->errorCallback);
        }

        virtual ~GLFW() {
            glfwTerminate();
        }

        Window createWindow(Size const &size, GLFW_HPP_STRING const &title) {
            return Window{size, title};
        }

        void swapInterval(int const interval) {
            glfwSwapInterval(interval);
        }

        void waitEvents() {
            glfwWaitEvents();
        }

    private:
        ErrorFun errorCallback{};
    };

}


#endif /* _glfw3_h_ */
