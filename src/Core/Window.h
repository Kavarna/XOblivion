#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include "Oblivion.h"


#include "Input.h"

class WindowObject : public ISingletone<WindowObject>
{
public:
    WindowObject(uint32_t width, uint32_t height):
        m_windowWidth(width), m_windowHeight(height)
    {
        glfwInit();
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_window = glfwCreateWindow(width, height, APPLICATION_NAME, nullptr, nullptr);
        
        glfwSetWindowSizeCallback(m_window, WindowObject::ResizeWindow);
        glfwSetCursorPosCallback(m_window, WindowObject::MouseCallback);
        glfwSetKeyCallback(m_window, KeyboardCallback);
        glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
        if (!m_window)
            THROW_INITIALIZATION_EXCEPTION("Couldn't create window");
    }

    ~WindowObject()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

public:
    void run()
    {
        while (!glfwWindowShouldClose(m_window))
        {
            m_update();
            m_render();
            updateCore();
            glfwPollEvents();
        }
    }

    void toggleMouse()
    {
        if (m_mouseEnabled)
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_mouseEnabled = !m_mouseEnabled;
    }

    bool mouseEnabled()
    {
        return m_mouseEnabled;
    }

    void close()
    {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }

    void setRenderCallback(std::function<void()> callback)
    {
        m_render = callback;
    }

    void setUpdateCallback(std::function<void()> callback)
    {
        m_update = callback;
    }

    void setResizeCallback(std::function<void(uint32_t, uint32_t)> callback)
    {
        m_resize = callback;
    }

    void setWindowTitle(const std::string& title)
    {
        glfwSetWindowTitle(m_window, title.c_str());
    }

    vk::Result createVulkanSurface(vk::Instance instance, vk::SurfaceKHR& surface)
    {
        return vk::Result(glfwCreateWindowSurface((VkInstance)instance, m_window, nullptr, (VkSurfaceKHR*)&surface));
    }

    GLFWwindow* getWindow()
    {
        return m_window;
    }

    float getWindowWidth()
    {
        return (float)m_windowWidth;
    }

    float getWindowHeight()
    {
        return (float)m_windowHeight;
    }

    void setWindowTitle(const char* name)
    {
        glfwSetWindowTitle(m_window, name);
    }

public:
    std::vector<const char*> getWindowExtensions()
    {
        uint32_t numExtensions;
        const char** extensions = glfwGetRequiredInstanceExtensions(&numExtensions);
        return std::vector<const char*>(extensions, extensions + numExtensions);
    }

private:
    void updateCore()
    {
        Input::Get()->update();
        if (!m_mouseEnabled)
        {
            glfwSetCursorPos(m_window, 0.0, 0.0);
            MouseCallback(m_window, 0.0, 0.0);
        }
    }


private:
    static void ResizeWindow(GLFWwindow* window, int width, int height)
    {
        WindowObject::Get()->m_windowWidth    = width;
        WindowObject::Get()->m_windowHeight   = height;
        if (window == WindowObject::Get()->m_window && WindowObject::Get()->m_resize)
        {
            WindowObject::Get()->m_resize(width, height);
        }
    }

    static void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Input::Get()->setKeyState(key, action);
    }

    static void MouseCallback(GLFWwindow* window, double xpos, double ypos)
    {
        if (WindowObject::Get()->mouseEnabled())
        {
            Input::Get()->setCursorPos(xpos, ypos);
        }
        else
        {
            uint32_t width, height;
            width = WindowObject::Get()->m_windowWidth;
            height = WindowObject::Get()->m_windowHeight;
            Input::Get()->setCursorPos(xpos / width, ypos / height);
        }
    }

    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        Input::Get()->setKeyState(button, action);
    }

private:
    GLFWwindow*									m_window;
    bool                                        m_mouseEnabled = true;
    int                                         m_windowWidth;
    int                                         m_windowHeight;


    // Callbacks
    std::function<void()>						m_update;
    std::function<void()>						m_render;
    std::function<void(uint32_t, uint32_t)>		m_resize;
    std::function<void(double, double)>         m_cursorPosition;
};
