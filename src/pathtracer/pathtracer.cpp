#include "pathtracer.h"

#include "scene/light.h"
#include "scene/sphere.h"
#include "scene/triangle.h"


using namespace CGL::SceneObjects;

namespace CGL {

PathTracer::PathTracer() {
  gridSampler = new UniformGridSampler2D();
  hemisphereSampler = new UniformHemisphereSampler3D();

  tm_gamma = 2.2f;
  tm_level = 1.0f;
  tm_key = 0.18;
  tm_wht = 5.0f;
}

PathTracer::~PathTracer() {
  delete gridSampler;
  delete hemisphereSampler;
}

void PathTracer::set_frame_size(size_t width, size_t height) {
  sampleBuffer.resize(width, height);
  sampleCountBuffer.resize(width * height);
}

void PathTracer::clear() {
  bvh = NULL;
  scene = NULL;
  camera = NULL;
  sampleBuffer.clear();
  sampleCountBuffer.clear();
  sampleBuffer.resize(0, 0);
  sampleCountBuffer.resize(0, 0);
}

void PathTracer::write_to_framebuffer(ImageBuffer &framebuffer, size_t x0,
                                      size_t y0, size_t x1, size_t y1) {
  sampleBuffer.toColor(framebuffer, x0, y0, x1, y1);
}

Vector3D
PathTracer::estimate_direct_lighting_hemisphere(const Ray &r,
                                                const Intersection &isect) {
  // Estimate the lighting from this intersection coming directly from a light.
  // For this function, sample uniformly in a hemisphere.

  // Note: When comparing Cornel Box (CBxxx.dae) results to importance sampling, you may find the "glow" around the light source is gone.
  // This is totally fine: the area lights in importance sampling has directionality, however in hemisphere sampling we don't model this behaviour.

  // make a coordinate system for a hit point
  // with N aligned with the Z direction.
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  // w_out points towards the source of the ray (e.g.,
  // toward the camera if this is a primary ray)
  const Vector3D hit_p = r.o + r.d * isect.t;
  const Vector3D w_out = w2o * (-r.d);

  // This is the same number of total samples as
  // estimate_direct_lighting_importance (outside of delta lights). We keep the
  // same number of samples for clarity of comparison.
  double num_samples = scene->lights.size() * ns_area_light;
    Vector3D L_out;

  // TODO (Part 3): Write your sampling loop here
  // TODO BEFORE YOU BEGIN
  // UPDATE `est_radiance_global_illumination` to return direct lighting instead of normal shading
    for (int i = 0; i < num_samples; i++) {
        Vector3D w_in = hemisphereSampler->get_sample().unit();
        Ray sample_ray = Ray(hit_p, (o2w * w_in).unit());
        sample_ray.min_t = EPS_D;
        Intersection sample_i;
        if (bvh->intersect(sample_ray, &sample_i)) {
            Vector3D emission = sample_i.bsdf->get_emission();
            if (emission[0] || emission[1] || emission[2]) {
                Vector3D f = isect.bsdf->f(w_out, w_in);
                Vector3D f_l = Vector3D(f[0] * emission[0], f[1] * emission[1], f[2] * emission[2]);
                L_out += f_l * cos_theta(w_in) * 2.0 * PI / num_samples;
            }
        }
    }
    return L_out;
}

Vector3D
PathTracer::estimate_direct_lighting_importance(const Ray &r,
                                                const Intersection &isect) {
  // Estimate the lighting from this intersection coming directly from a light.
  // To implement importance sampling, sample only from lights, not uniformly in
  // a hemisphere.

  // make a coordinate system for a hit point
  // with N aligned with the Z direction.
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  // w_out points towards the source of the ray (e.g.,
  // toward the camera if this is a primary ray)
  const Vector3D hit_p = r.o + r.d * isect.t;
  const Vector3D w_out = w2o * (-r.d);
  Vector3D L_out;
    int num_samples = scene->lights.size() * ns_area_light;
    
    for (auto p = scene->lights.begin(); p != scene->lights.end(); p++) {
        for (int i = 0; i < ns_area_light; i++) {
            // sample at the direction of the light
            Vector3D w_in;
            double disToLight;
            double pdf;
            Vector3D emission = (*p)->sample_L(hit_p, &w_in, &disToLight, &pdf);
            
            // light behind the surface at the hit point
            if (dot(w_in, isect.n) < 0) {
                continue;
            }
            
            // cast a ray in the sampled direction
            Ray sample_ray = Ray(hit_p, w_in);
            sample_ray.min_t = EPS_F;
            sample_ray.max_t = disToLight - EPS_F;
            // if no object between the hit point and the light
            if (!bvh->has_intersection(sample_ray)) {
                Vector3D f = isect.bsdf->f(w_out, w2o * w_in);
                L_out += f * emission * cos_theta(w2o * w_in) / pdf / num_samples;
                
                // optimize for point light source
                if ((*p)->is_delta_light()) {
                    L_out += (f * emission * cos_theta(w2o * w_in) / pdf / num_samples) * (ns_area_light - 1);
                    break;
                }
            }
        }
    }
    return L_out;
}

Vector3D PathTracer::zero_bounce_radiance(const Ray &r,
                                          const Intersection &isect) {
  // TODO: Part 3, Task 2
  // Returns the light that results from no bounces of light
  return isect.bsdf->get_emission();
}

Vector3D PathTracer::one_bounce_radiance(const Ray &r,
                                         const Intersection &isect) {
  // TODO: Part 3, Task 3
  // Returns either the direct illumination by hemisphere or importance sampling
  // depending on `direct_hemisphere_sample`

    if (direct_hemisphere_sample) {
        return estimate_direct_lighting_hemisphere(r, isect);
    } else {
        return estimate_direct_lighting_importance(r, isect);
    }
}

Vector3D PathTracer::at_least_one_bounce_radiance(const Ray &r,
                                                  const Intersection &isect) {
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  Vector3D hit_p = r.o + r.d * isect.t;
  Vector3D w_out = w2o * (-r.d);

  Vector3D L_out(0, 0, 0);

  // TODO: Part 4, Task 2
  // Returns the one bounce radiance + radiance from extra bounces at this point.
  // Should be called recursively to simulate extra bounces.
    
    // russian roulette continuation probability
    double p = 0.7;
    if (!isect.bsdf->is_delta()) {
        L_out += one_bounce_radiance(r, isect);
    }
    
    // keep tracing with probability p and if there is depth left
    if (r.depth > 1 && coin_flip(p)) {
        // sample a incoming direction
        Vector3D w_in;
        double pdf;
        Vector3D f = isect.bsdf->sample_f(w_out, &w_in, &pdf);
            
        // cast a ray
        Ray sample_ray = Ray(hit_p, (o2w * w_in).unit(), INF_D, r.depth - 1);
        sample_ray.min_t = EPS_F;
            
        // recurse if the sampled ray intersects the scene
        Intersection sample_i;
        if (bvh->intersect(sample_ray, &sample_i)) {
            Vector3D l = at_least_one_bounce_radiance(sample_ray, sample_i);
            //Vector3D f_l = Vector3D(f[0] * l[0], f[1] * l[1], f[2] * l[2]);
            if (isect.bsdf->is_delta()) {
                l += zero_bounce_radiance(sample_ray, sample_i);
            }
            Vector3D f_l = f * l;
            L_out += f_l * abs_cos_theta(w_in) / pdf / p;
        }
    }

  return L_out;
}

Vector3D PathTracer::est_radiance_global_illumination(const Ray &r) {
  Intersection isect;
  Vector3D L_out;

  // You will extend this in assignment 3-2.
  // If no intersection occurs, we simply return black.
  // This changes if you implement hemispherical lighting for extra credit.

  // The following line of code returns a debug color depending
  // on whether ray intersection with triangles or spheres has
  // been implemented.
  //
  // REMOVE THIS LINE when you are ready to begin Part 3.
  
  if (!bvh->intersect(r, &isect))
    return envLight ? envLight->sample_dir(r) : L_out;


  //L_out = (isect.t == INF_D) ? debug_shading(r.d) : normal_shading(isect.n);
    
    
  // TODO (Part 3): Return the direct illumination.

    L_out = zero_bounce_radiance(r, isect);
    //L_out += one_bounce_radiance(r, isect);

    
  // TODO (Part 4): Accumulate the "direct" and "indirect"
  // parts of global illumination into L_out rather than just direct
    if (max_ray_depth >= 1) {
        L_out += at_least_one_bounce_radiance(r, isect);
    }

  return L_out;
}

void PathTracer::raytrace_pixel(size_t x, size_t y) {
  // TODO (Part 1.2):
  // Make a loop that generates num_samples camera rays and traces them
  // through the scene. Return the average Vector3D.
  // You should call est_radiance_global_illumination in this function.

  // TODO (Part 5):
  // Modify your implementation to include adaptive sampling.
  // Use the command line parameters "samplesPerBatch" and "maxTolerance"

  int num_samples = ns_aa;          // total samples to evaluate
  Vector2D origin = Vector2D(x, y); // bottom left corner of the pixel
    
    Vector3D radiance = Vector3D(0, 0, 0);
    
    double s1 = 0;
    double s2 = 0;
    double mean, sd, I;
    int i;
    for (i = 0; i < num_samples; i++) {
        Vector2D sample = gridSampler->get_sample() + origin;
        Vector2D samplesForLens = gridSampler->get_sample();
        Ray ray = camera->generate_ray_for_thin_lens(sample[0] / sampleBuffer.w, sample[1] / sampleBuffer.h, samplesForLens.x, samplesForLens.y * 2.0 * PI);
        double min_t = ray.min_t;
        ray = Ray(ray.o, ray.d, ray.max_t, max_ray_depth);
        ray.min_t = min_t;
        Vector3D sample_radiance = est_radiance_global_illumination(ray);
        radiance += sample_radiance;
        
        s1 += sample_radiance.illum();
        s2 += pow(sample_radiance.illum(), 2);
        if ((i + 1) % samplesPerBatch == 0) {
            mean = s1 / (double) (i + 1);
            sd = sqrt( (1.0 / (double) i) * (s2 - (pow(s1, 2) / (double) (i + 1))) );
            I = 1.96 * sd / sqrt((double)(i + 1));
            if (I <= maxTolerance * mean) {
                i++;
                break;
            }
        }
    }
  sampleBuffer.update_pixel(radiance / i, x, y);
  sampleCountBuffer[x + y * sampleBuffer.w] = i;
}

void PathTracer::autofocus(Vector2D loc) {
  Ray r = camera->generate_ray(loc.x / sampleBuffer.w, loc.y / sampleBuffer.h);
  Intersection isect;

  bvh->intersect(r, &isect);

  camera->focalDistance = isect.t;
}

} // namespace CGL
