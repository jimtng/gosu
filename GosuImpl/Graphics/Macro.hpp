#ifndef GOSUIMPL_GRAPHICS_MACRO_HPP
#define GOSUIMPL_GRAPHICS_MACRO_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/ImageData.hpp>
#include <Gosu/TR1.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <GosuImpl/Graphics/DrawOpQueue.hpp>
#include <cmath>
#include <algorithm>
#include <memory>
#include <stdexcept>

class Gosu::Macro : public Gosu::ImageData
{
    Graphics& graphics;
    VertexArrays vertexArrays;
    std::vector<Transform> embeddedTransforms;
    int givenWidth, givenHeight;
    
    void realDraw(double x1, double y1, double x2, double y3) const
    {
        // TODO: Macros should not be split up because they have different transforms! This is insane.
        // They should be premultiplied and have the same transform by definition. Then, the transformation
        // only had to be performed here once.
        
        #ifndef GOSU_IS_IPHONE
        glEnable(GL_BLEND);
        glMatrixMode(GL_MODELVIEW);
        
        for (VertexArrays::const_iterator it = vertexArrays.begin(), end = vertexArrays.end(); it != end; ++it)
        {
            glPushMatrix();
            it->renderState.apply();
            
            glTranslated(x1, y1, 0);
            glScaled((x2 - x1) / width(), (y3 - y1) / height(), 1);
            
            glInterleavedArrays(GL_T2F_C4UB_V3F, 0, &it->vertices[0]);
            glDrawArrays(GL_QUADS, 0, it->vertices.size());
            glPopMatrix();
        }
        #endif
    }
    
public:
    Macro(Graphics& graphics, DrawOpQueue& queue, int width, int height)
    : graphics(graphics), givenWidth(width), givenHeight(height)
    {
        queue.compileTo(vertexArrays);
        
        // Very important fix: RenderState only contains a (non-owned) pointer to a Transform.
        // If we want to use this Macro in more than a single frame, the pointer would be
        // invalidated. Hence, we copy the transform into this macro and just let the
        // RenderState refer to that one.
        
        // TODO: As the next step, we should flatten all the transforms into a single one. But
        // maybe not here, as the VertexArrays are already split up, possibly because their
        // transforms differed.
        
        embeddedTransforms.reserve(vertexArrays.size());
        for (VertexArrays::iterator it = vertexArrays.begin(), end = vertexArrays.end(); it != end; ++it)
        {
            embeddedTransforms.push_back(*it->renderState.transform);
            it->renderState.transform = &embeddedTransforms.back();
        }
    }
    
    int width() const
    {
        return givenWidth;
    }
    
    int height() const
    {
        return givenHeight;
    }
    
    void draw(double x1, double y1, Color c1,
        double x2, double y2, Color c2,
        double x3, double y3, Color c3,
        double x4, double y4, Color c4,
        ZPos z, AlphaMode mode) const
    {
        if (x1 != x3 || x2 != x4 || y1 != y2 || y3 != y4)
            throw std::invalid_argument("Macros cannot be rotated yet");
        if (c1 != 0xffffffff || c2 != 0xffffffff || c3 != 0xffffffff || c4 != 0xffffffff)
            throw std::invalid_argument("Macros cannot be tinted with colors yet");
        std::tr1::function<void()> f = std::tr1::bind(&Macro::realDraw, this, x1, y1, x2, y3);
        graphics.scheduleGL(f, z);
    }
    
    const Gosu::GLTexInfo* glTexInfo() const
    {
        return 0;
    }
    
    Gosu::Bitmap toBitmap() const
    {
        throw std::logic_error("Gosu::Macro cannot be rendered as Gosu::Bitmap yet");
    }
    
    void insert(const Bitmap& bitmap, int x, int y)
    {
        throw std::logic_error("Gosu::Macro cannot be updated with a Gosu::Bitmap yet");
    }
};

#endif
