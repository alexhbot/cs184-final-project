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

void Camera::initialize_zoom_lens(double i) {
    // standard element positions: 0.0, -1.0, -1.5, -3.3, -3.6
    // standard aperture position: -1.7
    
    double one_two_wide = 0.79;
    double one_two_tele = 2.87;
    
    double four_five_wide = 1.47;
    double four_five_tele = 0.24;
    
    // From the back-most element to the front-most element
    //Lens(
    //     double pos,
    //     double radius,
    //     double front_radius_of_curvature,
    //     double front_sphere_offset,
    //     double front_ior,
    //     double back_radius_of_curvature,
    //     double back_sphere_offset,
    //     double back_ior
    //     )
    lens_elem_one = new Lens(0.0,   //pos
                             1.5/2.0,   //radius
                             7.14,  //front_radius_of_curvature
                             7.04,  //front_sphere_offset
                             1.77,  //front_ior
                             2.46,  //back_radius_of_curvature
                             -2.36, //back_sphere_offset
                             1.77); //back_ior
    lens_elem_two = new Lens(lens_elem_one->pos - (one_two_wide + zoom_index * (one_two_tele - one_two_wide)) - 0.1 - 0.03,  //pos
                             0.75/2.0,  //radius
                             0.76,  //front_radius_of_curvature
                             -0.78,  //front_sphere_offset
                             1.0/1.72,  //front_ior *******************
                             0.57,  //back_radius_of_curvature
                             0.60,  //back_sphere_offset
                             1.72); //back_ior *******************
    lens_elem_three = new Lens(lens_elem_two->pos - 0.19,  //pos
                             0.75/2.0,  //radius
                             1.87,  //front_radius_of_curvature
                             1.70,  //front_sphere_offset
                             1.83,  //front_ior
                             0.76,  //back_radius_of_curvature
                             -0.59,  //back_sphere_offset
                             1.83); //back_ior
    lens_elem_four = new Lens(lens_elem_three->pos - 0.36,    //pos
                            0.8/2.0,    //radius
                            0.93,   //front_radius_of_curvature
                            0.83,   //front_sphere_offset
                            1.62,   //front_ior
                            2.35,   //back_radius_of_curvature
                            -2.25,  //back_sphere_offset
                            1.62);  //back_ior
    aperture_pos = lens_elem_four->pos - 0.19;
    aperture_radius = 0.8/2.0;
    lens_elem_five = new Lens(aperture_pos - (four_five_wide + zoom_index * (four_five_tele - four_five_wide)) - 0.05,  //pos
                            1.3/2.0,    //radius
                            1.47,   //front_radius_of_curvature
                            1.37,   //front_sphere_offset
                            1.92,   //front_ior
                            3.33,   //back_radius_of_curvature
                            3.38,   //back_sphere_offset
                            1.92);  //back_ior *******************
    lens_elem_six = new Lens(lens_elem_five->pos - 0.37,    //pos
                            1.5/2.0,    //radius
                            1000000000.6,    //front_radius_of_curvature
                            1000000000, //front_sphere_offset
                            1.88,   //front_ior
                            0.99,   //back_radius_of_curvature
                            1,      //back_sphere_offset
                            1.88);  //back_ior *******************
}

Ray Camera::generate_ray_for_zoom_lens(double x, double y, double rndR, double rndTheta) const {
    
    double sensor_one_wide = 0.49;
    double sensor_one_tele = 0.29;
    
    // distance from sensor to the closest lens element
    double dist = 4.5 + zoom_index * 18 + (sensor_one_wide + zoom_index * (sensor_one_tele - sensor_one_wide)) + 0.10 + 0.10 + 0.17;
    
    // radius of the closest lens element
    double r = 0.08/2.0;
    
    // find point on the image plane, pFilm
    double bl_corner_x = -tan(0.5 * hFov * ( M_PI / 180.0 ));
    double bl_corner_y = -tan(0.5 * vFov * ( M_PI / 180.0 ));
    double tr_corner_x = tan(0.5 * hFov * ( M_PI / 180.0 ));
    double tr_corner_y = tan(0.5 * vFov * ( M_PI / 180.0 ));
    
    double camera_x = bl_corner_x + (tr_corner_x - bl_corner_x) * x;
    double camera_y = bl_corner_y + (tr_corner_y - bl_corner_y) * y;
    
    double scale_factor = 1.125 / (tr_corner_y - bl_corner_y);
    
    Vector3D pfilm = Vector3D(camera_x * scale_factor, camera_y * scale_factor, dist + focus_offset);
    
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
    
    if (!pass_aperture(ray)) {
        return Ray();
    }
    
    if (!lens_elem_five->refract(ray, &ray)) {
        return Ray();
    }
    if (!lens_elem_six->refract(ray, &ray)) {
        return Ray();
    }
    
    
    ray = Ray((c2w * ray.o) + pos, c2w * ray.d);
    ray.min_t = nClip;
    ray.max_t = fClip;
    
    return ray;
}

bool Lens::refract(Ray in_ray, Ray *out_ray) const {
    
    // check if the ray misses the lens element
//    double t = (pos - in_ray.o.z) / in_ray.d.z;
//    Vector3D p = in_ray.at_time(t);
//    if ((Vector3D(0.0, 0.0, pos) - p).norm() >= radius) {
//        return false;
//    }
    
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
    
    out_ray->o = out_ray->o - (out_ray->d);
    
    return true;
}

bool Camera::pass_aperture(Ray in_ray) const {

    double t = (aperture_pos - in_ray.o.z) / in_ray.d.z;
    Vector3D p = in_ray.at_time(t);
    
    if ((Vector3D(0.0, 0.0, aperture_pos) - p).norm() > (lensRadius * aperture_radius)) {
        return false;
    } else {
        return true;
    }
    
}

}// namespace CGL
