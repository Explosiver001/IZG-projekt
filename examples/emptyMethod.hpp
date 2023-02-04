/*!
 * @file
 * @brief This file contains empty rendering method
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#pragma once

#include <framework/method.hpp>

namespace emptyMethod{

/**
 * @brief Empty rendering method
 */
class Method: public ::Method{
  public:
    Method(MethodConstructionData const*){}
    virtual void onDraw(Frame&frame,SceneParam const&sceneParam) override;
};

void Method::onDraw(Frame&frame,SceneParam const&sceneParam){
  (void)sceneParam;
  GPUContext ctx;
  ctx.frame = frame;
  clear(ctx,0,0,0,1);
}

}
