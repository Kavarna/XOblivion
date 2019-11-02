#pragma once


#include <Oblivion.h>


class IFrameDependent
{
public:
    virtual ~IFrameDependent() { };

    virtual void            create(uint32_t totalFrames, uint32_t width, uint32_t height) = 0; // called on size changed
    virtual void            render(uint32_t frameIndex) = 0;
    virtual void            frameCleanup() = 0;
    virtual void            recreate(uint32_t totalFrames, uint32_t width, uint32_t height) final { frameCleanup(); create(totalFrames, width, height); };

};