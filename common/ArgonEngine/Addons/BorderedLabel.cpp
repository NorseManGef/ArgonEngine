#include "BorderedLabel.h"

namespace Argon {
    namespace Addons {
    BorderedLabel::BorderedLabel() {
        Label::vertex_array= std::make_shared<Argon::VertexArray>();
        Label::vertex_array->add_attribute<float>(2, "position");
        Label::vertex_array->add_attribute<float>(2, "texture_coord");
        Label::vertex_array->add_attribute<float>(4, "modifiers");
        Label::vertex_array->add_attribute<uint8_t>(4, "map_location");
        Label::vertex_array->add_attribute<uint8_t>(4, "fill_color");
        Label::vertex_array->add_attribute<uint8_t>(4, "stroke_color");
        Label::vertex_array->add_attribute<uint8_t>(4, "glow_color");

        Label::material=std::make_shared<Argon::Material>();
        Label::material->shader=TEXT_SHADER;
        Label::uniform.get_texture("texture")=GlyphCache::get_cache().texture;
        Label::material = Label::material;
        Label::cull_face=Argon::kCullNone;
        Label::material->blend=Argon::kBlendAlpha;
        line_style = kLineLeft;
        max_width = 0.;

        Sprite::material=std::make_shared<Argon::Material>();
        Sprite::material->shader="shader://sprite_shader.shd";
        Sprite::render_flags&=~(kRenderDepthMask);
        Sprite::cull_face = kCullNone;
        Sprite::bounds.origin=Vector3f(-0.5,-0.5,0.0);
        Sprite::bounds.size=Vector3f(1,1,0);
        Sprite::vertex_array=std::make_shared<VertexArray>();
        Sprite::vertex_array->add_attribute<float>(2, "position");
        Sprite::vertex_array->add_attribute<float>(2, "texture_coord");

        Sprite::vertex_array->set_size(4);
        Sprite::vertex_array->generate_indices();
        VertexIterator tex_it = Sprite::vertex_array->begin("texture_coord");
        VertexIterator pos_it = Sprite::vertex_array->begin("position");

        *(pos_it)  =Vector2f(-0.5,-0.5);
        *(++pos_it)=Vector2f(-0.5,0.5);
        *(++pos_it)=Vector2f(0.5,-0.5);
        *(++pos_it)=Vector2f(0.5,0.5);

        *(tex_it)  =Vector2f(0.0,1.0);
        *(++tex_it)=Vector2f(0.0,0.0);
        *(++tex_it)=Vector2f(1.0,1.0);
        *(++tex_it)=Vector2f(1.0,0.0);

        Sprite::vertex_array->draw_type = kDrawTriangleStrip;
        Sprite::vertex_array->update_id++;

        Sprite::color.set(1,1,1,1);
        Label::color.set(0,0,0,1);

        marginX = 0.5;
        marginY = 0.5;
    }

    void BorderedLabel::render(const char* string,
                              VirtualResource font,
                              VirtualResource bgTexture,
                              LineStyle style,
                              float marginX,
                              float marginY) {  
        this->marginX = marginX;
        this->marginY = marginY;

        texture = bgTexture;
        render_str(string, font, style);
        
        setxPosition(Sprite::position[0]);
        setyPosition(Sprite::position[1]);

        Sprite::scale[0] = Label::bounds.size[0]*Label::scale[0] + marginX*Label::scale[0];
        Sprite::scale[1] = Label::bounds.size[1]*Label::scale[1] + marginY*Label::scale[1];
    } 
    }
}
