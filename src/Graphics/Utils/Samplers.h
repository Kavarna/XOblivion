#pragma once


#include <Oblivion.h>
#include "../Interfaces/IGraphicsObject.h"



class Samplers : public ISingletone<Samplers>, public IVulkanDeviceObject
{
public:
    Samplers();
    ~Samplers();

public:
    auto                        getLinearAnisotropicSampler()->vk::Sampler;
    auto                        getLinearNonAnisotropicSampler()->vk::Sampler;

private:
    vk::Sampler                 m_linearAnisotropicSampler;
    vk::Sampler                 m_linearNonAnisotropicSampler;
};