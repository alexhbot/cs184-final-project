#include "camera.h"
#include "bsdf.h"
#include "scene/sphere.h"
#include "intersection.h"


#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include "CGL/misc.h"
#include "CGL/vector2D.h"
#include "CGL/vector3D.h"

using std::cout;
using std::endl;
using std::max;
using std::min;
using std::ifstream;
using std::ofstream;

namespace CGL {

using namespace CGL::SceneObjects;
using Collada::CameraInfo;

Ray Camera::generate_ray_for_thin_lens(double x, double y, double rndR, double rndTheta) const {

  // TODO Project 3-2: Part 4
  // compute position and direction of ray from the input sensor sample coordinate.
  // Note: use rndR and rndTheta to uniformly sample a unit disk.
    double bl_corner_x = -tan(0.5 * hFov * ( M_PI / 180.0 ));
    double bl_corner_y = -tan(0.5 * vFov * ( M_PI / 180.0 ));
    double tr_corner_x = tan(0.5 * hFov * ( M_PI / 180.0 ));
    double tr_corner_y = tan(0.5 * vFov * ( M_PI / 180.0 ));
    
    double camera_x = bl_corner_x + (tr_corner_x - bl_corner_x) * x;
    double camera_y = bl_corner_y + (tr_corner_y - bl_corner_y) * y;
    
    Vector3D pfilm = Vector3D(-camera_x, -camera_y, 1.0);
    
    Ray center_ray = Ray(pfilm, (Vector3D() - pfilm).unit());
     
    Vector3D plens = Vector3D(lensRadius * sqrt(rndR) * cos(rndTheta), lensRadius * sqrt(rndR) * sin(rndTheta), 0);
    double t = (-focalDistance - center_ray.o.z) / center_ray.d.z;
    Vector3D pfocus = center_ray.at_time(t);
    
    Ray ray = Ray((c2w * plens) + pos, c2w * (pfocus - plens).unit());
    ray.min_t = nClip;
    ray.max_t = fClip;
    
    return ray;
}

// Scale down the camera system to fit the scene, since original lens spec is in millimeters
#define scale_camera_to_scene 0.03

void Camera::initialize_zoom_lens() {
    
    double x = scale_camera_to_scene;
    
    // Zoom parameters
    double one_two_wide = 4.54;
    double one_two_tele = 38.63;
    
    double five_six_wide = 70.03;
    double five_six_tele = 3.00;
    
    // Set up the lens elements
    // From the back-most element to the front-most element
    lens_elem_one = new Lens(0.0,   //pos
                             x * 45.0/2.0,   //radius
                             x * 200.00,  //front_radius_of_curvature
                             x * -203.00,  //front_sphere_offset
                             1.0/1.589,  //front_ior
                             x * 289.81,  //back_radius_of_curvature
                             x * -289.81, //back_sphere_offset
                             1.589); //back_ior
    lens_elem_two = new Lens(lens_elem_one->pos - x * (3.0 + (one_two_wide * (1.0 - zoom_index) + one_two_wide * zoom_index)),  //pos
                             x * 29.0/2.0,  //radius
                             x * 111.55,  //front_radius_of_curvature
                             x * -115.63,  //front_sphere_offset
                             1.0/1.689,  //front_ior
                             x * 47.47,  //back_radius_of_curvature
                             x * -47.47,  //back_sphere_offset
                             1.689); //back_ior
    lens_elem_three = new Lens(lens_elem_two->pos - x * 15.26,  //pos
                               x * 32.0/2.0,  //radius
                               x * 199.02,  //front_radius_of_curvature
                               x * -207.14,  //front_sphere_offset
                               1.0/1.805,  //front_ior
                               x * 31.67,  //back_radius_of_curvature
                               x * 31.67,  //back_sphere_offset
                               1.0/1.805); //back_ior
    lens_elem_four = new Lens(lens_elem_three->pos - x * 9.20,    //pos
                              x * 34.0/2.0,    //radius
                              x * 40.32,   //front_radius_of_curvature
                              x * 34.67,   //front_sphere_offset
                              1.720,   //front_ior
                              x * 302.87,   //back_radius_of_curvature
                              x * 302.87,  //back_sphere_offset
                              1.0/1.720);  //back_ior
    lens_elem_five = new Lens(lens_elem_four->pos - x * 11.05,  //pos
                              x * 48.0/2.0,    //radius
                              x * 60.56,   //front_radius_of_curvature
                              x * 54.45,   //front_sphere_offset
                              1.713,   //front_ior
                              x * 238.75,   //back_radius_of_curvature
                              x * -238.75,   //back_sphere_offset
                              1.713);  //back_ior
    lens_elem_six = new Lens(lens_elem_five->pos - x * (6.11 + (five_six_wide * (1.0 - zoom_index) + five_six_tele * zoom_index)),    //pos
                             x * 68.0/2.0,    //radius
                             x * 53.35,    //front_radius_of_curvature
                             x * 44.42, //front_sphere_offset
                             1.805,   //front_ior
                             x * 114.94,   //back_radius_of_curvature
                             x * 114.94,      //back_sphere_offset
                             1.0/1.805);  //back_ior
    lens_elem_seven = new Lens(lens_elem_six->pos - x * 14.27,    //pos
                               x * 68.0/2.0,    //radius
                               x * 78.96,    //front_radius_of_curvature
                               x * 75.90, //front_sphere_offset
                               1.773,   //front_ior
                               x * 53.77,   //back_radius_of_curvature
                               x * 53.77,      //back_sphere_offset
                               1.0/1.773);  //back_ior
    lens_elem_eight = new Lens(lens_elem_seven->pos - x * 7.58,    //pos
                               x * 80.0/2.0,    //radius
                               x * 231.88,    //front_radius_of_curvature
                               x * 228.58, //front_sphere_offset
                               1.834,   //front_ior
                               x * 52.68,   //back_radius_of_curvature
                               x * 52.68,      //back_sphere_offset
                               1.0/1.834);  //back_ior
}

Ray Camera::generate_ray_for_zoom_lens(double x, double y, double rndR, double rndTheta) const {
    
    // distance from sensor to the closest lens element
    double dist = scale_camera_to_scene * 75.61;
    
    // radius of the back-most lens element
    double r = lens_elem_one->radius * max(lensRadius, 0.000000000001);
    
    // find point on the image plane, pFilm
    double bl_corner_x = -tan(0.5 * hFov * ( M_PI / 180.0 ));
    double bl_corner_y = -tan(0.5 * vFov * ( M_PI / 180.0 ));
    double tr_corner_x = tan(0.5 * hFov * ( M_PI / 180.0 ));
    double tr_corner_y = tan(0.5 * vFov * ( M_PI / 180.0 ));
    
    double camera_x = bl_corner_x + (tr_corner_x - bl_corner_x) * x;
    double camera_y = bl_corner_y + (tr_corner_y - bl_corner_y) * y;
    
    // scale the image plane to the size of a full frame sensor (24mm tall), so that size relative to the lens is correct
    double sensor_scale_factor = 24.0 / (tr_corner_y - bl_corner_y);
    
    Vector3D pfilm = Vector3D(-camera_x * sensor_scale_factor * scale_camera_to_scene,
                              -camera_y * sensor_scale_factor * scale_camera_to_scene,
                              dist - 10 * scale_camera_to_scene * focalDistance);
    
    // find sample point on the closest lens element, pLens
    Vector3D plens = Vector3D(r * sqrt(rndR) * cos(rndTheta), r * sqrt(rndR) * sin(rndTheta), 0);
    
    Ray ray = Ray(pfilm, (plens - pfilm).unit());
    ray.max_t = INF_D;
    ray.min_t = EPS_F;
    
    if (!lens_elem_one->refract(ray, &ray)) {
        return Ray();
    }
    if (!lens_elem_two->refract(ray, &ray)) {
        return Ray();
    }
    if (!lens_elem_three->refract(ray, &ray)) {
        return Ray();
    }
    if (!lens_elem_four->refract(ray, &ray)) {
        return Ray();
    }
    if (!lens_elem_five->refract(ray, &ray)) {
        return Ray();
    }
    if (!lens_elem_six->refract(ray, &ray)) {
        return Ray();
    }
    if (!lens_elem_seven->refract(ray, &ray)) {
        return Ray();
    }
    if (!lens_elem_eight->refract(ray, &ray)) {
        return Ray();
    }

    // Configure the final generated ray
    // Offset the ray to reduce the impact of lens length
    ray.o.z = 0;
    
    ray = Ray((c2w * ray.o) + pos, c2w * ray.d);
    ray.min_t = nClip;
    ray.max_t = fClip;
    
    return ray;
}

bool Lens::refract(Ray in_ray, Ray *out_ray) const {
    
    Ray temp_ray;
    
    //********************************************
    //* Step 1: refract through the back surface *
    //********************************************
    
    // Step 1.a: construct sphere of back surface and intersect ray with it
    SphereObject back_surface_obj = SphereObject(Vector3D(0.0, 0.0, pos + back_sphere_offset),
                                                 back_radius_of_curvature,
                                                 new RefractionBSDF(Vector3D(), 0, back_ior));
    Sphere back_surface = Sphere(&back_surface_obj, back_surface_obj.o, back_surface_obj.r);
    Intersection back_isect;
    if (!back_surface.intersect(in_ray, &back_isect)) {
        return false;
    }
    
    // check if the ray misses the lens element
    if (Vector3D(in_ray.at_time(back_isect.t).x, in_ray.at_time(back_isect.t).y, 0).norm() >= radius) {
        return false;
    }
    
    // Step 1.b: obtain transition matrices for local refraction calculation
    Matrix3x3 o2w;
    make_coord_space(o2w, back_isect.n);
    Matrix3x3 w2o = o2w.T();
    
    // Step 1.c: calculate refracted ray in local space
    Vector3D w_in;
    if (!back_isect.bsdf->refract(w2o * (-in_ray.d), &w_in, back_ior)) {
        return false;
    }
    
    // Step 1.d: transform the refracted ray back to world space (camera space)
    temp_ray = Ray(in_ray.at_time(back_isect.t), (o2w * w_in).unit());
    temp_ray.max_t = INF_D;
    temp_ray.min_t = EPS_F;
    
    
    //*********************************************
    //* Step 2: refract through the front surface *
    //*********************************************
    
    // Step 2.a: construct sphere of front surface and intersect ray with it
    SphereObject front_surface_obj = SphereObject(Vector3D(0.0, 0.0, pos + front_sphere_offset),
                                                 front_radius_of_curvature,
                                                 new RefractionBSDF(Vector3D(), 0, front_ior));
    Sphere front_surface = Sphere(&front_surface_obj, front_surface_obj.o, front_surface_obj.r);
    Intersection front_isect;
    if (!front_surface.intersect(temp_ray, &front_isect)) {
        return false;
    }
    
    // check if the ray misses the lens element
    if (Vector3D(temp_ray.at_time(front_isect.t).x, temp_ray.at_time(front_isect.t).y, 0).norm() >= radius) {
        return false;
    }
    
    // Step 2.b: obtain transition matrices for local refraction calculation
    make_coord_space(o2w, front_isect.n);
    w2o = o2w.T();
    
    // Step 2.c: calculate refracted ray in local space
    if (!front_isect.bsdf->refract(w2o * (-temp_ray.d), &w_in, front_ior)) {
        return false;
    }
    
    // Step 2.d: transform the refracted ray back to world space (camera space)
    *out_ray = Ray(temp_ray.at_time(front_isect.t), (o2w * w_in).unit());
    out_ray->max_t = INF_D;
    out_ray->min_t = EPS_F;
    
    // Slightly offset the ray origin to allow for back-to-back lens elements
    out_ray->o = out_ray->o - (0.000001 * out_ray->d);
    
    return true;
}

}// namespace CGL
