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