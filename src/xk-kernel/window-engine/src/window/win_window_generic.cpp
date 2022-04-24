//
// Created by kafka on 2/3/2022.
//

#include <window/win_window_generic.h>

namespace xk::win
{
    template<typename GraphicsApiHandler, bool IsMainWindow, bool IsValidationEnabled>
    GenericWindow<GraphicsApiHandler, IsMainWindow, IsValidationEnabled>::GenericWindow(std::weak_ptr<GraphicsApiHandler> graphicsEngine, u32 width, u32 height, std::string_view title)
            : m_graphicsEngine{ graphicsEngine }
            , m_title{ title }
            , m_width{ width }
            , m_height{ height }
            , m_window{ nullptr }
            , m_isShown{ false }
    {
        init();
    }

    template<typename GraphicsApiHandler, bool IsMainWindow, bool IsValidationEnabled>
    GenericWindow<GraphicsApiHandler, IsMainWindow, IsValidationEnabled>::~GenericWindow() noexcept
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    template<typename GraphicsApiHandler, bool IsMainWindow, bool IsValidationEnabled>
    void
    GenericWindow<GraphicsApiHandler, IsMainWindow, IsValidationEnabled>::close()
    {
        if(m_isShown)
        {
            if constexpr(IsValidationEnabled)
            {
                log::info("Closing window");
            }

            glfwWindowShouldClose(m_window);

            m_isShown = false;
        }
    }

