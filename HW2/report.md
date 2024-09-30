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

### Task 6: Direct Illumination
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


### Task 6: Global Illumination
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