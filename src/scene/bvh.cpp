#include "bvh.h"

#include "CGL/CGL.h"
#include "triangle.h"

#include <iostream>
#include <stack>

using namespace std;

namespace CGL {
namespace SceneObjects {

BVHAccel::BVHAccel(const std::vector<Primitive *> &_primitives,
                   size_t max_leaf_size) {

  primitives = std::vector<Primitive *>(_primitives);
  root = construct_bvh(primitives.begin(), primitives.end(), max_leaf_size);
}

BVHAccel::~BVHAccel() {
  if (root)
    delete root;
  primitives.clear();
}

BBox BVHAccel::get_bbox() const { return root->bb; }

void BVHAccel::draw(BVHNode *node, const Color &c, float alpha) const {
  if (node->isLeaf()) {
    for (auto p = node->start; p != node->end; p++) {
      (*p)->draw(c, alpha);
    }
  } else {
    draw(node->l, c, alpha);
    draw(node->r, c, alpha);
  }
}

void BVHAccel::drawOutline(BVHNode *node, const Color &c, float alpha) const {
  if (node->isLeaf()) {
    for (auto p = node->start; p != node->end; p++) {
      (*p)->drawOutline(c, alpha);
    }
  } else {
    drawOutline(node->l, c, alpha);
    drawOutline(node->r, c, alpha);
  }
}

BVHNode *BVHAccel::construct_bvh(std::vector<Primitive *>::iterator start,
                                 std::vector<Primitive *>::iterator end,
                                 size_t max_leaf_size) {

  // TODO (Part 2.1):
  // Construct a BVH from the given vector of primitives and maximum leaf
  // size configuration. The starter code build a BVH aggregate with a
  // single leaf node (which is also the root) that encloses all the
  // primitives.


//  BBox bbox;
//
//  for (auto p = start; p != end; p++) {
//    BBox bb = (*p)->get_bbox();
//    bbox.expand(bb);
//  }
//
//  BVHNode *node = new BVHNode(bbox);
//  node->start = start;
//  node->end = end;
//
//  return node;

    BBox bbox;
    size_t count = 0;
    for (auto p = start; p != end; p++) {
        BBox bb = (*p)->get_bbox();
        bbox.expand(bb);
        count++;
    }
    BVHNode *node = new BVHNode(bbox);

    // base case, is a leaf node
    if (count <= max_leaf_size) {
        node->start = start;
        node->end = end;
        return node;
    }

    // find longest axis of bbox
    Vector3D bbox_range = bbox.max - bbox.min;
    int axis;
    for (int i = 0; i < 3; i++) {
        if (bbox_range[i] == max({bbox_range[0], bbox_range[1], bbox_range[2]})) {
            axis = i;
        }
    }

    // compute  split point along the axis
    double split_point = 0;
    for (auto p = start; p != end; p++) {
        Vector3D centroid = (*p)->get_bbox().centroid();
        split_point += centroid[axis] / count;
    }

    // split
    auto split = std::partition(start, end, [axis, split_point](Primitive* p){return p->get_bbox().centroid()[axis] <= split_point;});

    // recurse
    node->l = construct_bvh(start, split, max_leaf_size);
    node->r = construct_bvh(split, end, max_leaf_size);

    return node;
    
}

bool BVHAccel::has_intersection(const Ray &ray, BVHNode *node) const {
  // TODO (Part 2.3):
  // Fill in the intersect function.
  // Take note that this function has a short-circuit that the
  // Intersection version cannot, since it returns as soon as it finds
  // a hit, it doesn't actually have to find the closest hit.
    
//    for (auto p : primitives) {
//        total_isects++;
//        if (p->has_intersection(ray))
//          return true;
//    }
//    return false;
    
    // stop if no intersection
    double t0 = ray.min_t;
    double t1 = ray.max_t;
    if (!node->bb.intersect(ray, t0, t1)) {
        return false;
    }

    // leaf base case
    if (node->isLeaf()) {
        for (auto p = node->start; p != node->end; p++) {
            total_isects++;
            if ((*p)->has_intersection(ray)) {
                return true;
            }
        }
        return false;
    }

    // recurse on l and r children
    return has_intersection(ray, node->l) || has_intersection(ray, node->r);
}

bool BVHAccel::intersect(const Ray &ray, Intersection *i, BVHNode *node) const {
  // TODO (Part 2.3):
  // Fill in the intersect function.

//  bool hit = false;
//  for (auto p : primitives) {
//    total_isects++;
//    hit = p->intersect(ray, i) || hit;
//  }
//  return hit;
    
    // stop if no intersection
    double t0 = ray.min_t;
    double t1 = ray.max_t;
    if (!node->bb.intersect(ray, t0, t1)) {
        return false;
    }

    // leaf base case
    if (node->isLeaf()) {
        bool hit = false;
        for (auto p = node->start; p != node->end; p++) {
            total_isects++;
            hit = (*p)->intersect(ray, i) || hit;
        }
        return hit;
    }

    // recurse on l and r children
    bool hit_l = intersect(ray, i, node->l);
    bool hit_r = intersect(ray, i, node->r);
    return hit_l || hit_r;

}

} // namespace SceneObjects
} // namespace CGL
