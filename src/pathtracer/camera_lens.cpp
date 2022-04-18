#include "camera.h"
#include "bsdf.h"

#include <iostream>
#include <sstream>
#include <fstream>

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


} // namespace CGL
