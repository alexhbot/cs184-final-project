#include "triangle.h"

#include "CGL/CGL.h"
#include "GL/glew.h"

namespace CGL {
namespace SceneObjects {

Triangle::Triangle(const Mesh *mesh, size_t v1, size_t v2, size_t v3) {
  p1 = mesh->positions[v1];
  p2 = mesh->positions[v2];
  p3 = mesh->positions[v3];
  n1 = mesh->normals[v1];
  n2 = mesh->normals[v2];
  n3 = mesh->normals[v3];
  bbox = BBox(p1);
  bbox.expand(p2);
  bbox.expand(p3);

  bsdf = mesh->get_bsdf();
}

BBox Triangle::get_bbox() const { return bbox; }

bool Triangle::has_intersection(const Ray &r) const {
  // Part 1, Task 3: implement ray-triangle intersection
  // The difference between this function and the next function is that the next
  // function records the "intersection" while this function only tests whether
  // there is a intersection.
    return intersect(r, NULL);
}

bool Triangle::intersect(const Ray &r, Intersection *isect) const {
  // Part 1, Task 3:
  // implement ray-triangle intersection. When an intersection takes
  // place, the Intersection data should be updated accordingly
    Vector3D normal = cross((p2 - p1), (p3 - p1));
    double t = dot(p1 - r.o, normal) / dot(r.d, normal);
    if (t < r.min_t || t > r.max_t) {
        return false;
    }
    
    double x;
    double y;
    
    double x0;
    double y0;

    double x1;
    double y1;
    
    double x2;
    double y2;
    
    
    if (normal[0] != 0) {
        //collapse x
        x = r.at_time(t)[1];
        y = r.at_time(t)[2];

        x0 = p1[1];
        y0 = p1[2];
        
        x1 = p2[1];
        y1 = p2[2];
        
        x2 = p3[1];
        y2 = p3[2];
    } else if (normal[1] != 0) {
        //collapse y
        x = r.at_time(t)[0];
        y = r.at_time(t)[2];
        
        x0 = p1[0];
        y0 = p1[2];
        
        x1 = p2[0];
        y1 = p2[2];
        
        x2 = p3[0];
        y2 = p3[2];
    } else {
        //collapse z
        x = r.at_time(t)[0];
        y = r.at_time(t)[1];
        
        x0 = p1[0];
        y0 = p1[1];
        
        x1 = p2[0];
        y1 = p2[1];
        
        x2 = p3[0];
        y2 = p3[1];
    }
    
    if (-(x2-x0)*(y1-y0)+(y2-y0)*(x1-x0)>0) {
        //counter-clockwise
        if (-(x-x0)*(y1-y0)+(y-y0)*(x1-x0)>=0
            && -(x-x1)*(y2-y1)+(y-y1)*(x2-x1)>=0
            && -(x-x2)*(y0-y2)+(y-y2)*(x0-x2)>=0) {
            goto valid_intersection;
        }
    } else {
        //clockwise
        if (-(x-x0)*(y1-y0)+(y-y0)*(x1-x0)<=0
            && -(x-x1)*(y2-y1)+(y-y1)*(x2-x1)<=0
            && -(x-x2)*(y0-y2)+(y-y2)*(x0-x2)<=0) {
            goto valid_intersection;
        }
    }
    
  return false;
    
valid_intersection:
    r.max_t = t;
    if (isect != NULL) {
        float alpha = (-(x-x1)*(y2-y1)+(y-y1)*(x2-x1))/(-(x0-x1)*(y2-y1)+(y0-y1)*(x2-x1));
        float beta = (-(x-x2)*(y0-y2)+(y-y2)*(x0-x2))/(-(x1-x2)*(y0-y2)+(y1-y2)*(x0-x2));
        float gamma = 1 - alpha - beta;
        isect->t = t;
        isect->n = alpha * n1 + beta * n2 + gamma * n3;
        isect->primitive = this;
        isect->bsdf = get_bsdf();
    }
    return true;
}

void Triangle::draw(const Color &c, float alpha) const {
  glColor4f(c.r, c.g, c.b, alpha);
  glBegin(GL_TRIANGLES);
  glVertex3d(p1.x, p1.y, p1.z);
  glVertex3d(p2.x, p2.y, p2.z);
  glVertex3d(p3.x, p3.y, p3.z);
  glEnd();
}

void Triangle::drawOutline(const Color &c, float alpha) const {
  glColor4f(c.r, c.g, c.b, alpha);
  glBegin(GL_LINE_LOOP);
  glVertex3d(p1.x, p1.y, p1.z);
  glVertex3d(p2.x, p2.y, p2.z);
  glVertex3d(p3.x, p3.y, p3.z);
  glEnd();
}

} // namespace SceneObjects
} // namespace CGL
