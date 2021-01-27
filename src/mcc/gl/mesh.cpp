#include <mcc/gl/mesh.hpp>

#include <GL/glew.h>

using namespace mcc;
using namespace mcc::gl;

Mesh::Mesh(Mesh&& rhs) {
    this->va = std::move(rhs.va);
    this->vb = std::move(rhs.vb);
    this->ib = std::move(rhs.ib);
    this->opaque_count = rhs.opaque_count;
    this->transparent_count = rhs.transparent_count;
    this->transparent_offset = rhs.transparent_offset;
    rhs.opaque_count = 0;
    rhs.transparent_count = 0;
}

void Mesh::draw_opaque() const {
    if (this->opaque_count > 0) {
        this->va.bind();
        this->ib.bind();
        glDrawElements(GL_TRIANGLES, this->opaque_count, GL_UNSIGNED_INT, nullptr);
    }
}

void Mesh::draw_transparent() const {
    if (this->transparent_count > 0) {
        this->va.bind();
        this->ib.bind();
        glDrawElements(
            GL_TRIANGLES,
            this->transparent_count,
            GL_UNSIGNED_INT,
            (const void*)(this->transparent_offset * sizeof(unsigned int))
        );
    }
}

void Mesh::update(const Matrix& matrix, float vx_sz, bool generate_borders) {
    std::vector<Vertex> opaque_verts, transparent_verts;
    std::vector<unsigned int> opaque_indices, transparent_indices;
    std::vector<unsigned char> mask;

    auto& sz = matrix.size;

    auto get_mat = [&] (unsigned int vox_index) -> const Material& {
        return matrix.palette[matrix.voxels[vox_index]];
    };

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
                                            matrix.voxels[(x.x + q.x) * sz.y * sz.z + (x.y + q.y) * sz.z + (x.z + q.z)] :
                                            0;
                            } else if (x[d] == int(sz[d]) - 1) {
                                mask[n++] = back_face ? 
                                            0 :
                                            matrix.voxels[x.x * sz.y * sz.z + x.y * sz.z + x.z];
                            } else if (get_mat(x.x * sz.y * sz.z + x.y * sz.z + x.z).color.a != 255 ||
                                       get_mat((x.x + q.x) * sz.y * sz.z + (x.y + q.y) * sz.z + (x.z + q.z)).color.a != 255) {
                                mask[n++] = back_face ?
                                            matrix.voxels[(x.x + q.x) * sz.y * sz.z + (x.y + q.y) * sz.z + (x.z + q.z)] :
                                            matrix.voxels[x.x * sz.y * sz.z + x.y * sz.z + x.z];
                            } else {
                                mask[n++] = 0;
                            }
                        }
                    }
                } else {
                    for (x[v] = 0; x[v] < int(sz[v]); ++x[v]) {
                        for (x[u] = 0; x[u] < int(sz[u]); ++x[u]) {
                            if (x[d] >= 0 && x[d] < int(sz[d]) - 1 &&
                                get_mat(x.x * sz.y * sz.z + x.y * sz.z + x.z).color.a != 255 ||
                                get_mat((x.x + q.x) * sz.y * sz.z + (x.y + q.y) * sz.z + (x.z + q.z)).color.a != 255) {
                                mask[n++] = back_face ?
                                            matrix.voxels[(x.x + q.x) * sz.y * sz.z + (x.y + q.y) * sz.z + (x.z + q.z)] :
                                            matrix.voxels[x.x * sz.y * sz.z + x.y * sz.z + x.z];
                            } else {
                                mask[n++] = 0;
                            }
                        }
                    }
                }               
                
                ++x[d];
                n = 0;

                // Generate mesh from mask
                for (int j = 0; j < int(sz[v]); ++j) {
                    for (int i = 0; i < int(sz[u]);) {
                        if (mask[n] != 0) {
                            int w, h;
                            for (w = 1; i + w < int(sz[u]) && mask[n + w] == mask[n]; ++w);
                            bool done = false;
                            for (h = 1; j + h < int(sz[v]); ++h) {
                                for (int k = 0; k < w; ++k) {
                                    if (mask[n + k + h * sz[u]] == 0 || mask[n + k + h * sz[u]] != mask[n]) {
                                        done = true;
                                        break;
                                    }
                                }

                                if (done) {
                                    break;
                                }
                            }

                            if (mask[n] != 0) {
                                auto& verts = matrix.palette[mask[n]].color.a == 255 ? opaque_verts : transparent_verts;
                                auto& indices = matrix.palette[mask[n]].color.a == 255 ? opaque_indices : transparent_indices;

                                x[u] = i;
                                x[v] = j;

                                glm::ivec3 du = { 0, 0, 0 }, dv = { 0, 0, 0 };
                                du[u] = w;
                                dv[v] = h;
                                
                                auto vi = verts.size();
                                verts.resize(vi + 4, { { 0.0f, 0.0f, 0.0f }, back_face ? -q : q, matrix.palette[mask[n]].color });
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
                                    mask[n + k + l * sz[u]] = 0;
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

    for (auto& i : transparent_indices) {
        i += opaque_verts.size();
    }

    opaque_verts.insert(opaque_verts.end(), transparent_verts.begin(), transparent_verts.end());
    this->update(opaque_verts, opaque_indices, transparent_indices);
}

void mcc::gl::Mesh::update(
    const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& opaque_indices,
    const std::vector<unsigned int>& transparent_indices
) {
    this->opaque_count = opaque_indices.size();
    this->transparent_count = transparent_indices.size();
    this->transparent_offset = transparent_indices.size();

    if (this->opaque_count > 0 || this->transparent_count > 0) {
        // Create index buffer
        auto indices = opaque_indices;
        indices.insert(indices.end(), opaque_indices.begin(), opaque_indices.end());
        this->ib = gl::IndexBuffer::create(indices.size() * sizeof(unsigned int), indices.data(), gl::Usage::Static).unwrap();

        // Create vertex buffer
        this->vb = gl::VertexBuffer::create(vertices.size() * sizeof(Vertex), vertices.data(), gl::Usage::Static).unwrap();

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
