#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <Oblivion.h>


class Input : public ISingletone<Input>
{
public:
    enum class EKeyState
    {
        eRelease = 0,
        ePress = 1,
        eRepeat = 2
    };
public:
    Input() {};
    ~Input() {};

public:
    auto                                        setCursorPos(double x, double y)
    {
        m_cursorPositionX = x;
        m_cursorPositionY = y;
    }

    auto                                        setKeyState(int key, int state)
    {
        const char* string = glfwGetKeyName(key, state);
        if (string) 
        {
            m_keyStates[std::string(string)] = static_cast<EKeyState>(state);
            return;
        }
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            m_keyStates[std::string("ESCAPE")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_ENTER:
            m_keyStates[std::string("ENTER")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_TAB:
            m_keyStates[std::string("TAB")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_BACKSPACE:
            m_keyStates[std::string("BACKSPACE")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_INSERT:
            m_keyStates[std::string("INSERT")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_DELETE:
            m_keyStates[std::string("DELETE")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_RIGHT:
            m_keyStates[std::string("RIGHT")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_LEFT:
            m_keyStates[std::string("LEFT")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_DOWN:
            m_keyStates[std::string("DOWN")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_UP:
            m_keyStates[std::string("UP")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_PAGE_UP:
            m_keyStates[std::string("PAGE_UP")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_PAGE_DOWN:
            m_keyStates[std::string("PAGE_DOWN")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_HOME:
            m_keyStates[std::string("HOME")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_END:
            m_keyStates[std::string("END")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_CAPS_LOCK:
            m_keyStates[std::string("CAPS_LOCK")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_SCROLL_LOCK:
            m_keyStates[std::string("SCROLL_LOCK")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_NUM_LOCK:
            m_keyStates[std::string("NUM_LOCK")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_PRINT_SCREEN:
            m_keyStates[std::string("PRINT_SCREEN")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_PAUSE:
            m_keyStates[std::string("PAUSE")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_F1:
            m_keyStates[std::string("F1")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_F2:
            m_keyStates[std::string("F2")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_F3:
            m_keyStates[std::string("F3")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_F4:
            m_keyStates[std::string("F4")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_F5:
            m_keyStates[std::string("F5")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_F6:
            m_keyStates[std::string("F6")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_F7:
            m_keyStates[std::string("F7")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_F8:
            m_keyStates[std::string("F8")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_F9:
            m_keyStates[std::string("F9")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_F10:
            m_keyStates[std::string("F10")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_F11:
            m_keyStates[std::string("F11")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_F12:
            m_keyStates[std::string("F12")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_LEFT_SHIFT:
            m_keyStates[std::string("LEFT_SHIFT")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_LEFT_CONTROL:
            m_keyStates[std::string("LEFT_CONTROL")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_LEFT_ALT:
            m_keyStates[std::string("LEFT_ALT")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_RIGHT_SHIFT:
            m_keyStates[std::string("RIGHT_SHIFT")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_RIGHT_CONTROL:
            m_keyStates[std::string("RIGHT_CONTROL")] = static_cast<EKeyState>(state);
            break;
        case GLFW_KEY_RIGHT_ALT:
            m_keyStates[std::string("RIGHT_ALT")] = static_cast<EKeyState>(state);
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            m_keyStates[std::string("RIGHT_CLICK")] = static_cast<EKeyState>(state);
            break;
        case GLFW_MOUSE_BUTTON_LEFT:
            m_keyStates[std::string("LEFT_CLICK")] = static_cast<EKeyState>(state);
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            m_keyStates[std::string("MIDDLE_CLICK")] = static_cast<EKeyState>(state);
            break;

        default:
            break;
        }
    }
    auto                                        getKeyState(const char* key) -> EKeyState
    {
        if (auto it = m_keyStates.find(key);
            it != m_keyStates.end())
        {
            return it->second;
        }
        return EKeyState::eRelease;
    }
    auto                                        update() -> void
    {
    }

    auto                                        getMouseX() -> double
    {
        return m_cursorPositionX;
    }

    auto                                        getMouseY() -> double
    {
        return m_cursorPositionY;
    }

private:
    std::unordered_map<std::string, EKeyState>  m_keyStates;
    double                                      m_cursorPositionX = -1;
    double                                      m_cursorPositionY = -1;

};