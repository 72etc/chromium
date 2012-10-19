// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CCYUVVideoDrawQuad_h
#define CCYUVVideoDrawQuad_h

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "CCDrawQuad.h"
#include "CCVideoLayerImpl.h"

namespace cc {

class CCYUVVideoDrawQuad : public CCDrawQuad {
public:
    static scoped_ptr<CCYUVVideoDrawQuad> create(const CCSharedQuadState*, const IntRect&, const CCVideoLayerImpl::FramePlane& yPlane, const CCVideoLayerImpl::FramePlane& uPlane, const CCVideoLayerImpl::FramePlane& vPlane);

    const CCVideoLayerImpl::FramePlane& yPlane() const { return m_yPlane; }
    const CCVideoLayerImpl::FramePlane& uPlane() const { return m_uPlane; }
    const CCVideoLayerImpl::FramePlane& vPlane() const { return m_vPlane; }

    static const CCYUVVideoDrawQuad* materialCast(const CCDrawQuad*);
private:
    CCYUVVideoDrawQuad(const CCSharedQuadState*, const IntRect&, const CCVideoLayerImpl::FramePlane& yPlane, const CCVideoLayerImpl::FramePlane& uPlane, const CCVideoLayerImpl::FramePlane& vPlane);

    CCVideoLayerImpl::FramePlane m_yPlane;
    CCVideoLayerImpl::FramePlane m_uPlane;
    CCVideoLayerImpl::FramePlane m_vPlane;

    DISALLOW_COPY_AND_ASSIGN(CCYUVVideoDrawQuad);
};

}

#endif
