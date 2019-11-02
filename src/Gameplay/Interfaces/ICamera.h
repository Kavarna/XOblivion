#pragma once



#include <glm/glm.hpp>

class ICamera
{
public:
    ICamera() {};
    virtual ~ICamera() {};

public:
    virtual void construct() = 0;

    virtual glm::mat4& getView()         = 0;
    virtual glm::mat4& getProjection()   = 0;

};