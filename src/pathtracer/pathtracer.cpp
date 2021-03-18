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
  int num_samples = scene->lights.size() * ns_area_light;
  Vector3D L_out;
   

  // TODO (Part 3): Write your sampling loop here
  // TODO BEFORE YOU BEGIN
  // UPDATE `est_radiance_global_illumination` to return direct lighting instead of normal shading
    for(int ii=0; ii <num_samples; ii++){
        Vector3D w_i = hemisphereSampler->get_sample();
        Vector3D f = isect.bsdf->f(w_out,w_i);
        Ray ray_out = Ray(hit_p, o2w*w_i);
        ray_out.min_t = EPS_F;
        double cos = w_i.z;
        Intersection light;
        if(bvh->intersect(ray_out,&light)){
            Vector3D light_n = light.bsdf->get_emission();
            L_out += light_n*f*cos*2*PI;
        }
        
    }
  return L_out/num_samples;

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
  
    for (auto& l : scene->lights){
        if(l->is_delta_light()){
            Vector3D w_i;
            double distToLight;
            double pdf;
            Vector3D illum = l->sample_L(hit_p, &w_i, &distToLight, &pdf);
            Vector3D object_w_i = w2o*w_i;
            if(object_w_i.z>0){
                Ray ray_out = Ray(hit_p, w_i,distToLight-EPS_F);
                ray_out.min_t = EPS_F;
                Intersection light;
                if(!bvh->intersect(ray_out, &light)){
                        L_out += object_w_i.z*illum*isect.bsdf->f(w_out,object_w_i)/pdf;
                }
            }
        }
        else{
            for(int ii=0; ii<ns_area_light; ii++){
                Vector3D w_i;
                double distToLight;
                double pdf;
                Vector3D illum = l->sample_L(hit_p, &w_i, &distToLight, &pdf);
                Vector3D object_w_i = w2o*w_i;
                if(object_w_i.z>0){
                    Ray ray_out = Ray(hit_p, w_i,distToLight-EPS_F);
                    ray_out.min_t = EPS_F;
                    Intersection light;
                    if(!bvh->intersect(ray_out, &light)){
                            L_out += object_w_i.z*illum*isect.bsdf->f(w_out,object_w_i)/pdf/ns_area_light;
                    }
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
//    isect.bsdf->get_emission();
    


    return isect.bsdf->get_emission();;


}

Vector3D PathTracer::one_bounce_radiance(const Ray &r,
                                         const Intersection &isect) {
  // TODO: Part 3, Task 3
  // Returns either the direct illumination by hemisphere or importance sampling
  // depending on `direct_hemisphere_sample`
    

    if(direct_hemisphere_sample){
        return estimate_direct_lighting_hemisphere(r, isect);
    }
    else{
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
  
    if (!bvh->intersect(r, &isect)){
        return  L_out;}
  else{
    L_out = zero_bounce_radiance(r, isect);
        L_out += one_bounce_radiance(r, isect);
    }


//  L_out = (isect.t == INF_D) ? debug_shading(r.d) : normal_shading(isect.n);

  // TODO (Part 3): Return the direct illumination.

  // TODO (Part 4): Accumulate the "direct" and "indirect"
  // parts of global illumination into L_out rather than just direct

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
  Vector3D sample = Vector3D(0,0,0);
    for(int ii=0; ii<num_samples; ii++){
        Vector2D sampleLoc = origin + gridSampler->get_sample();
        Vector3D pathColor = est_radiance_global_illumination(camera->generate_ray(sampleLoc.x/sampleBuffer.w, sampleLoc.y/sampleBuffer.h));
        sample = sample + pathColor*1/num_samples;
    }
    
  


  sampleBuffer.update_pixel(sample, x, y);
  sampleCountBuffer[x + y * sampleBuffer.w] = num_samples;


}

void PathTracer::autofocus(Vector2D loc) {
  Ray r = camera->generate_ray(loc.x / sampleBuffer.w, loc.y / sampleBuffer.h);
  Intersection isect;

  bvh->intersect(r, &isect);

  camera->focalDistance = isect.t;
}

} // namespace CGL
