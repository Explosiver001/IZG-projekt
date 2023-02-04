/*!
 * @file
 * @brief This file contains rendering method that renders triangle stored in buffer.
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#pragma once

#include <framework/method.hpp>

#include <vector>

namespace triangleBufferMethod{

/**
 * @brief This class represents triangle buffer rendering method
 */
class Method: public ::Method{
  public:
    Method(MethodConstructionData const*);
    virtual ~Method(){}
    virtual void onDraw(Frame&frame,SceneParam const&sceneParam) override;
    GPUContext ctx;///< gpu context
    std::vector<float>buffer;///< vertex buffer
};

}
