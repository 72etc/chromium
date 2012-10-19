// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"

#include "cc/solid_color_draw_quad.h"

#include "base/logging.h"

namespace cc {

scoped_ptr<CCSolidColorDrawQuad> CCSolidColorDrawQuad::create(const CCSharedQuadState* sharedQuadState, const IntRect& quadRect, SkColor color)
{
    return make_scoped_ptr(new CCSolidColorDrawQuad(sharedQuadState, quadRect, color));
}

CCSolidColorDrawQuad::CCSolidColorDrawQuad(const CCSharedQuadState* sharedQuadState, const IntRect& quadRect, SkColor color)
    : CCDrawQuad(sharedQuadState, CCDrawQuad::SolidColor, quadRect)
    , m_color(color)
{
    if (SkColorGetA(m_color) < 255)
        m_quadOpaque = false;
    else
        m_opaqueRect = quadRect;
}

const CCSolidColorDrawQuad* CCSolidColorDrawQuad::materialCast(const CCDrawQuad* quad)
{
    DCHECK(quad->material() == CCDrawQuad::SolidColor);
    return static_cast<const CCSolidColorDrawQuad*>(quad);
}

}  // namespacec cc
