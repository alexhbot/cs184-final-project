#ifndef CGL_CAMERA_H
#define CGL_CAMERA_H

#include <iostream>
#include <vector>

#include "scene/collada/camera_info.h"
#include "CGL/matrix3x3.h"

#include "math.h"
#include "ray.h"


namespace CGL {

/**
 * Lens element.
 */
struct Lens {
    
    double pos; ///z dimension of the element in camera space
    double radius;  ///radius of the element
    double front_radius_of_curvature;   ///the radius of curvature of the front surface
    double front_sphere_offset; ///offset in z dimension of the front surface's sphere's origin from pos
    double front_ior;  ///index of refraction of the front surface's sphere
    double back_radius_of_curvature;   ///the radius of curvature of the back surface
    double back_sphere_offset; ///offset in z dimension of the back surface's sphere's origin from pos
    double back_ior;  ///index of refraction of the back surface's sphere
    
    Lens(){}
    
    /**
     * Constructor.
     * Create a lens element instance with given position and geometries.
     * \param pos z dimension of the element in camera space
     * \param radius radius of the element
     * \param front_radius_of_curvature the radius of curvature of the front surface
     * \param front_sphere_offset offset in z dimension of the front surface's sphere's origin from pos
     * \param front_ior index of refraction of the front surface's sphere
     * \param back_radius_of_curvature the radius of curvature of the back surface
     * \param back_sphere_offset offset in z dimension of the back surface's sphere's origin from pos
     * \param back_ior index of refraction of the back surface's sphere
     */
    Lens(
         double pos,
         double radius,
         double front_radius_of_curvature,
         double front_sphere_offset,
         double front_ior,
         double back_radius_of_curvature,
         double back_sphere_offset,
         double back_ior
         )
        :   pos(pos),
    radius(radius),
    front_radius_of_curvature(front_radius_of_curvature),
    front_sphere_offset(front_sphere_offset),
    front_ior(front_ior),
    back_radius_of_curvature(back_radius_of_curvature),
    back_sphere_offset(back_sphere_offset),
    back_ior(back_ior) {}
    
    /**
     * Takes in a ray coming from the back, and an address to store the ray refracted to the front.
     * Return true iff the ray successfully passes through the lens without hitting the lens barrel or the aperture blades.
     */
    bool refract(Ray in_ray, Ray *out_ray) const;
};

/**
 * Camera.
 */
class Camera {
 public:
  /*
    Sets the field of view to match screen screenW/H.
    NOTE: data and screenW/H will almost certainly disagree about the aspect
          ratio. screenW/H are treated as the source of truth, and the field
          of view is expanded along whichever dimension is too narrow.
    NOTE2: info.hFov and info.vFov are expected to be in DEGREES.
  */
  void configure(const Collada::CameraInfo& info,
                 size_t screenW, size_t screenH);

  /*
    Phi and theta are in RADIANS.
  */
  void place(const Vector3D targetPos, const double phi, const double theta,
             const double r, const double minR, const double maxR);

  string param_string() {
    return "";
  }

  /*
    Copies just placement data from the other camera.
  */
  void copy_placement(const Camera& other);

  /*
    Updates the screen size to be the specified size, keeping screenDist
    constant.
  */
  void set_screen_size(const size_t screenW, const size_t screenH);

  /*
    Translates the camera such that a value at distance d directly in front of
    the camera moves by (dx, dy). Note that dx and dy are in screen coordinates,
    while d is in world-space coordinates (like pos/dir/up).
  */
  void move_by(const double dx, const double dy, const double d);

  /*
    Move the specified amount along the view axis.
  */
  void move_forward(const double dist);

  /*
    Rotate by the specified amount around the target.
  */
  void rotate_by(const double dPhi, const double dTheta);

  Vector3D position() const { return pos; }
  Vector3D view_point() const { return targetPos; }
  Vector3D up_dir() const { return c2w[1]; }
  double v_fov() const { return vFov; }
  double aspect_ratio() const { return ar; }
  double near_clip() const { return nClip; }
  double far_clip() const { return fClip; }

  virtual void dump_settings(std::string filename);
  virtual void load_settings(std::string filename);

  /**
   * Returns a world-space ray from the camera that corresponds to a
   * ray exiting the camera that deposits light at the sensor plane
   * position given by (x,y).  x and y are provided in the normalized
   * coordinate space of the sensor.  For example (0.5, 0.5)
   * corresponds to the middle of the screen.
   *
   * \param x x-coordinate of the ray sample in the view plane
   * \param y y-coordinate of the ray sample in the view plane
   */
  Ray generate_ray(double x, double y) const;

  Ray generate_ray_for_thin_lens(double x, double y, double rndR, double rndTheta) const;
    
    void initialize_zoom_lens(double i) ;
    
    Ray generate_ray_for_zoom_lens(double x, double y, double rndR, double rndTheta) const;
    
    /**
     * Returns true iff the ray passes through the aperture successfully
     */
    bool pass_aperture(Ray in_ray) const;

  // Lens aperture and focal distance for depth of field effects.
  double lensRadius;
  double focalDistance;
  
    // Lens elements, where the first is closest to the sensor, and the last is closest to the object
    Lens* lens_elem_one;
    Lens* lens_elem_two;
    Lens* lens_elem_three;
    Lens* lens_elem_four;
    Lens* lens_elem_five;
    Lens* lens_elem_six;
    
    // position in z-axis of the aperture
    double aperture_pos;
    
    // radius of the aperture
    double aperture_radius;
    
    // between 0 and 1, where 0 is the wide end and 1 is the telephoto end
    double zoom_index = 0;
    
    // to be adjusted by autofocus, can be negative or positive
    double focus_offset = 0;

 private:
  // Computes pos, screenXDir, screenYDir from target, r, phi, theta.
  void compute_position();

  // Field of view aspect ratio, clipping planes.
  double hFov, vFov, ar, nClip, fClip;

  // Current position and target point (the point the camera is looking at).
  Vector3D pos, targetPos;

  // Orientation relative to target, and min & max distance from the target.
  double phi, theta, r, minR, maxR;

  // camera-to-world rotation matrix (note: also need to translate a
  // camera-space point by 'pos' to perform a full camera-to-world
  // transform)
  Matrix3x3 c2w;

  // Info about screen to render to; it corresponds to the camera's full field
  // of view at some distance.
  size_t screenW, screenH;
  double screenDist;
};





} // namespace CGL

#endif // CGL_CAMERA_H
