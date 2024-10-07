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