    template<typename GraphicsApiHandler, bool IsMainWindow, bool IsValidationEnabled>
    void
    GenericWindow<GraphicsApiHandler, IsMainWindow, IsValidationEnabled>::init()
    {
        if constexpr(IsValidationEnabled)
        {
            log::info("Creating window with Width {} and Height {}", m_width, m_height);
        }

        glfwInit();

        if constexpr (IsVulkanUsed())
        {
            if(!glfwVulkanSupported())
            {
                throw Exception::WindowError("Vulkan library is not supported");
            }
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        if constexpr(IsOpenGl())
        {
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }

        m_window = glfwCreateWindow(static_cast<int>(m_width), static_cast<int>(m_height), m_title.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(m_window, this);

        //callback functions
        glfwSetFramebufferSizeCallback(m_window, frameBufferResizeCallback);
        glfwSetKeyCallback(m_window, keyHandlerCallBack);
        glfwSetWindowCloseCallback(m_window, windowCloseCallback);
    }

    template<typename GraphicsApiHandler, bool IsMainWindow, bool IsValidationEnabled>
    void
    GenericWindow<GraphicsApiHandler, IsMainWindow, IsValidationEnabled>::frameBufferResizeCallback(WindowEngine* window, int width, int height)
    {
        auto _this = static_cast<GenericWindow*>(glfwGetWindowUserPointer(window));

        _this->resizedEvent(static_cast<u32>(width), static_cast<u32>(height));
    }

    template<typename GraphicsApiHandler, bool IsMainWindow, bool IsValidationEnabled>
    void
    GenericWindow<GraphicsApiHandler, IsMainWindow, IsValidationEnabled>::keyHandlerCallBack(WindowEngine* window, int key, int scancode, int action, int mods)
    {
        auto _this = static_cast<GenericWindow*>(glfwGetWindowUserPointer(window));

        _this->keyPressedEvent(static_cast<Key>(key), scancode, action, mods);
    }

    template<typename GraphicsApiHandler, bool IsMainWindow, bool IsValidationEnabled>
    void
    GenericWindow<GraphicsApiHandler, IsMainWindow, IsValidationEnabled>::windowCloseCallback(WindowEngine* window)
    {
        auto _this = static_cast<GenericWindow*>(glfwGetWindowUserPointer(window));

        _this->closeEvent();
    }

    template<typename GraphicsApiHandler, bool IsMainWindow, bool IsValidationEnabled>
    void
    GenericWindow<GraphicsApiHandler, IsMainWindow, IsValidationEnabled>::resize(u32 width, u32 height)
    {
        //TODO check if this is not more then size of desktop ...
        m_width = width;
        m_height = height;

        glfwSetWindowSize(m_window, static_cast<int>(width), static_cast<int>(height));
    }

    template<typename GraphicsApiHandler, bool IsMainWindow, bool IsValidationEnabled>
    void
    GenericWindow<GraphicsApiHandler, IsMainWindow, IsValidationEnabled>::show()
    {
        if(!m_isShown)
        {
            if constexpr(IsValidationEnabled)
            {
                log::info("Popping up window");
            }

            glfwShowWindow(m_window);

            m_isShown = true;
        }
    }

    template<typename GraphicsApiHandler, bool IsMainWindow, bool IsValidationEnabled>
    void
    GenericWindow<GraphicsApiHandler, IsMainWindow, IsValidationEnabled>::showFullWindow()
    {

    }

    template<typename GraphicsApiHandler, bool IsMainWindow, bool IsValidationEnabled>
    VkSurfaceKHR
    GenericWindow<GraphicsApiHandler, IsMainWindow, IsValidationEnabled>::createVulkanWindowSurface(VkInstance vulkanInstance) const
    {
        if constexpr (IsValidationEnabled)
        {
            log::info("Creating vulkan window surface");
        }

        VkSurfaceKHR surface{};

        if (glfwCreateWindowSurface(vulkanInstance, m_window, nullptr, &surface) != VK_SUCCESS)
        {
            throw Exception::VulkanSurfaceError("Could not create Vulkan surface");
        }

        return surface;
    }


    template<typename GraphicsApiHandler, bool IsMainWindow, bool IsValidationEnabled>
    std::vector<const char*>
    GenericWindow<GraphicsApiHandler, IsMainWindow, IsValidationEnabled>::requiredWindowExtensions() const
    {
        if constexpr(IsValidationEnabled)
        {
            log::info("Obtaining required window extensions");
        }

        u32 extensionCount = 0;

        const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

        std::vector<const char*> returnValue(extensions, extensions + extensionCount);

        if constexpr(IsValidationEnabled)
        {
            returnValue.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

            log::info("Extensions:");

            for(const auto& name : returnValue)
            {
                log::info("{}", name);
            }
        }

        if(returnValue.empty())
        {
            throw Exception::WindowError("Could not obtain extensions");
        }

        return returnValue;
    }
    template<typename GraphicsApiHandler, bool IsMainWindow, bool IsValidationEnabled>
    void
    GenericWindow<GraphicsApiHandler, IsMainWindow, IsValidationEnabled>::update()
    {
        glfwPollEvents();
    }

    extern template class GenericWindow<graphics_engine::gl::Core<true>, true, true>;
    extern template class GenericWindow<graphics_engine::gl::Core<false>, true, true>;

    extern template class GenericWindow<graphics_engine::gl::Core<true>, true, false>;
    extern template class GenericWindow<graphics_engine::gl::Core<false>, true, false>;

    extern template class GenericWindow<graphics_engine::gl::Core<true>, false, true>;
    extern template class GenericWindow<graphics_engine::gl::Core<false>, false, true>;

    extern template class GenericWindow<graphics_engine::gl::Core<true>, false, false>;
    extern template class GenericWindow<graphics_engine::gl::Core<false>, false, false>;

    extern template class GenericWindow<graphics_engine::vulkan::Core<true>, true, true>;
    extern template class GenericWindow<graphics_engine::vulkan::Core<false>, true, true>;

    extern template class GenericWindow<graphics_engine::vulkan::Core<true>, true, false>;
    extern template class GenericWindow<graphics_engine::vulkan::Core<false>, true, false>;

    extern template class GenericWindow<graphics_engine::vulkan::Core<true>, false, true>;
    extern template class GenericWindow<graphics_engine::vulkan::Core<false>, false, true>;

    extern template class GenericWindow<graphics_engine::vulkan::Core<true>, false, false>;
    extern template class GenericWindow<graphics_engine::vulkan::Core<false>, false, false>;
}