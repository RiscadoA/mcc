#include <mcc/gl/mesh.hpp>

#include <GL/glew.h>

using namespace mcc;
using namespace mcc::gl;

struct Vertex {
    glm::vec3 pos, normal;
    glm::u8vec4 color;
};

Mesh::Mesh(Mesh&& rhs) {
    this->va = std::move(rhs.va);
    this->vb = std::move(rhs.vb);
    this->ib = std::move(rhs.ib);
    this->index_count = rhs.index_count;
    rhs.index_count = 0;
}

void Mesh::draw() const {
    if (this->index_count > 0) {
        this->va.bind();
        this->ib.bind();
        glDrawElements(GL_TRIANGLES, this->index_count, GL_UNSIGNED_INT, nullptr);
    }
}

void Mesh::build_buffers(const glm::u8vec4* voxels, glm::uvec3 sz, float vx_sz, bool generate_borders) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> indices;
    std::vector<glm::u8vec4> mask;

    // For both back and front faces
    bool back_face = true;
    do {
        back_face = !back_face;

        // For each axis
        for (int d = 0; d < 3; ++d) {
            int u = (d + 1) % 3;
            int v = (d + 2) % 3;

            glm::ivec3 x = { 0, 0, 0 }, q = { 0, 0, 0 };
            q[d] = 1;
            mask.resize(sz[u] * sz[v]);

            for (x[d] = -1; x[d] < int(sz[d]);) {
                int n = 0;              

                // Create mask
                if (generate_borders) {
                    for (x[v] = 0; x[v] < int(sz[v]); ++x[v]) {
                        for (x[u] = 0; x[u] < int(sz[u]); ++x[u]) {
                            if (x[d] < 0) {
                                mask[n++] = back_face ?
                                            voxels[(x.x + q.x) * sz.y * sz.z + (x.y + q.y) * sz.z + (x.z + q.z)] :
                                            glm::u8vec4(0, 0, 0, 0);
                            } else if (x[d] == int(sz[d]) - 1) {
                                mask[n++] = back_face ? 
                                            glm::u8vec4(0, 0, 0, 0) :
                                            voxels[x.x * sz.y * sz.z + x.y * sz.z + x.z];
                            } else if (voxels[x.x * sz.y * sz.z + x.y * sz.z + x.z] != voxels[(x.x + q.x) * sz.y * sz.z + (x.y + q.y) * sz.z + (x.z + q.z)]) {
                            //} else if (voxels[x.x * sz.y * sz.z + x.y * sz.z + x.z].a != 255 || voxels[(x.x + q.x) * sz.y * sz.z + (x.y + q.y) * sz.z + (x.z + q.z)].a != 255) {
                                mask[n++] = back_face ?
                                            voxels[(x.x + q.x) * sz.y * sz.z + (x.y + q.y) * sz.z + (x.z + q.z)] :
                                            voxels[x.x * sz.y * sz.z + x.y * sz.z + x.z];
                            } else {
                                mask[n++] = glm::u8vec4(0, 0, 0, 0);
                            }
                        }
                    }
                } else {
                    for (x[v] = 0; x[v] < int(sz[v]); ++x[v]) {
                        for (x[u] = 0; x[u] < int(sz[u]); ++x[u]) {
                            if (x[d] >= 0 && x[d] < int(sz[d]) - 1 &&
                                voxels[x.x * sz.y * sz.z + x.y * sz.z + x.z] != voxels[(x.x + q.x) * sz.y * sz.z + (x.y + q.y) * sz.z + (x.z + q.z)]) {
                                mask[n++] = back_face ?
                                            voxels[(x.x + q.x) * sz.y * sz.z + (x.y + q.y) * sz.z + (x.z + q.z)] :
                                            voxels[x.x * sz.y * sz.z + x.y * sz.z + x.z];
                            } else {
                                mask[n++] = glm::u8vec4(0, 0, 0, 0);
                            }
                        }
                    }
                }               
                
                ++x[d];
                n = 0;

                // Generate mesh from mask
                for (int j = 0; j < int(sz[v]); ++j) {
                    for (int i = 0; i < int(sz[u]);) {
                        if (mask[n].a != 0) {
                            int w, h;
                            for (w = 1; i + w < int(sz[u]) && mask[n + w] == mask[n]; ++w);
                            bool done = false;
                            for (h = 1; j + h < int(sz[v]); ++h) {
                                for (int k = 0; k < w; ++k) {
                                    if (mask[n + k + h * sz[u]].a == 0 || mask[n + k + h * sz[u]] != mask[n]) {
                                        done = true;
                                        break;
                                    }
                                }

                                if (done) {
                                    break;
                                }
                            }

                            if (mask[n].a != 0) {
                                x[u] = i;
                                x[v] = j;

                                glm::ivec3 du = { 0, 0, 0 }, dv = { 0, 0, 0 };
                                du[u] = w;
                                dv[v] = h;
                                
                                auto vi = verts.size();
                                verts.resize(vi + 4, { { 0.0f, 0.0f, 0.0f }, back_face ? -q : q, mask[n] });
                                verts[vi + 0].pos = glm::vec3(x) * vx_sz;
                                verts[vi + 1].pos = glm::vec3(x + du) * vx_sz;
                                verts[vi + 2].pos = glm::vec3(x + du + dv) * vx_sz;
                                verts[vi + 3].pos = glm::vec3(x + dv) * vx_sz;

                                auto ii = indices.size();
                                indices.resize(ii + 6);
                                if (back_face) {
                                    indices[ii + 0] = int(vi) + 0;
                                    indices[ii + 1] = int(vi) + 2;
                                    indices[ii + 2] = int(vi) + 1;
                                    indices[ii + 3] = int(vi) + 3;
                                    indices[ii + 4] = int(vi) + 2;
                                    indices[ii + 5] = int(vi) + 0;                              
                                } else {
                                    indices[ii + 0] = int(vi) + 0;
                                    indices[ii + 1] = int(vi) + 1;
                                    indices[ii + 2] = int(vi) + 2;
                                    indices[ii + 3] = int(vi) + 2;
                                    indices[ii + 4] = int(vi) + 3;
                                    indices[ii + 5] = int(vi) + 0;
                                }                          
                            }

                            for (int l = 0; l < h; ++l) {
                                for (int k = 0; k < w; ++k) {
                                    mask[n + k + l * sz[u]].a = 0;
                                }
                            }

                            i += w;
                            n += w;
                        } else {
                            ++i;
                            ++n;
                        }
                    }
                }
            }
        }
    } while(back_face != true);

    this->index_count = int(indices.size());
    if (this->index_count > 0) {
        // Create index buffer
        this->ib = gl::IndexBuffer::create(indices.size() * sizeof(unsigned int), &indices[0], gl::Usage::Static).unwrap();

        // Create vertex buffer
        this->vb = gl::VertexBuffer::create(verts.size() * sizeof(Vertex), &verts[0], gl::Usage::Static).unwrap();
    }
}

void Mesh::build_va() {
    if (this->index_count > 0) {
        // Create vertex array
        this->va = gl::VertexArray::create({
            gl::Attribute(
                this->vb,
                sizeof(Vertex), offsetof(Vertex, Vertex::pos),
                3, gl::Attribute::Type::F32,
                0
            ),
            gl::Attribute(
                this->vb,
                sizeof(Vertex), offsetof(Vertex, Vertex::normal),
                3, gl::Attribute::Type::F32,
                1
            ),
            gl::Attribute(
                this->vb,
                sizeof(Vertex), offsetof(Vertex, Vertex::color),
                4, gl::Attribute::Type::NU8,
                2
            )
        }).unwrap();
    }
}
