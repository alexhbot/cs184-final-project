#include "bbox.h"

#include "GL/glew.h"

#include <algorithm>
#include <iostream>
#include <vector>

namespace CGL {

bool BBox::intersect(const Ray& r, double& t0, double& t1) const {
  // TODO (Part 2.2):
  // Implement ray - bounding box intersection test
  // If the ray intersected the bouding box within the range given by
  // t0, t1, update t0 and t1 with the new intersection times.
    double t_min = r.min_t - 1;
    double t_max = r.max_t + 1;
    
    if (r.d[0]) {
        double p1 = (min[0] - r.o[0]) / r.d[0];
        double p2 = (max[0] - r.o[0]) / r.d[0];
        t_min = std::max(std::min(p1, p2), t_min);
        t_max = std::min(std::max(p1, p2), t_max);
    } else {
        if (r.o[0] < min[0] || r.o[0] > max[0]) {
            return false;
        }
    }
    
    if (r.d[1]) {
        double p1 = (min[1] - r.o[1]) / r.d[1];
        double p2 = (max[1] - r.o[1]) / r.d[1];
        t_min = std::max(std::min(p1, p2), t_min);
        t_max = std::min(std::max(p1, p2), t_max);
    } else {
        if (r.o[1] < min[1] || r.o[1] > max[1]) {
            return false;
        }
    }
    
    if (r.d[2]) {
        double p1 = (min[2] - r.o[2]) / r.d[2];
        double p2 = (max[2] - r.o[2]) / r.d[2];
        t_min = std::max(std::min(p1, p2), t_min);
        t_max = std::min(std::max(p1, p2), t_max);
    } else {
        if (r.o[2] < min[2] || r.o[2] > max[2]) {
            return false;
        }
    }
    
    if (t_min > t_max || t_max < t0 || t_min > t1) {
        return false;
    } else {
        t0 = std::max(t_min, t0);
        t1 = std::min(t_max, t1);
        return true;
    }
}

void BBox::draw(Color c, float alpha) const {

  glColor4f(c.r, c.g, c.b, alpha);

  // top
  glBegin(GL_LINE_STRIP);
  glVertex3d(max.x, max.y, max.z);
  glVertex3d(max.x, max.y, min.z);
  glVertex3d(min.x, max.y, min.z);
  glVertex3d(min.x, max.y, max.z);
  glVertex3d(max.x, max.y, max.z);
  glEnd();

  // bottom
  glBegin(GL_LINE_STRIP);
  glVertex3d(min.x, min.y, min.z);
  glVertex3d(min.x, min.y, max.z);
  glVertex3d(max.x, min.y, max.z);
  glVertex3d(max.x, min.y, min.z);
  glVertex3d(min.x, min.y, min.z);
  glEnd();

  // side
  glBegin(GL_LINES);
  glVertex3d(max.x, max.y, max.z);
  glVertex3d(max.x, min.y, max.z);
  glVertex3d(max.x, max.y, min.z);
  glVertex3d(max.x, min.y, min.z);
  glVertex3d(min.x, max.y, min.z);
  glVertex3d(min.x, min.y, min.z);
  glVertex3d(min.x, max.y, max.z);
  glVertex3d(min.x, min.y, max.z);
  glEnd();

}

std::ostream& operator<<(std::ostream& os, const BBox& b) {
  return os << "BBOX(" << b.min << ", " << b.max << ")";
}

} // namespace CGL
