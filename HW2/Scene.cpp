#include "Scene.h"

#include <filesystem>
#include <iostream>

#include "Accel.h"
#include "Config.h"
#include "Math.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


Vec3 Scene::trace(const Ray& ray, int bouncesLeft, bool discardEmission) {
    if constexpr (DEBUG) {
        assert(ray.isNormalized());
    }
    if (bouncesLeft < 0) return {};

    Intersection intersection = getIntersection(ray);
    if (!intersection.happened) {
        return Vec3();
    }

    // calculate direct illumination
    Vec3 direct_irradiance = Vec3();
    Intersection light_sample = sampleLight();
    Vec3 light_dir = light_sample.pos - intersection.pos;
    float dist_to_light = light_dir.getLength();
    light_dir.normalize();
    Ray light_ray = { intersection.pos, light_dir };
    Ray shadow_ray = { light_sample.pos, -light_dir };
    Intersection light_intersection = getIntersection(shadow_ray);

    if (light_intersection.happened && light_intersection.object == intersection.object) {
        const float p = 1 / (lightArea);
        float cos_theta_1 = light_ray.dir.dot(intersection.getNormal());
        float cos_theta_2 = light_ray.dir.dot(-light_sample.getNormal()) / (dist_to_light * dist_to_light);
        Vec3 brdf = intersection.calcBRDF(-light_ray.dir, -ray.dir);
        direct_irradiance = light_sample.getEmission() * brdf * cos_theta_1 * cos_theta_2 / p;
    }

    // calculate indirect illumination
    Vec3 rand_inRay = Random::cosWeightedHemisphere(intersection.getNormal());
    rand_inRay.normalize();

    Ray bounce_ray = { intersection.pos, rand_inRay };
    Intersection sample_intersec = getIntersection(bounce_ray);

    if (!sample_intersec.happened) {
        return direct_irradiance + (discardEmission ? Vec3() : intersection.getEmission());
    }

    float cos_theta = bounce_ray.dir.dot(intersection.getNormal());
    Vec3 brdf = intersection.calcBRDF(-rand_inRay, -ray.dir);
    const float p = cos_theta / PI;
    Vec3 indirect_light = brdf * trace(bounce_ray, bouncesLeft - 1, true) * cos_theta / p;

    return direct_irradiance + indirect_light + (discardEmission ? Vec3() : intersection.getEmission());
}

tinyobj::ObjReader Scene::reader {};

void Scene::addObjects(std::string_view modelPath, std::string_view searchPath) {
    tinyobj::ObjReaderConfig config;
    config.mtl_search_path = searchPath;
    if (!reader.ParseFromFile(std::string(modelPath), config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
            std::filesystem::path relative(modelPath);
            std::cerr << "Reading file " << std::filesystem::absolute(relative)
                      << " error. File may be malformed or not exist.\n";
        }
        exit(1);
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        auto* object = new Object();
        object->name = shapes[s].name;
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            std::vector<Vec3> positions;
            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                positions.push_back({ vx, vy, vz });
            }   // per-vertex
            index_offset += fv;
            Mesh mesh { positions[0], positions[1], positions[2] };
            object->area += mesh.area;
            object->meshes.push_back(std::move(mesh));
        }   // per-face
        object->constructBoundingBox();
        // we assume each object uses only a single material for all meshes
        auto materialId = shapes[s].mesh.material_ids[0];
        auto& material = materials[materialId];
        object->kd = Vec3 {
            material.diffuse[0],
            material.diffuse[1],
            material.diffuse[2],
        };
        if (material.emission[0] + material.emission[1] + material.emission[2] > 0) {   // is light
            object->ke = Vec3 { material.emission[0], material.emission[1], material.emission[2] };
            object->hasEmission = true;
            lights.push_back(object);
            lightArea += object->area;
        }
        objects.push_back(object);
    }   // per-shape
}

void Scene::constructBVH() {
    assert(!objects.empty());
    bvh.root = BVH::build(objects);
}

Intersection Scene::getIntersection(const Ray& ray) {
    assert(bvh.root);
    return bvh.root->intersect(ray);
}

Intersection Scene::sampleLight() const {
    assert(lights.size() == 1 && "Currently only support a single light object");
    assert(lightArea > 0.0f);
    Intersection inter;
    return lights[0]->sample();
}

Scene::~Scene() {
    for (auto obj : objects) {
        delete obj;
    }
}
