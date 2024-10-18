# Report for HW2 of the UM Computer Graphics & Generative Models class
Name: Swesik Ramineni

## Task 5: The Trace Function
```
Vec3 Scene::trace(const Ray& ray, int bouncesLeft, bool discardEmission) {
    if constexpr (DEBUG) {
        assert(ray.isNormalized());
    }
    if (bouncesLeft < 0) return {};

    // TODO...
    Intersection intersection = getIntersection(ray);
    if (!intersection.happened) {
        return Vec3();
    }

    return intersection.getDiffuseColor();
}
```

## Task 6: Direct Illumination
```
Vec3 Scene::trace(const Ray& ray, int bouncesLeft, bool discardEmission) {
    if constexpr (DEBUG) {
        assert(ray.isNormalized());
    }
    if (bouncesLeft < 0) return {};

    Intersection intersection = getIntersection(ray);
    if (!intersection.happened) {
        return Vec3();
    }

    Vec3 rand_inRay = Random::randomHemisphereDirection(intersection.getNormal());
    rand_inRay.normalize();

    Ray sample_ray = { intersection.pos, rand_inRay };
    Intersection sample_intersec = getIntersection(sample_ray);

    if (!sample_intersec.happened) {
        return intersection.getEmission();
    }

    const float p = 1 / (2 * PI);
    float cos_theta = sample_ray.dir.dot(intersection.getNormal());
    Vec3 brdf = intersection.calcBRDF(-rand_inRay, -ray.dir);

    return intersection.getEmission() + brdf * sample_intersec.getEmission() * cos_theta / p;
}
```
![32 spp image - local illumination](local_illumination.png)


## Task 7: Global Illumination
```
Vec3 Scene::trace(const Ray& ray, int bouncesLeft, bool discardEmission) {
    if constexpr (DEBUG) {
        assert(ray.isNormalized());
    }
    if (bouncesLeft < 0) return {};

    Intersection intersection = getIntersection(ray);
    if (!intersection.happened) {
        return Vec3();
    }

    Vec3 rand_inRay = Random::randomHemisphereDirection(intersection.getNormal());
    rand_inRay.normalize();

    Ray bounce_ray = { intersection.pos, rand_inRay };
    Intersection sample_intersec = getIntersection(bounce_ray);

    if (!sample_intersec.happened) {
        return intersection.getEmission();
    }

    const float p = 1 / (2 * PI);
    float cos_theta = bounce_ray.dir.dot(intersection.getNormal());
    Vec3 brdf = intersection.calcBRDF(-rand_inRay, -ray.dir);
    Vec3 bounce_emission = trace(bounce_ray, bouncesLeft - 1, discardEmission);

    return intersection.getEmission() + brdf * bounce_emission * cos_theta / p;
}
```

![32 spp image - global illumination](global_illumination.png)

## Task 8: Acceleration
```
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
```

![32 spp image - acceleration](acceleration.png)

## Task 9.1: Multi-threading acceleration
```
void update_pixel(size_t x, size_t y, Scene& scene, std::vector<std::vector<Vec3>>& image, int width, int height,
                  Vec3 cameraPos) {
    Vec3 worldPos = { (float) x / width - 0.5f, 1.5f - (float) y / height, (cameraPos.z + 1.0f) / 2 };
    Ray ray {
        cameraPos,
        worldPos - cameraPos,
    };
    ray.dir.normalize();
    Vec3 value {};
    for (int i = 0; i < SPP; i++) {
        value += scene.trace(ray, MAX_DEPTH);
    }
    image[y][x] = value / SPP;
    UpdateProgress((float) (y * width + x) / (width * height));
}

void update_row(size_t y, Scene& scene, std::vector<std::vector<Vec3>>& image, int width, int height, Vec3 cameraPos) {
    for (size_t x = 0; x < width; x++) {
        update_pixel(x, y, std::ref(scene), std::ref(image), width, height, cameraPos);
    }
}

int main() {
    using namespace std::chrono;
    auto startTime = high_resolution_clock::now();

    Scene scene;
    scene.addObjects(OBJ_PATH, MTL_SEARCH_DIR);
    scene.constructBVH();

    auto timeAfterVBVH = high_resolution_clock::now();
    std::cout << "BVH Construction time in seconds: " << duration_cast<seconds>(timeAfterVBVH - startTime).count()
              << '\n';
    int width = RESOLUTION, height = RESOLUTION;
    std::vector<std::vector<Vec3>> image(height, std::vector<Vec3>(width));
    Vec3 cameraPos = { 0.0f, 1.0f, 4.0f };

    if constexpr (!DEBUG) {
        std::cout << "Debug mode disabled. Progress output will be in brief." << '\n';
    }

    // x: right
    // y: up
    // z: outwards
    std::vector<std::thread> pixel_threads;
    for (size_t y = 0; y < height; y++) {
        pixel_threads.emplace_back(update_row, y, std::ref(scene), std::ref(image), width, height, cameraPos);
    }
    for (auto& i : pixel_threads) {
        i.join();
    }
    std::cout << std::endl;

    auto finishTime = high_resolution_clock::now();
    std::cout << "Rendering time in seconds: " << duration_cast<seconds>(finishTime - timeAfterVBVH).count() << '\n';

    std::filesystem::path outPath = std::filesystem::absolute(OUTPUT_PATH);

    FILE* fp = fopen(outPath.string().c_str(), "wb");
    (void) fprintf(fp, "P6\n%d %d\n255\n", width, height);
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            static unsigned char color[3];
            color[0] = toLinear(image[y][x].x);
            color[1] = toLinear(image[y][x].y);
            color[2] = toLinear(image[y][x].z);
            fwrite(color, 1, 3, fp);
        }
    }
    fclose(fp);
    std::cout << "Output image written to " << outPath << '\n';
}
```

The below image was generated with 128 spp and max depth 8 in 109 seconds.
![128 spp image - parellelized](parallelized.png)