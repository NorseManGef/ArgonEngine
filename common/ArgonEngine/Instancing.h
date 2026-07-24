#ifndef INSTANCING_H
#define INSTANCING_H
#include <ArgonEngine/Node.h>
#include <ArgonEngine/RenderSystem.h>
namespace Argon {
struct InstancedLines : public Node {
    enum {
        kLinesPerBatch = 16384,
        kLinesMask = kSpritesPerBatch - 1,
        kVerticesPerBatch = kLinesPerBatch * 2
    };

    std::shared_ptr<Material> material = std::make_shared<Material>();
    VirtualResource& texture = material->uniform.get_texture("texture");
    std::vector<std::shared_ptr<Renderable>> renders;
    std::vector<std::shared_ptr<VertexArray>> arrays;

    InstancedLines() {
        material->shader = "shader://instanced_line_shader.shd";
    }
    void set_lines(size_t total_liness) {
        int total_renders = total_lines / kLinesPerBatch + 1;
        while (arrays.size() < total_renders) {
            arrays.push_back(std::make_shared<VertexArray>());
            arrays.back()->add_attribute<float>(3, kPositionAttribute);
            arrays.back()->add_attribute<float>(4, kColorAttribute);
            arrays.back()->draw_type = kDrawLines;
            arrays.back()->updates_frequently = true;
            arrays.back()->recalc_stride();
        }
        if (arrays.size() != total_renders) {
            arrays.resize(total_renders);
        }

        while (renders.size() < total_renders) {
            renders.push_back(std::make_shared<Renderable>());
            std::shared_ptr<Renderable>& render = renders.back();
            render->material = material;
            render->cull_face = kCullNone;
            render->transform = this;
            render->vertex_array = arrays[renders.size() - 1];
        }
        if (renders.size() != total_renders)
            renders.resize(total_renders);
        size_t size_r = total_lines;
        int index = 0;
        while (size_r >= kLinesPerBatch) {
            if (arrays[index]->vertex_count() != kVerticesPerBatch) {
                arrays[index]->set_size(kVerticesPerBatch);
                arrays[index]->generate_indices();
            }
            size_r -= kLinesPerBatch;
            index++;
        }
        if (arrays.back()->vertex_count() != size_r * 2) {
            arrays.back()->set_size(size_r * 2);
            arrays[index]->generate_indices();
        }
    }
    void set_trail(size_t index, const Vector3f& v, const Vector4f& c) {
        VertexArray& it = *arrays[index >> 14ul];
        float* d = ((float*)it.data_start()) + (index & kSpritesMask) * 14;
        d[0] = d[7];
        d[1] = d[8];
        d[2] = d[9];

        d[3] = d[10];
        d[4] = d[11];
        d[5] = d[12];
        d[6] = d[13];

        d[7] = v.data[0];
        d[8] = v.data[1];
        d[9] = v.data[2];

        d[10] = c.data[0];
        d[11] = c.data[1];
        d[12] = c.data[2];
        d[13] = c.data[3];

        it.update_id++;
    }
    void set_line(size_t index, const Vector3f& v0, const Vector3f& v1,
                  const Vector4f& c0, const Vector4f& c1) {
        VertexArray& it = *arrays[index >> 14ul];
        float* d = ((float*)it.data_start()) + (index & kSpritesMask) * 14;
        d[0] = v0.data[0];
        d[1] = v0.data[1];
        d[2] = v0.data[2];

        d[3] = c0.data[0];
        d[4] = c0.data[1];
        d[5] = c0.data[2];
        d[6] = c0.data[3];

        d[7] = v1.data[0];
        d[8] = v1.data[1];
        d[9] = v1.data[2];

        d[10] = c1.data[0];
        d[11] = c1.data[1];
        d[12] = c1.data[2];
        d[13] = c1.data[3];

        it.update_id++;
    }

    void set_transform(size_t index, Transform& transform) {
        if (transform.get_should_render()) {
            Matrix4f world_matrix = transform.world_matrix();
            Vector4f pos[2] = {Vector4f(-0.5, 0.0, 0, 1),
                               Vector4f(0.5, 0, 0, 1)};
            pos[0] = world_matrix * pos[0];
            pos[1] = world_matrix * pos[1];
            Vector4f c = transform.get_color();

            set_line(index, Vector3f(pos[0] / pos[0][3]),
                     Vector3f(pos[1] / pos[1][3]), c, c);
        } else {
            set_line(index, Vector3f(0, 0, 0), Vector3f(), kClearColor,
                     kClearColor);
        }
    }
};
} // namespace Argon

#endif // INSTANCING_H
