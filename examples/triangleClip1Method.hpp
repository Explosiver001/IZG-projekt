/*!
 * @file
 * @brief This file contains triangle clipping method 1
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#pragma once

#include <framework/method.hpp>

namespace triangleClip1Method{

/**
 * @brief Triangle clipping method 1
 */
class Method: public ::Method{
  public:
    Method(MethodConstructionData const*);
    virtual ~Method(){}
    virtual void onDraw(Frame&frame,SceneParam const&sceneParam) override;
    GPUContext ctx;///< gpu context
};

}
