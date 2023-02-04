/*!
 * @file
 * @brief This file contains procedural flag method
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#pragma once

#include <framework/method.hpp>

/**
 * @brief This class represents procedural flag method
 */
class SKFlagMethod: public Method{
  public:
    SKFlagMethod(MethodConstructionData const*);
    virtual ~SKFlagMethod();
    virtual void onDraw(Frame&frame,SceneParam const&sceneParam) override;
    GPUContext ctx;///< gpu context
};

